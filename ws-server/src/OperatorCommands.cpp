#include "Log.hpp"
//#include "SearchIndexClient.hpp"
#include "OperatorCommands.hpp"
#include "ReportGenerator.hpp"


using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;
typedef sindex::IndexIds DevicesIds;


PDbFacade BaseCommand::_db;
PIndexDbFacade BaseCommand::_xdb;
std::string BaseCommand::_reports_path;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef std::pair<std::string, ExecuteFn> CommandsExecuter;


template <class CmdType>
class ExecuteCmd {
    CommandsExecuter _cmd_exec;

public:
    ExecuteCmd(const std::string& name)
        : _cmd_exec(name, [](const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = CmdType(token, js, mutex, snd_fn);
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


bool BaseCommand::executeByName(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
    /// Инициализация локальных статических массивов с обработчиками команд.
    static CommandsExecuters cmd_execs = {
        ExecuteCmd<LogoutCommand>("auth").value(),
        ExecuteCmd<LogoutCommand>("logout").value(),
        ExecuteCmd<LogoutCommand>("get_devs").value(),
        ExecuteCmd<LogoutCommand>("set_dev_status").value(),
        ExecuteCmd<LogoutCommand>("get_events").value(),
        ExecuteCmd<LogoutCommand>("get_report").value(),
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
            return exec_fn(token, js, mutex, snd_fn);
        } else {
            LOG(WARNING) << "Can`t find command name \"" << *jname << "\"!";
        }
    } else {
        LOG(WARNING) << "Command is not correct! Can`t find \"name\".";
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


BaseCommand::BaseCommand(const std::string& token, const std::string& name, const Json& js,
                         std::mutex& mutex, const SendFn& snd_fn)
    : JsonCommand(name, js)
    , _token(token)
    , _mutex(mutex)
    , _snd_fn(snd_fn)
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AuthorizeCommand::AuthorizeCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "auth", js, mutex, snd_fn) {
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
            _mutex.lock();
            if (_db) {
                jusr = _db->findUser(*jlogin, *jpswd);
                _mutex.unlock();
            } else {
                _mutex.unlock();
                LOG(FATAL) << "DB is NULL!";
                jres = getErrorResponce("Internal DB error.");
            }
            if (not jusr.empty()) {
                eraseMongoId(jusr);
                std::string person = jusr.value("peson", "");
                std::string desc = jusr.value("desc", "");
                jres["resp"] = {
                    {"name", _name},
                    {"status", "ok"},
                    {"token", _token},
                    {"user_info", {
                        {"person", person},
                        {"desc", desc}
                    }}
                };
                jusr["token"] = _token;
                LockQuard l(_mutex);
                _db->insertUser(jusr);
            } else {
                LOG(FATAL) << "Value user is null.";
                jres = getErrorResponce("Can`t found user " + jlogin->get<std::string>() + " | " + jpswd->get<std::string>() + ".");
            }
        } else if (jtoken not_eq _jdata.end() and jtoken->is_string()) {
            Json jusr;
            _mutex.lock();
            if (_db) {
                jusr = _db->findUser(*jtoken);
                _mutex.unlock();
            } else {
                _mutex.unlock();
                LOG(FATAL) << "DB is NULL!";
                jres = getErrorResponce("Internal DB error.");
            }
            if (not jusr.empty()) {
                _token = *jtoken;
                eraseMongoId(jusr);
                std::string person = jusr.value("peson", "");
                std::string desc = jusr.value("desc", "");
                jres["resp"] = {
                    {"name", _name},
                    {"status", "ok"},
                    {"token", _token},
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
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


LogoutCommand::LogoutCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "logout", js, mutex, snd_fn) {
    LOG(DEBUG);
}


LogoutCommand::~LogoutCommand() {
    LOG(DEBUG);
}


Json LogoutCommand::execute() {
    Json jres;
    if (_is_corrected) {
        if (not _token.empty()) {
            Json jusr;
            _mutex.lock();
            if (_db) {
                jusr = _db->findUser(_token);
                _mutex.unlock();
            } else {
                _mutex.unlock();
                LOG(FATAL) << "DB is NULL!";
                jres = getErrorResponce("Internal DB error.");
            }
            if (not jusr.empty()) {
                eraseMongoId(jusr);
                jres["resp"] = {
                    {"name", _name},
                    {"status", "ok"}
                };
                _token = "";
                jusr["token"] = _token;
                LockQuard l(_mutex);
                jusr = _db->insertUser(jusr);
            } else {
                LOG(FATAL) << "Value user is null. " << _token;
                jres = getErrorResponce("Can`t found user |" + _token + "|.");
            }
        } else {
            LOG(ERROR) << "This user is not loginned.";
            jres = getErrorResponce("This user is not loginned.");
        }
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetDevicesListCommand::GetDevicesListCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "get_devs", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetDevicesListCommand::~GetDevicesListCommand() {
    LOG(DEBUG);
}


Json GetDevicesListCommand::execute() {
    Json jres;
    if (_is_corrected) {
        size_t num_devices = 0;
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and *jtoken == _token) {
            auto jskip = _jdata.find("skip");
            auto jnum  = _jdata.find("num");
            if (not jskip->empty() and jskip->is_number() and not jnum->empty() and jnum->is_number()) {
                size_t skip = *jskip;
                size_t num = *jnum;
                auto jgeo_poly = _jdata.find("geo_poly");
                auto jgeo      = _jdata.find("geo");
                auto jradius   = _jdata.find("radius");
                auto jfilter   = _jdata.find("filter");
                if (jgeo_poly not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 4) {
                    double x = (*jgeo_poly)[0];
                    double y = (*jgeo_poly)[1];
                    double w = (*jgeo_poly)[2];
                    double h = (*jgeo_poly)[3];
                    Json jdevs;
                    _mutex.lock();
                    if (_db) {
                        jdevs = _db->getDevicesByGeo(x, y, w, h, skip, num);
                        num_devices = _db->getCollectionCount("", CONTROOLERS_COLLECTION_NAME);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                    eraseMongoId(jdevs);
                    jres = fillResponceData(jdevs);
                } else if (jgeo not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 2 and
                    jradius not_eq _jdata.end() and jradius->is_number()) {
                    double lo = (*jgeo)[0];
                    double la = (*jgeo)[1];
                    double r = *jradius;
                    Json jdevs;
                    _mutex.lock();
                    if (_db) {
                        jdevs = _db->getDevicesByGeo(lo, la, r, skip, num);
                        num_devices = _db->getCollectionCount("", CONTROOLERS_COLLECTION_NAME);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                    eraseMongoId(jdevs);
                    jres = fillResponceData(jdevs);
                } else if (jfilter not_eq _jdata.end()) {
                    DevicesIds dev_ids;
                    _mutex.lock();
                    if (_xdb) {
                        dev_ids = _xdb->getDevicesIndexes(*jfilter, skip, num);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "Index Search DB is NULL!";
                        jres = getErrorResponce("Internal XDB error.");
                    }
                    Json jdevs;
                    if (not dev_ids.empty()) {
                        _mutex.lock();
                        if (_db) {
                            jdevs = _db->getDevicesByIds(dev_ids);
                            num_devices = _db->getCollectionCount("", CONTROOLERS_COLLECTION_NAME);
                            _mutex.unlock();
                        } else {
                            _mutex.unlock();
                            LOG(FATAL) << "DB is NULL!";
                            jres = getErrorResponce("Internal DB error.");
                        }
                    }
                    eraseMongoId(jdevs);
                    jres = fillResponceData(jdevs);
                } else {
                    LOG(WARNING) << "Incorrect cmd: [" << _name << "] \"" << _jdata.dump() << "\"";
                    jres = getErrorResponce("Incorrect command.");
                }
            }
        } else {
            LOG(FATAL) << "Auth error!";
            jres = getErrorResponce("Invalid token.");
        }
        jres["count"] = num_devices;
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ActivateDeviceCommands::ActivateDeviceCommands(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "set_dev_status", js, mutex, snd_fn) {
    LOG(DEBUG);
}


ActivateDeviceCommands::~ActivateDeviceCommands() {
    LOG(DEBUG);
}


Json ActivateDeviceCommands::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and *jtoken == _token) {
            auto jdev_id = _jdata.find("dev_id");
            auto jstatus = _jdata.find("status");
            if (jdev_id not_eq _jdata.end() and jdev_id->is_string() and
                jstatus not_eq _jdata.end() and jstatus->is_string()) {
                _mutex.lock();
                if (_db) {
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
                        LOG(WARNING) << "Can`t find device: " << *jdev_id;
                        jres = getErrorResponce("Can`t find device: " + jdev_id->get<std::string>());
                    }
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
            LOG(WARNING) << "Auth error!";
            jres = getErrorResponce("Invalid token.");
        }
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetEventsListCommand::GetEventsListCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "get_events", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetEventsListCommand::~GetEventsListCommand() {
    LOG(DEBUG);
}


Json GetEventsListCommand::execute() {
    Json jres;
    if (_is_corrected) {
        size_t num_events = 0;
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and *jtoken == _token) {
            auto jskip = _jdata.find("skip");
            auto jnum  = _jdata.find("num");
            if (not jskip->empty() and jskip->is_number() and not jnum->empty() and jnum->is_number()) {
                size_t skip = *jskip;
                size_t num = *jnum;
                auto jgeo_poly = _jdata.find("geo_poly");
                auto jgeo      = _jdata.find("geo");
                auto jradius   = _jdata.find("radius");
                auto jfilter   = _jdata.find("filter");
                if (jgeo_poly not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 4) {
                    double x = (*jgeo_poly)[0];
                    double y = (*jgeo_poly)[1];
                    double w = (*jgeo_poly)[2];
                    double h = (*jgeo_poly)[3];
                    Json jevs;
                    _mutex.lock();
                    if (_db) {
                        jevs = _db->getEventsByGeo(x, y, w, h, skip, num);
                        num_events = _db->getCollectionCount("", EVENTS_COLLECTION_NAME);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                } else if (jgeo not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 2 and
                    jradius not_eq _jdata.end() and jradius->is_number()) {
                    double lo = (*jgeo)[0];
                    double la = (*jgeo)[1];
                    double r = *jradius;
                    Json jevs;
                    _mutex.lock();
                    if (_db) {
                        jevs = _db->getEventsByGeo(lo, la, r, skip, num);
                        num_events = _db->getCollectionCount("", EVENTS_COLLECTION_NAME);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                } else if (jfilter not_eq _jdata.end()) {
                    DevicesIds dev_ids;
                    _mutex.lock();
                    if (_xdb) {
                        dev_ids = _xdb->getEventsIndexes(*jfilter, skip, num);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "Index Search DB is NULL!";
                        jres = getErrorResponce("Internal XDB error.");
                    }
                    Json jevs;
                    if (not dev_ids.empty()) {
                        _mutex.lock();
                        if (_db) {
                            jevs = _db->getEventsByIds(dev_ids);
                            num_events = _db->getCollectionCount("", EVENTS_COLLECTION_NAME);
                            _mutex.unlock();
                        } else {
                            _mutex.unlock();
                            LOG(FATAL) << "DB is NULL!";
                            jres = getErrorResponce("Internal DB error.");
                        }
                    }
                    eraseMongoId(jevs);
                    jres = fillResponceData(jevs);
                } else {
                    LOG(WARNING) << "Incorrect cmd: [" << _name << "] \"" << _jdata.dump() << "\"";
                    jres = getErrorResponce("Incorrect command.");
                }
            }
        } else {
            LOG(FATAL) << "Auth error!";
            jres = getErrorResponce("Invalid token.");
        }
        jres["count"] = num_events;
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CreateReportCommand::CreateReportCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "get_report", js, mutex, snd_fn) {
    LOG(DEBUG);
}


CreateReportCommand::~CreateReportCommand() {
    LOG(DEBUG);
}


Json CreateReportCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_string() and *jtoken == _token) {
            auto jcoll = _jdata.find("coll");
            if (jcoll not_eq _jdata.end()) {
                DevicesIds dev_ids;
                _mutex.lock();
                if (_xdb) {
                    dev_ids = _xdb->getDevicesIndexes(*jcoll, 0, MAX_NUMBER_DEVICES_FOR_REPORT);
                    _mutex.unlock();
                } else {
                    _mutex.unlock();
                    LOG(FATAL) << "Index Search DB is NULL!";
                    jres = getErrorResponce("Internal XDB error.");
                }
                Json jdevs;
                if (not dev_ids.empty()) {
                    _mutex.lock();
                    if (_db) {
                        jdevs = _db->getDevicesByIds(dev_ids);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                }
                if (not jdevs.empty()) {
                    ReportGenerator rep_gen(jdevs);
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
            LOG(WARNING) << "Auth error!";
            jres = getErrorResponce("Invalid token.");
        }
        _snd_fn(jres.dump());
    }
    return jres;
}
