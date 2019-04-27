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
    std::string coll = json.value("coll", "");
    if (not coll.empty()) {
        _db->setCollection(coll);
    }
    if (not dev_id.empty()) {
        fn(dev_id, json);
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
    bool res = parseMessage(msg, [&](const std::string &dev_id, const Json &json) {
        /// Сохранить параметры комнаты
        con_val = std::make_shared<DeviceConnectionValue>();
        con_val->dev_id = dev_id;
        /// Загрузить данные в БД.
        Json jcounts = json.value("counters", Json());
        Json status = {{"status", "ok"}};
        if (not jcounts.empty() and jcounts.is_array()) {
            { /// LOCK
                LockQuard l(_mutex);
                if (_db) {
                    _db->insertDevice(json);
                } else {
                    status = {{"status", "DB is NULL"}};
                    LOG(FATAL) << "DB is NULL!";
                }
            }
            /// Если оператор уже подключён - оправить ему данные устройства.
            size_t operator_id = BaseWorker::getOperatorConnectionId();
            if (operator_id) {
                LOG(DEBUG) << "send to operator: " << operator_id << "; \"" <<  jcounts.dump() << "\"";
                _msg_fn(operator_id, json.dump(), WS_STRING_MESSAGE);
            }
        } else {
            LOG(WARNING) << "Empty counters. \"" <<  jcounts.dump() << "\"";
            status = {{"status", "Empty counters"}};
        }
        /// Отправить Status устройству.
        LOG(DEBUG) << "send to device: " << connection_id << "; \"" <<  status.dump() << "\"";
        _msg_fn(connection_id, status.dump(), WS_STRING_MESSAGE);
    });
    if (not res) {
        Json status = {{"status", "error"}};
        _msg_fn(connection_id, status.dump(), WS_STRING_MESSAGE);
    }
    return con_val;
}


bool DevicePeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    return true;
}


void DevicePeerWorker::sendClose(size_t connection_id)
{}


DevicePeerWorker::DevicePeerWorker(std::mutex &mutex, const PDbFacade& db)
    : BaseWorker(mutex, db)
{}


DevicePeerWorker::~DevicePeerWorker() 
{}
