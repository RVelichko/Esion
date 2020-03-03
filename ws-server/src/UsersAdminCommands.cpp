#include <limits>

#include "Log.hpp"
#include "uuid.hpp"
#include "UsersAdminCommands.hpp"


namespace ph = std::placeholders;

using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


size_t UsersAdminBaseCommand::_garb_timer = DEFAULT_GARBAGE_TIMEOUT;
PDbFacade UsersAdminBaseCommand::_db;
Auth UsersAdminBaseCommand::_auth;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef std::pair<std::string, ExecuteFn> CommandsExecuter;


template <class CmdType>
class ExecuteCmd {
    CommandsExecuter _cmd_exec;

public:
    ExecuteCmd(const std::string& name)
        : _cmd_exec(name, [](const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = CmdType(js, mutex, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
        })
    {}

    CommandsExecuter value() {
        return _cmd_exec;
    }
};


bool UsersAdminBaseCommand::executeByName(const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
    /// Инициализация локальных статических массивов с обработчиками команд.
    static CommandsExecuters cmd_execs = {
        ExecuteCmd<AdminAuthorizeCommand>("auth").value(),
        ExecuteCmd<AdminLogoutCommand>("logout").value(),
        ExecuteCmd<GetUsersCommand>("get_users").value(),
        ExecuteCmd<GetUserCommand>("get_user").value(),
        ExecuteCmd<AddUserCommand>("add_user").value(),
        ExecuteCmd<UpdateUserCommand>("update_user").value(),
        ExecuteCmd<GetUserAddressesCommand>("get_addrs").value()
    };
    static std::mutex cmd_execs_mutex;
    /// Выполнить обработку команды.
    auto jname = js.find("name");
    if (jname not_eq js.end()) {
        ExecuteFn exec_fn;
        { /// LOCK Для доступа к списку команд.
            LockQuard l(cmd_execs_mutex);
            auto jexecute = cmd_execs.find(*jname);
            if (jexecute not_eq cmd_execs.end()) {
                exec_fn = jexecute->second;
            }
        }
        if (exec_fn) {
            return exec_fn(js, mutex, snd_fn);
        } else {
            LOG(WARNING) << "Can`t find command name " << *jname << "!";
            Json jresp = {
                {"resp", {
                    {"name", *jname},
                    {"status", "err"},
                    {"desc", "Can`t find command name " + jname->get<std::string>() + "!"}
                }}
            };
            snd_fn(jresp.dump());
        }
    } else {
        LOG(WARNING) << "Command is not correct! Can`t find tag `name`.";
        Json jresp = {
            {"resp", {
                {"name", *jname},
                {"status", "err"},
                {"desc", "Command " + jname->get<std::string>() + " is`t correct! Can`t find tag `name`."}
            }}
        };
        snd_fn(jresp.dump());
    }
    return false;
}


Json UsersAdminBaseCommand::getErrorResponce(const std::string &desc) {
    return {
        {"resp", {
            {"name", _name},
            {"status", "err"},
            {"desc", desc}
        }}
    };
}


void UsersAdminBaseCommand::eraseMongoId(Json& js) {
    auto erase_fn = [](Json& js) {
        auto j_id = js.find("_id");
        if (j_id not_eq js.end()) {
            js.erase("_id");
        }
    };
    if (js.is_array()) {
        for (auto& jobj : js) {
            erase_fn(jobj);
        }
    } else {
        erase_fn(js);
    }
}


Json UsersAdminBaseCommand::fillResponceData(const Json& js) {
    /// Заполнить переменную ответа.
    Json jres = {
      {"resp", {
        {"name", _name},
        {"status", "ok"},
        {"data", js}
      }}
    };
    LOG(DEBUG) << jres;
    return jres;
}


bool UsersAdminBaseCommand::checkToken(const std::string& token) {
    LockQuard l(_mutex);
    auto jusr = _db->findToken(token, ADMIN_COLLECTION_NAME);
    return not jusr.empty();
}


std::string UsersAdminBaseCommand::getCollectionId(const std::string& token) {
    std::string coll_id;
    auto jusr = _db->findToken(token, ADMIN_COLLECTION_NAME);
    if (not jusr.empty() and jusr.is_object()) {
        auto jcoll_id = jusr.find("coll_id");
        if (jcoll_id not_eq jusr.end() and jcoll_id->is_string()) {
            coll_id = jcoll_id->get<std::string>();
        }
    }
    LOG(DEBUG) << "Collection ID: " << coll_id;
    return coll_id;
}


UsersAdminBaseCommand::UsersAdminBaseCommand(const std::string& name, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : JsonCommand(name, js)
    , _mutex(mutex)
    , _snd_fn(snd_fn)
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AdminAuthorizeCommand::AdminAuthorizeCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("auth", js, mutex, snd_fn) {
    LOG(DEBUG);
}


AdminAuthorizeCommand::~AdminAuthorizeCommand() {
    LOG(DEBUG);
}


Json AdminAuthorizeCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jlogin = _jdata.find("login");
        auto jpswd = _jdata.find("pswd");
        auto jtoken = _jdata.find("token");
        if (jlogin not_eq _jdata.end() and jlogin->is_string() and
            jpswd not_eq _jdata.end() and jpswd->is_string()) {
            Json jusr;
            { /// LOCK
                LockQuard l(_mutex);
                _db->eraseOldTokens(OLD_TOKENS_TIMEOUT);
                jusr = _db->findUser(*jlogin, *jpswd, ADMIN_COLLECTION_NAME);
            }
            if (not jusr.empty()) {
                eraseMongoId(jusr);
                std::string desc = jusr.value("desc", "");
                std::string person = jusr.value("person", "");
                std::string token = std::to_string(time(nullptr));
                jres["resp"] = {
                    {"name", _name},
                    {"status", "ok"},
                    {"token", token},
                    {"user_info", {
                        {"person", person},
                        {"desc", desc}
                    }}
                };
                jusr["token"].push_back(token);
                LockQuard l(_mutex);
                _db->insertUser(jusr);
            } else {
                LOG(FATAL) << "Value user is null.";
                jres = getErrorResponce("Can`t found user " + jlogin->get<std::string>() + " | " + jpswd->get<std::string>() + ".");
            }
        } else if (jtoken not_eq _jdata.end() and
                   jtoken->is_string() and
                   not jtoken->get<std::string>().empty() and
                   jtoken->get<std::string>() not_eq "null" and
                   std::atoll(jtoken->get<std::string>().c_str()) not_eq 0) {
            Json jusr;
            { /// LOCK
                LockQuard l(_mutex);
                _db->eraseOldTokens(OLD_TOKENS_TIMEOUT);
                jusr = _db->findToken(*jtoken, ADMIN_COLLECTION_NAME);
            }
            if (not jusr.empty()) {
                eraseMongoId(jusr);
                std::string person = jusr.value("person", "");
                std::string desc = jusr.value("desc", "");
                jres["resp"] = {
                    {"name", _name},
                    {"status", "ok"},
                    {"token", *jtoken},
                    {"user_info", {
                        {"person", person},
                        {"desc", desc}
                    }}
                };
            } else {
                LOG(FATAL) << "Invalid token " << *jtoken;
                jres = getErrorResponce("Invalid token!");
            }
        } else {
            LOG(WARNING) << "Incorrect cmd: [" << _name << "] \"" << _jdata.dump() << "\"";
            jres = getErrorResponce("Incorrect command.");
        }
    } else {
        LOG(ERROR) << "Incorrect Command: " << _name;
        jres = getErrorResponce("Incorrect Command: " + _name);
    }
    _snd_fn(jres.dump());
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AdminLogoutCommand::AdminLogoutCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("logout", js, mutex, snd_fn) {
    LOG(DEBUG);
}


AdminLogoutCommand::~AdminLogoutCommand() {
    LOG(DEBUG);
}


Json AdminLogoutCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string()) {
            std::string token = *jtoken;
            Json jusr;
            { /// LOCK
                LockQuard l(_mutex);
                jusr = _db->findToken(token, ADMIN_COLLECTION_NAME);
            }
            if (not jusr.empty()) {
                eraseMongoId(jusr);
                jres["resp"] = {
                    {"name", _name},
                    {"status", "ok"}
                };
                auto jtokens = jusr.find("token");
                if (jtokens not_eq jusr.end()) {
                    std::remove(jtokens->begin(), jtokens->end(), token);
                }
                LockQuard l(_mutex);
                jusr = _db->insertUser(jusr);
            } else {
                LOG(FATAL) << "Value user is null. " << token;
                jres = getErrorResponce("Can`t found user |" + token + "|.");
            }
        } else {
            LOG(ERROR) << "Tag token is not set.";
            jres = getErrorResponce("This user is not loginned.");
        }
    } else {
        LOG(ERROR) << "Incorrect Command: " << _name;
        jres = getErrorResponce("Incorrect Command: " + _name);
    }
    _snd_fn(jres.dump());
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetUsersCommand::GetUsersCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("get_users", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetUsersCommand::~GetUsersCommand() {
    LOG(DEBUG);
}


Json GetUsersCommand::execute() {
    Json jres;
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetUserCommand::GetUserCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("get_user", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetUserCommand::~GetUserCommand() {
    LOG(DEBUG);
}


Json GetUserCommand::execute() {
    Json jres;
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AddUserCommand::AddUserCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("add_user", js, mutex, snd_fn) {
    LOG(DEBUG);
}


AddUserCommand::~AddUserCommand() {
    LOG(DEBUG);
}


Json AddUserCommand::execute() {
    Json jres;
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


UpdateUserCommand::UpdateUserCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("update_user", js, mutex, snd_fn) {
    LOG(DEBUG);
}


UpdateUserCommand::~UpdateUserCommand() {
    LOG(DEBUG);
}


Json UpdateUserCommand::execute() {
    Json jres;
    if (_is_corrected) {
        size_t found = 0;
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jskip = _jdata.find("skip");
            auto jnum  = _jdata.find("num");
            if (not jskip->empty() and jskip->is_number() and not jnum->empty() and jnum->is_number()) {
                /// Подготовка полей для определения типа команды.
                size_t skip = *jskip;
                size_t num = *jnum;
                auto jfilter    = _jdata.find("filter");
                auto jsort      = _jdata.find("sort");
                auto jdate_time_from = _jdata.find("date_time_from");
                auto jdate_time_to   = _jdata.find("date_time_to");
                auto jdate_type      = _jdata.find("date_type");
                std::string field;
                bool direct_flag = true;
                if (jsort not_eq _jdata.end() and jsort->is_object()) {
                    auto jfield = jsort->find("field");
                    auto jdirection = jsort->find("direction");
                    if (jfield not_eq jsort->end() and jfield->is_string() and
                        jdirection not_eq jsort->end() and jdirection->is_string() and
                        ((*jdirection) == "asc" or (*jdirection) == "desc")) {
                        field = jsort->value("field", "");
                        direct_flag = ((jsort->value("direction", "asc") == "asc") ? true : false);
                    } else {
                        LOG(WARNING) << "Incorrect sort tag: [" << _name << "] \"" << _jdata.dump() << "\"";
                        jres = getErrorResponce("Incorrect 'sort' tag.");
                    }
                }
                Json jlist;
                std::string filter;
                if (jfilter not_eq _jdata.end()) {
                    filter = *jfilter;
                }
                size_t date_from = 0;
                if (jdate_time_from not_eq _jdata.end()) {
                    date_from = JsonCommand::ToNumber(_jdata, "date_time_from");
                }
                size_t date_to = 0;
                if (jdate_time_to not_eq _jdata.end()) {
                    date_to = JsonCommand::ToNumber(_jdata, "date_time_to");
                }
                std::string date_type;
                if (jdate_type not_eq _jdata.end()) {
                    date_type = *jdate_type;
                }
                { /// LOCK
                    LockQuard l(_mutex);
                    jlist = _db->getUsersList(found, filter,
                                              date_type, date_from, date_to,
                                              field, direct_flag,
                                              num, skip);
                }
                eraseMongoId(jlist);
                jres = fillResponceData(jlist);
                jres["resp"]["count"] = found;
            } else {
                LOG(WARNING) << "Incorrect cmd: [" << _name << "] \"" << _jdata.dump() << "\"";
                jres = getErrorResponce("Incorrect command.");
            }
        } else {
            LOG(FATAL) << "Invalid token " << *jtoken;
            jres = getErrorResponce("Invalid token.");
        }
    } else {
        LOG(ERROR) << "Incorrect Command: " << _name;
        jres = getErrorResponce("Incorrect Command: " + _name);
    }
    _snd_fn(jres.dump());
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetUserAddressesCommand::GetUserAddressesCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : UsersAdminBaseCommand("get_addrs", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetUserAddressesCommand::~GetUserAddressesCommand() {
    LOG(DEBUG);
}


Json GetUserAddressesCommand::execute() {
    Json jres;
    return jres;
}
