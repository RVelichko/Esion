#include "BaseWorker.hpp"
#include "Log.hpp"


using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


void BaseWorker::parseMessage(const std::string &msg, const ConcreteFn &fn) try {
    Json json;
    { ///< LOCK Розобрать полученную строку в json.
        LockQuard l(_mutex);
        json = Json::parse(msg);
    };
    if (not json["room_id"].is_null() and json["room_id"].is_string()) {
        /// Обработать привильный, в первом приближении, JSON.
        fn(json["room_id"], json);
    } else {
        LOG(ERROR) << "Can`t find value \"room_id\".";
    }
} catch(std::exception &e) {
    LOG(ERROR) << "Can`t parse recieved json: " << e.what();
}


std::string BaseWorker::getRoomId(size_t connection_id) {
    std::string room_id;
    { ///< LOCK
        LockQuard l(_mutex);
        ConnectionValuesIter iter = _connection_values.find(connection_id);
        if (iter not_eq _connection_values.end()) {
            ConnectionRoom *con_rom = dynamic_cast<ConnectionRoom*>(iter->second.get());
            if (con_rom) {
                room_id = con_rom->_room_id;
            }
        }
    }
    return room_id;
}


void BaseWorker::sendClose(size_t connection_id) {
    /// Получить ссылку на комнату и идентификатор подключения.
    std::string room_id = getRoomId(connection_id);
    if (not room_id.empty()) {
        SingleRoomMembers room_ch;
        { ///< LOCK
            LockQuard l(_mutex);
            room_ch = _room_controller->getRoom(room_id);
        }
        sendCloseTo(room_id, room_ch);
    }
}


void BaseWorker::onError(size_t connection_id, const boost::system::error_code &ec) {
    sendClose(connection_id);
    Worker::onError(connection_id, ec);
}


void BaseWorker::onClose(size_t connection_id, int status, const std::string &reason) {
    sendClose(connection_id);
    Worker::onClose(connection_id, status, reason);
}


BaseWorker::BaseWorker(std::mutex &mutex, const PSingleRoomController &room_controller, const PDbFacade& db)
    : Worker(mutex)
    , _room_controller(room_controller) 
    , _db(db)
{}


BaseWorker::~BaseWorker() 
{}
