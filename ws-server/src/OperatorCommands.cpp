#include "Log.hpp"
//#include "SearchIndexClient.hpp"
#include "OperatorCommands.hpp"


using namespace server;
//using namespace sindex;

typedef std::lock_guard<std::mutex> LockQuard;
typedef sindex::IndexIds DevicesIds;
//typedef std::unique_ptr<SearchIndexClient> PSearchIndexClient;


//std::string BaseCommand::_si_url;
//std::string BaseCommand::_si_login;
//std::string BaseCommand::_si_pswd;
//
//
//SearchIndexClient* GetSIndexClient() {
//    static PSearchIndexClient si_client = PSearchIndexClient(new SearchIndexClient(_si_url, _si_login, _si_pswd));
//}


PDbFacade BaseCommand::_db;
PIndexDbFacade BaseCommand::_xdb;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool BaseCommand::executeByName(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
    static CommandsExecuters cmd_execs = {
        {"auth", [](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = AuthorizeCommand(token, js, mutex, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
        }},
        {"get_devs",[](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = GetDevicesListCommand(token, js, mutex, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
        }},
        {"set_dev_status",[](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = ActivateDeviceCommands(token, js, mutex, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
        }},
        {"get_events",[](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = GetEventsListCommand(token, js, mutex, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
        }},
        {"get_report",[](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
            auto cmd = CreateReportCommand(token, js, mutex, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
        }}
        //}},
        //{"set_event_geo",[](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
        //     auto cmd = EventGeopositionCommand(token, js, mutex, snd_fn);
        //     if (cmd) {
        //         cmd.execute();
        //         return true;
        //     }
        //     return false;
        //}},
        //{"set_dev_geo",[](size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn) {
        //     auto cmd = DeviceGeopositionCommand(token, js, mutex, snd_fn);
        //     if (cmd) {
        //         cmd.execute();
        //         return true;
        //     }
        //     return false;
        //}}
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
    auto j_id = js.find("_id");
    if (j_id not_eq js.end()) {
        js.erase("_id");
    }
}


BaseCommand::BaseCommand(size_t token, const std::string& name, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : JsonCommand(name, js)
    , _token(token)
    , _mutex(mutex)
    , _snd_fn(snd_fn)
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AuthorizeCommand::AuthorizeCommand(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "auth", js, mutex, snd_fn) {
    LOG(DEBUG);
}


AuthorizeCommand::~AuthorizeCommand() {
    LOG(DEBUG);
}


Json AuthorizeCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jlogin = _jdata.value("login", Json());
        auto jpswd = _jdata.value("pswd", Json());
        if (not jlogin.empty() and not jpswd.empty()) {
            auto login = _jdata.value("login", "");
            auto pswd = _jdata.value("pswd", "");
            if (login.empty()) {
                LOG(FATAL) << "Value user is null.";
                jres = getErrorResponce("Value user is null.");
            } else if (pswd.empty()) {
                LOG(FATAL) << "Value user is null.";
                jres = getErrorResponce("Value password is null.");
            } else {
                Json jusr;
                { /// LOCK Для доступа к БД.
                    _mutex.lock();
                    if (_db) {
                        jusr = _db->findUser(login, pswd);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                };
                if (not jusr.empty()) {
                    eraseMongoId(jusr);
                    jres["resp"] = {
                        {"name", _name},
                        {"status", "ok"},
                        {"token", _token}
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


GetDevicesListCommand::GetDevicesListCommand(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "get_devs", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetDevicesListCommand::~GetDevicesListCommand() {
    LOG(DEBUG);
}


Json GetDevicesListCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto jtoken  = _jdata.find("token");
        if (jtoken not_eq _jdata.end() and jtoken->is_number() and *jtoken == _token) {
            auto fill_resp_fn = [&](Json& jdevs) {
                /// Удалить идентификатор для БД.
                for (auto& jdev : jdevs) {
                    eraseMongoId(jdev);
                }
                /// Заполнить переменную ответа.
                jres = {
                  {"resp", {
                    {"name", _name},
                    {"status", "ok"},
                    {"data", jdevs}
                  }}
                };
                LOG(DEBUG) << jres;
            };
            auto jskip = _jdata.find("skip");
            auto jnum  = _jdata.find("num");
            if (not jskip->empty() and jskip->is_number() and not jnum->empty() and jnum->is_number()) {
                size_t skip = *jskip;
                size_t num = *jnum;
                auto jgeo    = _jdata.find("geo");
                auto jradius = _jdata.find("radius");
                auto jfilter = _jdata.find("filter");
                if (jgeo not_eq _jdata.end() and jgeo->is_array() and jgeo->size() == 2 and
                    jradius not_eq _jdata.end() and jradius->is_number()) {
                    double lo = (*jgeo)[0];
                    double la = (*jgeo)[1];
                    double r = *jradius;
                    Json jdevs;
                    _mutex.lock();
                    if (_db) {
                        jdevs = _db->getDevicesByGeo(lo, la, r, skip, num);
                        _mutex.unlock();
                    } else {
                        _mutex.unlock();
                        LOG(FATAL) << "DB is NULL!";
                        jres = getErrorResponce("Internal DB error.");
                    }
                    fill_resp_fn(jdevs);
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
                            _mutex.unlock();
                        } else {
                            _mutex.unlock();
                            LOG(FATAL) << "DB is NULL!";
                            jres = getErrorResponce("Internal DB error.");
                        }
                    }
                    fill_resp_fn(jdevs);
                } else {
                    LOG(WARNING) << "Incorrect cmd: [" << _name << "] \"" << _jdata.dump() << "\"";
                    jres = getErrorResponce("Incorrect command.");
                }
            }
        } else {
            LOG(FATAL) << "Auth error!";
            jres = getErrorResponce("Invalid token.");
        }
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ActivateDeviceCommands::ActivateDeviceCommands(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "set_dev_status", js, mutex, snd_fn) {
    LOG(DEBUG);
}


ActivateDeviceCommands::~ActivateDeviceCommands() {
    LOG(DEBUG);
}


Json ActivateDeviceCommands::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is not implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


GetEventsListCommand::GetEventsListCommand(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "get_events", js, mutex, snd_fn) {
    LOG(DEBUG);
}


GetEventsListCommand::~GetEventsListCommand() {
    LOG(DEBUG);
}


Json GetEventsListCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is not implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CreateReportCommand::CreateReportCommand(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
    : BaseCommand(token, "get_report", js, mutex, snd_fn) {
    LOG(DEBUG);
}


CreateReportCommand::~CreateReportCommand() {
    LOG(DEBUG);
}


Json CreateReportCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is not implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//EventGeopositionCommand::EventGeopositionCommand(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
//    : BaseCommand(token, "set_event_geo", js, mutex, snd_fn) {
//    LOG(DEBUG);
//}
//
//
//EventGeopositionCommand::~EventGeopositionCommand() {
//    LOG(DEBUG);
//}
//
//Json EventGeopositionCommand::execute() {
//    Json jres;
//    if (_is_corrected) {
//        jres = getErrorResponce("Is not implemented yet.");
//        _snd_fn(jres.dump());
//    }
//    return jres;
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//DeviceGeopositionCommand::DeviceGeopositionCommand(size_t token, const Json& js, std::mutex& mutex, const SendFn& snd_fn)
//    : BaseCommand(token, "set_dev_geo", js, mutex, snd_fn) {
//    LOG(DEBUG);
//}
//
//
//DeviceGeopositionCommand::~DeviceGeopositionCommand() {
//    LOG(DEBUG);
//}
//
//
//Json DeviceGeopositionCommand::execute() {
//    Json jres;
//    if (_is_corrected) {
//        jres = getErrorResponce("Is not implemented yet.");
//        _snd_fn(jres.dump());
//    }
//    return jres;
//}
