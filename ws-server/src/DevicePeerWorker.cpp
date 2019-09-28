#include "Log.hpp"
#include "DevicePeerWorker.hpp"

using namespace server;


struct DeviceConnectionValue : ConnectionValue {
    std::string dev_id;
};


typedef std::lock_guard<std::mutex> LockQuard;
typedef std::shared_ptr<DeviceConnectionValue> PDeviceConnectionValue;


bool DevicePeerWorker::parseMessage(const std::string &msg, const ConcreteFn &fn) try {
    Json json;
    { ///< LOCK Розобрать полученную строку в json.
        LockQuard l(_mutex);
        json = Json::parse(msg);
    };
    std::string dev_id = json.value("dev_id", "");
    if (not dev_id.empty()) {
        fn(json);
    } else {
        LOG(ERROR) << "Can`t find value \"id\".";
        return false;
    }
    return true;
} catch(std::exception &e) {
    LOG(ERROR) << "Can`t parse recieved json: " << e.what();
    return false;
}


PConnectionValue DevicePeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "con_id = " << connection_id << "; " << msg;
    PDeviceConnectionValue con_val;
    Json jstatus = {{"status", "ok"}};
    bool res = parseMessage(msg, [&](const Json &json) {
        /// Сохранить параметры комнаты
        con_val = std::make_shared<DeviceConnectionValue>();
        auto jdev_id = json.find("dev_id");
        if (jdev_id not_eq json.end() and jdev_id->is_string()) {
            con_val->dev_id = *jdev_id;
            /// Получить обязательные параметры.
            auto japmt = json.find("apmt");
            auto jcoll = json.find("coll");
            auto jcoll_id = json.find("coll_id");
            auto jin_countsp = json.find("counters");
            if (jin_countsp not_eq json.end() and jin_countsp->is_array() and
                jcoll not_eq json.end() and jcoll->is_string() and
                jcoll_id not_eq json.end() and jcoll_id->is_string() and
                japmt not_eq json.end() and japmt->is_number()) {
                std::string geo_addr = json["coll"];
                Json jin_counts = json["counters"];
                Json jdev;
                _mutex.lock();
                if (_db) {
                    jdev = _db->getDevice(*jdev_id);
                    _mutex.unlock();
                    time_t rawtime;
                    time(&rawtime);
                    Json jtype_none = {
                        {"type", "none"}
                    };
                    bool is_new = jdev.empty();
                    if (is_new) {
                        jdev = json; /// Зафиксировать принятые поля нового устройства.
                        auto jgeo = _geo_req->request(geo_addr);
                        jdev["geo"] = jgeo["point"];
                        jdev["status"] = "not_active";
                        jdev["start_time"] = rawtime; /// Время первого выхода в эфир.
                        /// Заполнить показания для нового устройства.
                        Json jcounters;
                        for (auto jc : jin_counts) {
                            Json jcounter_line = jtype_none;
                            if (jc.find("type") not_eq jc.end() and jc["type"] not_eq "none") {
                                jcounter_line["type"] = jc["type"];
                                double cm = 0.0;
                                int cnt = 0;
                                int ucnt = 1;
                                if (jc.find("count") not_eq jc.end() and jc.find("unit_count") not_eq jc.end()) {
                                    cnt = jc["count"];
                                    ucnt = jc["unit_count"];
                                    cm = static_cast<double>(cnt * ucnt) * 0.001;
                                }
                                Json jcubic_meter;
                                jcubic_meter.push_back({
                                    {"cm", cm},
                                    {"t", rawtime}
                                });
                                if (jc.find("serial") not_eq jc.end()) {
                                    jcounter_line["serial"] = jc["serial"];
                                }
                                if (jc.find("verify_date") not_eq jc.end()) {
                                    jcounter_line["verify_date"] = jc["verify_date"];
                                }
                                if (jc.find("unit") not_eq jc.end()) {
                                    jcounter_line["unit"] = jc["unit"];
                                }
                                jcounter_line["count"] = cnt;
                                jcounter_line["cubic_meter"] = jcubic_meter;
                                jcounter_line["unit_type"] = "Импульс";
                                jcounter_line["unit_count"] = ucnt;
                            }
                            jcounters.push_back(jcounter_line);
                        }
                        jdev["counters"] = jcounters;
                    } else {
                        try {
                            Json jgeo;
                            if (jdev["geo"][0] == 0.0 or jdev["geo"][1] == 0.0) {
                                jgeo = _geo_req->request(geo_addr);
                            } else {
                                jgeo = jdev["geo"];
                            }
                            auto jstatus = jdev["status"];
                            auto jdev_counters = jdev["counters"];
                            jdev = json;
                            jdev["geo"] = jgeo;
                            jdev["status"] = jstatus;
                            /// Обновить показания.
                            for (size_t i = 0; i < jdev_counters.size(); ++i) {
                                Json jin_cnt;
                                if (i < jin_counts.size()) {
                                    jin_cnt = jin_counts[i];
                                }
                                Json& jdev_cnt = jdev_counters[i];
                                if (jdev_cnt.find("type") not_eq jdev_cnt.end() and jdev_cnt["type"] not_eq "none" and
                                    jin_cnt.find("type") not_eq jin_cnt.end() and jin_cnt["type"] not_eq "none") {
                                    int cnt = 0;
                                    double cm = 0.0;
                                    if (jin_cnt.find("count") not_eq jin_cnt.end() and
                                        jin_cnt.find("unit_count") not_eq jin_cnt.end()) {
                                        cnt = jin_cnt["count"];
                                        int ucnt = jin_cnt["unit_count"];
                                        cm = static_cast<double>(cnt * ucnt) * 0.001;
                                    }
                                    Json jcm = {
                                        {"cm", cm},
                                        {"t", rawtime}
                                    };
                                    jdev_cnt["count"] = cnt;
                                    jdev_cnt["cubic_meter"].push_back(jcm);
                                }
                            }
                            jdev["counters"] = jdev_counters;
                        } catch(const std::exception& e) {
                            LOG(FATAL) << "In DB device is corupted!";
                        }
                    }
                    /// Зафиксировать время последнего обновления.
                    jdev["update_time"] = rawtime;
                    /// Зафиксировать процент нарпряжения.
                    double vperc = 0.0;
                    if (json.find("voltage") not_eq json.end() and json["voltage"].is_string()) {
                        vperc = std::stod(json.value("voltage", "0.0").c_str()) / 100.0;
                        if (1 <= vperc) {
                            vperc = 0.0;
                        }
                    }
                    jdev["voltage_perc"] = vperc;
                    double voltage = 0.0;
                    if (json.find("power_type") not_eq json.end() and json["power_type"].is_string()) {
                        if (json["power_type"] == "LiOn [3.8V]") {
                            voltage = 3.8 * vperc;
                        } else if (json["power_type"] == "4AA [6V]") {
                            voltage = 6.0 * vperc;
                        }
                    }
                    jdev["voltage"] = voltage;
                    /// Загрузить данные в БД.
                    LockQuard l(_mutex);
                    _db->insertDevice(jdev);
                } else {
                    _mutex.unlock();
                    jstatus = {
                        {"status", "DB is NULL"}
                    };
                    LOG(FATAL) << "DB is NULL!";
                }
            } else {
                LOG(WARNING) << "Empty counters.";
                jstatus = {
                    {"status", "Empty counters"}
                };
            }
        } else {
            LOG(ERROR) << "Can`t find device id in receaved data!";
            jstatus = {
                {"status", "Empty dev_id"}
            };
        }
        /// Отправить Status устройству.
        LOG(DEBUG) << "send to device: " << connection_id << "; \"" <<  jstatus.dump() << "\"";
        _msg_fn(connection_id, jstatus.dump(), WS_STRING_MESSAGE);
    });
    if (not res) {
        jstatus = {
            {"status", "error"}
        };
        _msg_fn(connection_id, jstatus.dump(), WS_STRING_MESSAGE);
    }
    return con_val;
}


bool DevicePeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    return true;
}


void DevicePeerWorker::sendClose(size_t connection_id)
{}


DevicePeerWorker::DevicePeerWorker(std::mutex &mutex, const PDbFacade& db, const std::string& ymap_api_key)
    : BaseWorker(mutex)
    , _db(db)
    , _geo_req(std::make_shared<GeoRequester>(ymap_api_key)) {
    LOG(DEBUG);
}


DevicePeerWorker::~DevicePeerWorker() {
    LOG(DEBUG);
}
