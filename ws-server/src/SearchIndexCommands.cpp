#include "Log.hpp"
#include "SearchIndexCommands.hpp"


using namespace sindex;

typedef std::lock_guard<std::mutex> LockQuard;

std::mutex BaseCommand::_xdb_mutex;

std::string BaseCommand::_login;
std::string BaseCommand::_pswd;


Json BaseCommand::getErrorResponce(const std::string& desc) {
    return {
        {"resp", {
            {"name", _name},
            {"status", "err"},
            {"desc", desc}
        }}
    };
}


void BaseCommand::eraseMongoId(Json& js) {
    auto j_id = js.find("_id");
    if (j_id not_eq js.end()) {
        js.erase("_id");
    }
}


bool BaseCommand::executeByName(const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn) {
    static CommandsExecuters cmd_execs = {
       {"auth", [](const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn) {
            auto cmd = AuthorizeCommand(js, xdb, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"add_dev", [](const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn) {
            auto cmd = AddDeviceCommand(js, xdb, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"find_devs",[](const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn) {
            auto cmd = FindDevicesCommand(js, xdb, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"add_ev",[](const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn) {
            auto cmd = AddEventCommand(js, xdb, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"find_evs",[](const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn) {
            auto cmd = FindEventsCommand(js, xdb, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }}
    };
    static std::mutex cmd_execs_mutex;

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
            return exec_fn(js, xdb, snd_fn);
        } else {
            LOG(WARNING) << "Can`t find command name " << *jname << "\"!";
        }
    } else {
        LOG(WARNING) << "Command is not correct! Can`t find \"name\".";
    }
    return false;
}


BaseCommand::BaseCommand(const std::string& name, const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn)
    : JsonCommand(name, js)
    , _xdb(xdb)
    , _snd_fn(snd_fn)
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AuthorizeCommand::AuthorizeCommand(const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn)
    : BaseCommand("auth", js, xdb, snd_fn) {
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
        if (jlogin not_eq _jdata.end() and jpswd not_eq _jdata.end()) {
            auto login = _jdata.value("login", "");
            auto pswd = _jdata.value("pswd", "");
            if (login.empty()) {
                LOG(FATAL) << "Value user is null.";
                jres = getErrorResponce("Value user is null.");
            } else if (pswd.empty()) {
                LOG(FATAL) << "Value user is null.";
                jres = getErrorResponce("Value password is null.");
            } else {
                if (BaseCommand::_login == login and
                    BaseCommand::_pswd == pswd) {
                    jres = {
                        {"resp", {
                            {"name", _name},
                            {"status", "ok"}
                        }}
                    };
                } else {
                    LOG(FATAL) << "Value user is null.";
                    jres = getErrorResponce("Can`t found user |" + login + " | " + pswd + "|.");
                }
            }
        } else {
            LOG(WARNING) << "Incorrect cmd: [" << _name << "] \"" << _jdata.dump() << "\"";
            jres = getErrorResponce("Incorrect command.");
        }
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AddDeviceCommand::AddDeviceCommand(const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn)
    : BaseCommand("add_dev", js, xdb, snd_fn) {
    LOG(DEBUG);
}


AddDeviceCommand::~AddDeviceCommand() {
    LOG(DEBUG);
}


Json AddDeviceCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jqstr = _jdata.find("qstr");
        auto j_id = _jdata.find("_id");
        if (jqstr not_eq _jdata.end() and j_id not_eq _jdata.end()) {
            bool is_ok = false;
            _xdb_mutex.lock();
            if (_xdb) {
                is_ok = _xdb->addDeviceIndex(*j_id, *jqstr);
                _xdb_mutex.unlock();
            } else {
                _xdb_mutex.unlock();
                LOG(FATAL) << "Index server ic NULL!";
            }
            if (is_ok) {
                jres = {
                    {"resp", {
                        {"name", "add_dev"},
                        {"status", "ok"}
                    }}
                };
            } else {
                jres = getErrorResponce("Can`t add device.");
            }
        } else {
            jres = getErrorResponce("Incorrect command data format.");
        }
    }
    _snd_fn(jres.dump());
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


FindDevicesCommand::FindDevicesCommand(const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn)
    : BaseCommand("find_devs", js, xdb, snd_fn) {
    LOG(DEBUG);
}


FindDevicesCommand::~FindDevicesCommand() {
    LOG(DEBUG);
}


Json FindDevicesCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is`t implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AddEventCommand::AddEventCommand(const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn)
    : BaseCommand("add_ev", js, xdb, snd_fn) {
    LOG(DEBUG);
}


AddEventCommand::~AddEventCommand() {
    LOG(DEBUG);
}


Json AddEventCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is`t implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


FindEventsCommand::FindEventsCommand(const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn)
    : BaseCommand("find_evs", js, xdb, snd_fn) {
    LOG(DEBUG);
}


FindEventsCommand::~FindEventsCommand() {
    LOG(DEBUG);
}


Json FindEventsCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is`t implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
