#include <limits>

#include "Log.hpp"
#include "uuid.hpp"
#include "OperatorCommands.hpp"
#include "ReportGenerator.hpp"


namespace ph = std::placeholders;

using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


//std::string BaseCommand::_token;
size_t BaseCommand::_garb_timer = DEFAULT_GARBAGE_TIMEOUT;
PDbFacade BaseCommand::_db;
AuthMap BaseCommand::_auth;
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


bool BaseCommand::executeByName(const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
    /// Инициализация локальных статических массивов с обработчиками команд.
    static CommandsExecuters cmd_execs = {
        ExecuteCmd<AuthorizeCommand>("auth").value(),
        ExecuteCmd<LogoutCommand>("logout").value(),
        ExecuteCmd<UniqueAddressesCommand>("get_uniq_addrs").value(),
        ExecuteCmd<GetDevicesListCommand>("get_devs").value(),
        ExecuteCmd<ActivateDeviceCommands>("set_dev_status").value(),
        ExecuteCmd<GetEventsListCommand>("get_events").value(),
        ExecuteCmd<CreateDevicesReportCommand>("get_devices_report").value(),
        ExecuteCmd<CreateEventsReportCommand>("get_events_report").value(),
        ExecuteCmd<GetCriticalNumberCommand>("get_critical").value(),
        ExecuteCmd<GetDeviceCommand>("get_device").value()
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


Json BaseCommand::getErrorResponce(const std::string &desc) {
    return {
        {"resp", {
            {"name", _name},
            {"status", "err"},
            {"desc", desc}
        }}
    };
}


void BaseCommand::eraseMongoId(Json& js) {
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


Json BaseCommand::fillResponceData(const Json& js) {
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


bool BaseCommand::checkToken(const std::string& token) {
    LockQuard l(_mutex);
    auto jusr = _db->findUser(token);
    return not jusr.empty();
}


std::string BaseCommand::getCollectionId(const std::string& token) {
    std::string coll_id;
    auto jusr = _db->findUser(token);
    if (not jusr.empty() and jusr.is_object()) {
        auto jcoll_id = jusr.find("coll_id");
        if (jcoll_id not_eq jusr.end() and jcoll_id->is_string()) {
            coll_id = jcoll_id->get<std::string>();
        }
    }
    LOG(DEBUG) << "Collection ID: " << coll_id;
    return coll_id;
}


BaseCommand::BaseCommand(const std::string& name, const Json& js,
                         std::mutex& mutex, const SendFn& snd_fn)
    : JsonCommand(name, js)
    , _mutex(mutex)
    , _snd_fn(snd_fn)
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AuthorizeCommand::AuthorizeCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("auth", js, mutex, snd_fn) {
    LOG(DEBUG);
}


AuthorizeCommand::~AuthorizeCommand() {
    LOG(DEBUG);
}


Json AuthorizeCommand::execute() {
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
                jusr = _db->findUser(*jlogin, *jpswd);
            }
            if (not jusr.empty()) {
                eraseMongoId(jusr);
                std::string person = jusr.value("person", "");
                std::string desc = jusr.value("desc", "");
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
                jusr = _db->findUser(*jtoken);
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


LogoutCommand::LogoutCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("logout", js, mutex, snd_fn) {
    LOG(DEBUG);
}


LogoutCommand::~LogoutCommand() {
    LOG(DEBUG);
}


Json LogoutCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string()) {
            std::string token = *jtoken;
            Json jusr;
            { /// LOCK
                LockQuard l(_mutex);
                jusr = _db->findUser(token);
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


UniqueAddressesCommand::UniqueAddressesCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("get_uniq_addrs", js, mutex, snd_fn) {
    LOG(DEBUG);
}


UniqueAddressesCommand::~UniqueAddressesCommand() {
    LOG(DEBUG);
}


Json UniqueAddressesCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jfilter = _jdata.find("filter");
            std::string filter;
            if (jfilter not_eq _jdata.end()) {
                filter = *jfilter;
            }
            Json juniq_addrs;
            { /// LOCK
                LockQuard l(_mutex);
                auto coll_id = getCollectionId(*jtoken);
                juniq_addrs = _db->getUniqueAddresses(coll_id, filter);
            }
            jres["resp"] = {
                {"name", _name},
                {"status", "ok"},
                {"uniq_addrs", juniq_addrs}
            };
        } else {
            LOG(FATAL) << "Invalid token " << *jtoken;
            jres = getErrorResponce("Invalid token!");
        }
    } else {
        LOG(ERROR) << "Incorrect Command: " << _name;
        jres = getErrorResponce("Incorrect Command: " + _name);
    }
    _snd_fn(jres.dump());
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetDevicesListCommand::GetDevicesListCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("get_devs", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetDevicesListCommand::~GetDevicesListCommand() {
    LOG(DEBUG);
}


Json GetDevicesListCommand::execute() {
    Json jres;
    if (_is_corrected) {
        size_t found = 0;
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jskip = _jdata.find("skip");
            auto jnum  = _jdata.find("num");
            if (not jskip->empty() and jskip->is_number() and not jnum->empty() and jnum->is_number()) {
                size_t skip = *jskip;
                size_t num = *jnum;
                /// Подготовка полей для определения типа команды.
                auto jgeo_poly  = _jdata.find("geo_poly");
                auto jgeo       = _jdata.find("geo");
                auto jradius    = _jdata.find("radius");
                auto jfilter    = _jdata.find("filter");
                auto jsort      = _jdata.find("sort");
                auto jdate_time = _jdata.find("date_time");
                auto jdate_type = _jdata.find("date_type");
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
                std::string coll_id;
                { /// LOCK
                    LockQuard l(_mutex);
                    coll_id = getCollectionId(*jtoken);
                }
                /// Выполнение команды в соответствии с заданными полями.
                if (jgeo_poly not_eq _jdata.end() and jgeo_poly->is_array() and jgeo_poly->size() == 4) {
                    double x = (*jgeo_poly)[0];
                    double y = (*jgeo_poly)[1];
                    double w = (*jgeo_poly)[2];
                    double h = (*jgeo_poly)[3];
                    Json jlist;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jlist = _db->getByPoly(found, CONTROOLERS_COLLECTION_NAME, coll_id, x, y, w, h, num, skip);
                    }
                    eraseMongoId(jlist);
                    jres = fillResponceData(jlist);
                    jres["resp"]["count"] = found;
                } else if (jgeo not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 2 and
                    jradius not_eq _jdata.end() and jradius->is_number()) {
                    double lo = (*jgeo)[0];
                    double la = (*jgeo)[1];
                    double r = *jradius;
                    Json jlist;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jlist = _db->getByGeo(found, CONTROOLERS_COLLECTION_NAME, coll_id, lo, la, r, num, skip);
                    }
                    eraseMongoId(jlist);
                    jres = fillResponceData(jlist);
                    jres["resp"]["count"] = found;
                } else if (jfilter not_eq _jdata.end() and jfilter->is_string() and not jfilter->empty()) {
                    Json jlist;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jlist = _db->getByFilter(found, CONTROOLERS_COLLECTION_NAME, coll_id, *jfilter,
                                                 field, direct_flag, num, skip);
                    }
                    eraseMongoId(jlist);
                    jres = fillResponceData(jlist);
                    jres["resp"]["count"] = found;
                } else if (jdate_time not_eq _jdata.end() and jdate_time->is_number() and
                           jdate_type not_eq _jdata.end() and jdate_type->is_string()) {
                    time_t date_time = static_cast<time_t>(jdate_time->get<size_t>());
                    std::string date_type = *jdate_type;
                    Json jlist;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jlist = _db->getDevicesByTime(found, coll_id, date_time, date_type, field, direct_flag, num, skip);
                    }
                    eraseMongoId(jlist);
                    jres = fillResponceData(jlist);
                    jres["resp"]["count"] = found;
                } else if (jgeo_poly == _jdata.end() and jgeo == _jdata.end() and
                           jradius == _jdata.end() and jfilter == _jdata.end()) {
                    Json jlist;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jlist = _db->getList(found, CONTROOLERS_COLLECTION_NAME, coll_id, field, direct_flag, num, skip);
                    }
                    eraseMongoId(jlist);
                    jres = fillResponceData(jlist);
                    jres["resp"]["count"] = found;
                }
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


ActivateDeviceCommands::ActivateDeviceCommands(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("set_dev_status", js, mutex, snd_fn) {
    LOG(DEBUG);
}


ActivateDeviceCommands::~ActivateDeviceCommands() {
    LOG(DEBUG);
}


Json ActivateDeviceCommands::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jdev_id = _jdata.find("dev_id");
            auto jstatus = _jdata.find("status");
            if (jdev_id not_eq _jdata.end() and jdev_id->is_string() and
                jstatus not_eq _jdata.end() and jstatus->is_string()) {
                _mutex.lock();
                auto jdev = _db->getDevice(*jdev_id);
                _mutex.unlock();
                if (not jdev.empty() and jdev.is_object()) {
                    jdev["status"] = *jstatus;
                    _mutex.lock();
                    _db->insertDevice(jdev);
                    _mutex.unlock();
                    jres = {
                      {"resp", {
                        {"name", _name},
                        {"status", "ok"}
                      }}
                    };
                    LOG(DEBUG) << jres;
                } else {
                    _mutex.unlock();
                    LOG(FATAL) << "DB is NULL!";
                    jres = getErrorResponce("Internal DB error.");
                }
            } else {
                LOG(WARNING) << "Incorrect command format! Need dev_id[string] and status[string]";
                jres = getErrorResponce("Incorrect command format! Need dev_id->string and status->string.");
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


GetEventsListCommand::GetEventsListCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("get_events", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetEventsListCommand::~GetEventsListCommand() {
    LOG(DEBUG);
}


Json GetEventsListCommand::execute() {
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
                auto jgeo_poly = _jdata.find("geo_poly");
                auto jgeo      = _jdata.find("geo");
                auto jradius   = _jdata.find("radius");
                auto jfilter   = _jdata.find("filter");
                auto jdev_id   = _jdata.find("dev_id");
                auto jsort      = _jdata.find("sort");
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
                std::string coll_id;
                { /// LOCK
                    LockQuard l(_mutex);
                    coll_id = getCollectionId(*jtoken);
                }
                /// Выполнение типа команды в соответствии с переданными полями.
                if (jgeo_poly not_eq _jdata.end() and jgeo_poly->is_array() and jgeo_poly->size() == 4) {
                    double x = (*jgeo_poly)[0];
                    double y = (*jgeo_poly)[1];
                    double w = (*jgeo_poly)[2];
                    double h = (*jgeo_poly)[3];
                    Json jevs;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jevs = _db->getByPoly(found, EVENTS_COLLECTION_NAME, coll_id, x, y, w, h, num, skip);
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                } else if (jgeo not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 2 and
                    jradius not_eq _jdata.end() and jradius->is_number()) {
                    double lo = (*jgeo)[0];
                    double la = (*jgeo)[1];
                    double r = *jradius;
                    Json jevs;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jevs = _db->getByGeo(found, EVENTS_COLLECTION_NAME, coll_id, lo, la, r, num, skip);
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                    jres["resp"]["count"] = found;
                } else if (jdev_id not_eq _jdata.end() and
                           jdev_id->is_string() and not jdev_id->empty()) {
                    Json jevs;
                    if (not field.empty()) {
                        LockQuard l(_mutex);
                        jevs = _db->getEventsByDevId(found, *jdev_id, field, direct_flag, num, skip);
                    } else {
                        LockQuard l(_mutex);
                        jevs = _db->getEventsByDevId(found, *jdev_id, num, skip);
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                    jres["resp"]["count"] = found;
                } else if (jfilter not_eq _jdata.end() and
                           jfilter->is_string() and not jfilter->empty()) {
                    Json jevs;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jevs = _db->getByFilter(found, EVENTS_COLLECTION_NAME, coll_id, *jfilter, field, direct_flag, num, skip);
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                    jres["resp"]["count"] = found;
                } else if (jgeo_poly == _jdata.end() and jgeo == _jdata.end() and
                           jradius == _jdata.end() and jfilter == _jdata.end()) {
                    Json jdevs;
                    { /// LOCK
                        LockQuard l(_mutex);
                        jdevs = _db->getList(found, EVENTS_COLLECTION_NAME, coll_id, field, direct_flag, num, skip);
                    }
                    eraseMongoId(jdevs);
                    jres = fillResponceData(jdevs);
                    jres["resp"]["count"] = found;
                }
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


CreateDevicesReportCommand::CreateDevicesReportCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("get_devices_report", js, mutex, snd_fn) {
    LOG(DEBUG);
}


CreateDevicesReportCommand::~CreateDevicesReportCommand() {
    LOG(DEBUG);
}


Json CreateDevicesReportCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jcoll = _jdata.find("coll");
            if (jcoll not_eq _jdata.end()) {
                size_t found = 0;
                Json jvals;
                { /// LOCK
                    LockQuard l(_mutex);
                    auto coll_id = getCollectionId(*jtoken);
                    jvals = _db->getByFilter(found, CONTROOLERS_COLLECTION_NAME, coll_id, *jcoll,
                                             "", true, std::numeric_limits<uint32_t>::max(), 0, true);
                }
                if (not jvals.empty()) {
                    std::string encoding;
                    auto jencoding = _jdata.find("encoding");
                    if (jencoding not_eq _jdata.end() and jencoding->is_string()) {
                        encoding = (*jencoding);
                    }
                    DevicesReportGenerator rep_gen(jvals, encoding, _snd_fn);
                    if (rep_gen) {
                        jres = {
                          {"resp", {
                            {"name", _name},
                            {"status", "ok"},
                            {"report_url", std::string(rep_gen)}
                          }}
                        };
                        LOG(DEBUG) << jres;
                    }
                } else {
                    LOG(ERROR) << "Can`t create report!";
                    jres = getErrorResponce("Can`t create report!");
                }
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


CreateEventsReportCommand::CreateEventsReportCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("get_events_report", js, mutex, snd_fn) {
    LOG(DEBUG);
}


CreateEventsReportCommand::~CreateEventsReportCommand() {
    LOG(DEBUG);
}


Json CreateEventsReportCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jcoll = _jdata.find("coll");
            if (jcoll not_eq _jdata.end()) {
                size_t found = 0;
                Json jvals;
                { /// LOCK
                    LockQuard l(_mutex);
                    auto coll_id = getCollectionId(*jtoken);
                    jvals = _db->getByFilter(found, EVENTS_COLLECTION_NAME, coll_id, *jcoll,
                                             "", true, std::numeric_limits<uint32_t>::max());
                }
                if (not jvals.empty()) {
                    std::string encoding;
                    auto jencoding = _jdata.find("encoding");
                    if (jencoding not_eq _jdata.end() and jencoding->is_string()) {
                        encoding = (*jencoding);
                    }
                    EventsReportGenerator rep_gen(jvals, encoding, _snd_fn);
                    if (rep_gen) {
                        jres = {
                          {"resp", {
                            {"name", _name},
                            {"status", "ok"},
                            {"report_url", std::string(rep_gen)}
                          }}
                        };
                        LOG(DEBUG) << jres;
                    }
                } else {
                    LOG(ERROR) << "Can`t create report!";
                    jres = getErrorResponce("Can`t create report!");
                }
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


GetCriticalNumberCommand::GetCriticalNumberCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand("get_critical", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetCriticalNumberCommand::~GetCriticalNumberCommand() {
    LOG(DEBUG);
}


Json GetCriticalNumberCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jfilter = _jdata.find("filter");
            if (jfilter not_eq _jdata.end()) {
                size_t found = 0;
                { /// LOCK
                    LockQuard l(_mutex);
                    auto coll_id = getCollectionId(*jtoken);
                    found = _db->getCriticalNum(coll_id, *jfilter);
                }
                jres = {
                  {"resp", {
                    {"name", _name},
                    {"status", "ok"},
                    {"num", found}
                  }}
                };
                LOG(DEBUG) << jres;
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


GetDeviceCommand::GetDeviceCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    :BaseCommand("get_device", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetDeviceCommand::~GetDeviceCommand() {
    LOG(DEBUG);
}


Json GetDeviceCommand::execute() {
    LOG(DEBUG);
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and checkToken(*jtoken)) {
            auto jdev_id = _jdata.find("dev_id");
            if (jdev_id not_eq _jdata.end() and jdev_id->is_string()) {
                Json jdev;
                { /// LOCK
                    LockQuard l(_mutex);
                    jdev = _db->getDevice(*jdev_id);
                }
                jres = {
                  {"resp", {
                    {"name", _name},
                    {"status", "ok"},
                    {"device", jdev}
                  }}
                };
                LOG(DEBUG) << jres;
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
