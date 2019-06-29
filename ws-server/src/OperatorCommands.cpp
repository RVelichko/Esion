#include "Log.hpp"
#include "OperatorCommands.hpp"


using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


bool BaseCommand::executeByName(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
    static CommandsExecuters cmd_execs = {
       {"auth", [](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = AuthorizeCommand(token, js, mutex, db, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"get_devs",[](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = GetDevicesListCommand(token, js, mutex, db, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"set_dev_geo",[](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = DeviceGeopositionCommand(token, js, mutex, db, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"set_dev_status",[](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = ActivateDeviceCommands(token, js, mutex, db, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"get_events",[](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = GetEventsListCommand(token, js, mutex, db, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"set_event_geo",[](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = EventGeopositionCommand(token, js, mutex, db, snd_fn);
            if (cmd) {
                cmd.execute();
                return true;
            }
            return false;
       }},
       {"get_report",[](size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn) {
            auto cmd = CreateReportCommand(token, js, mutex, db, snd_fn);
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
            return exec_fn(token, js, mutex, db, snd_fn);
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


BaseCommand::BaseCommand(size_t token, const std::string& name,
                         const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : JsonCommand(name, js)
    , _token(token)
    , _mutex(mutex)
    , _db(db)
    , _snd_fn(snd_fn)
{}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AuthorizeCommand::AuthorizeCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "auth", js, mutex, db, snd_fn) {
    LOG(DEBUG);
}


AuthorizeCommand::~AuthorizeCommand() {
    LOG(DEBUG);
}


Json AuthorizeCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto juser = _jdata.value("user", Json());
        auto jpswd = _jdata.value("pswd", Json());
        if (not juser.empty() and not jpswd.empty()) {
            auto user = _jdata.value("user", "");
            auto pswd = _jdata.value("pswd", "");
            if (user.empty()) {
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
                        jusr = _db->findUser(user, pswd);
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
                    jres = getErrorResponce("Can`t found user |" + user + " | " + pswd + "|.");
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


GetDevicesListCommand::GetDevicesListCommand(size_t token, const Json& js,
                                             std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "get_devs", js, mutex, db, snd_fn) {
    LOG(DEBUG);
}


GetDevicesListCommand::~GetDevicesListCommand() {
    LOG(DEBUG);
}


Json GetDevicesListCommand::execute() {
    Json jres;
    if (_is_corrected) {
        auto juser_id = _jdata.value("user_id", Json()); ///< Идентификатор пользователя сервиса.
        auto jskip    = _jdata.value("skip", Json());    ///< Количество пропускаемых записей в списке найденных устройств.
        auto jnum     = _jdata.value("num", Json());     ///< Количество возвращаемых устройств.
        auto jfilter  = _jdata.value("filter", Json());  ///< Любая строка, которой могут соответствовать строки в БД.
        if (not juser_id.empty() and not jskip.empty() and not jnum.empty() and not jfilter.empty()) {
            size_t user_id = _jdata.value("user_id", Json());
            if (_token == user_id) {
                size_t skip = static_cast<size_t>(_jdata.value("skip", 0));
                size_t num = static_cast<size_t>(_jdata.value("num", 10));
                std::string filter = _jdata.value("filter", "");
                auto fill_resp_fn = [&](const Json& jdevs) {
                    /// Удалить идентификатор для БД.
                    for (auto jdev : jdevs) {
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
                if (not filter.empty()) {
                    Json jds;
                    { /// LOCK Для доступа в БД.
                        _mutex.lock();
                        if (_db) {
                            jds = _db->findDevices(filter, num, skip);
                            _mutex.unlock();
                        } else {
                            _mutex.unlock();
                            LOG(FATAL) << "DB is NULL!";
                            jres = getErrorResponce("Internal DB error.");
                        }
                    };
                    fill_resp_fn(jds);
                } else {
                    Json jds;
                    { /// LOCK Для доступа в БД.
                        _mutex.lock();
                        if (_db) {
                            jds = _db->findDevices(filter, num, skip);
                            _mutex.unlock();
                        } else {
                            _mutex.unlock();
                            LOG(FATAL) << "DB is NULL!";
                            jres = getErrorResponce("Internal DB error.");
                        }
                    };
                    fill_resp_fn(jds);
                }
            } else {
                LOG(FATAL) << "Auth error!";
                jres = getErrorResponce("Invalid user_id.");
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


DeviceGeopositionCommand::DeviceGeopositionCommand(size_t token, const Json& js,
                                                   std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "set_dev_geo", js, mutex, db, snd_fn) {
    LOG(DEBUG);
}


DeviceGeopositionCommand::~DeviceGeopositionCommand() {
    LOG(DEBUG);
}


Json DeviceGeopositionCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is not implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ActivateDeviceCommands::ActivateDeviceCommands(size_t token, const Json& js,
                                               std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "set_dev_status", js, mutex, db, snd_fn) {
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


GetEventsListCommand::GetEventsListCommand(size_t token, const Json& js,
                                           std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "get_events", js, mutex, db, snd_fn) {
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


EventGeopositionCommand::EventGeopositionCommand(size_t token, const Json& js,
                                                 std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "set_event_geo", js, mutex, db, snd_fn) {
    LOG(DEBUG);
}


EventGeopositionCommand::~EventGeopositionCommand() {
    LOG(DEBUG);
}

Json EventGeopositionCommand::execute() {
    Json jres;
    if (_is_corrected) {
        jres = getErrorResponce("Is not implemented yet.");
        _snd_fn(jres.dump());
    }
    return jres;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CreateReportCommand::CreateReportCommand(size_t token, const Json& js,
                                         std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn)
    : BaseCommand(token, "get_report", js, mutex, db, snd_fn) {
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
