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
    std::string dev_id = json.value("id", "");
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
        auto jdev_id = json.find("id");
        if (jdev_id not_eq json.end() and jdev_id->is_string()) {
            con_val->dev_id = *jdev_id;
            /// Загрузить данные в БД.
            auto jcounts = json.find("counters");
            auto jcoll = json.find("coll");
            if (jcounts not_eq json.end() and jcounts->is_array() and
                jcoll not_eq json.end() and jcoll->is_string()) {
                Json jdev;
                _mutex.lock();
                if (_db) {
                    jdev = _db->getDevice(*jdev_id);
                    _mutex.unlock();
                    if (jdev.empty()) {
                        jdev = json;
                        auto jgeo = _geo_req->request(*jcoll);
                        jdev["geo"] = jgeo["point"];
                        jdev["status"] = "not_active";
                    } else {
                        auto jgeo = jdev["geo"];
                        auto jstatus =  jdev["status"];
                        jdev = json;
                        jdev["geo"] = jgeo;
                        jdev["status"] = jstatus;
                    }
                    /// Зафиксировать время последнего обновления.
                    time_t rawtime;
                    time(&rawtime);
                    jdev["update_time"] = rawtime;
                    LockQuard l(_mutex);
                    _db->insertDevice(jdev);
                } else {
                    _mutex.unlock();
                    jstatus = {{"status", "DB is NULL"}};
                    LOG(FATAL) << "DB is NULL!";
                }
            } else {
                LOG(WARNING) << "Empty counters. \"" <<  jcounts->dump() << "\"";
                jstatus = {{"status", "Empty counters"}};
            }
        } else {
            LOG(ERROR) << "Can`t find device id in receaved data!";
            jstatus = {{"status", "Empty dev_id"}};
        }
        /// Отправить Status устройству.
        LOG(DEBUG) << "send to device: " << connection_id << "; \"" <<  jstatus.dump() << "\"";
        _msg_fn(connection_id, jstatus.dump(), WS_STRING_MESSAGE);
    });
    if (not res) {
        jstatus = {{"status", "error"}};
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
    , _geo_req(new GeoRequester(ymap_api_key)) {
    LOG(DEBUG);
}


DevicePeerWorker::~DevicePeerWorker() {
    LOG(DEBUG);
}
