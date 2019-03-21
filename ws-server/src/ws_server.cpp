#include <exception>

#include "json.hpp"
#include "Log.hpp"
#include "Timer.hpp"
#include "RoomController.hpp"
#include "WebSocketServer.hpp"

#include "ws_server.hpp"


using namespace device;

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


BaseWorker::BaseWorker(std::mutex &mutex, const PSingleRoomController &room_controller)
    : Worker(mutex)
    , _room_controller(room_controller) {}


BaseWorker::~BaseWorker() {}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DevicePeerWorker::DeviceConnectionRoom::~DeviceConnectionRoom() {
    //if (_room_controller) {
    //    _room_controller->deleteFromRoom(_room_id, std::make_tuple(_connection_id, static_cast<size_t>(0)));
    //}
}


PConnectionValue DevicePeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "con_id = " << connection_id << "; " << msg;
    PDeviceConnectionRoom con_room;
    auto create_func = [&](const std::string &room_id, const Json &json) {
        /// Сохранить параметры комнаты
        con_room = std::make_shared<DeviceConnectionRoom>();
        con_room->_room_id = room_id;
        con_room->_room_controller = _room_controller;
        /// Создать ссылку на комнату и связать с ней идентификатор подключения
        size_t operator_id = 0;
        SingleRoomMembers room_ch;
        { ///< LOCK
            LockQuard l(_mutex);
            room_ch = _room_controller->getRoom(room_id);
            operator_id = std::get<1>(room_ch);    
            room_ch = _room_controller->addToRoom(room_id, std::make_tuple(connection_id, operator_id));
        }
        LOG(DEBUG) << "room_id: " << room_id << "; {" << connection_id << "," <<  operator_id << "}";
        //Json jmsg = json["msg"].get<Json>();
        /// Если оператор уже подключён - оправить ему настройки источника
        if (operator_id) {
            LOG(DEBUG) << "send to operator: " << operator_id << "; \"" <<  json.dump() << "\"";
            _msg_fn(operator_id, json.dump(), WS_STRING_MESSAGE);
        }
        /// Сохранить конфигурационные данные.
        _room_controller->setRoomData<RoomDataType>(room_id, json);
        /// Отправить Ok устройству.
        Json ok = {
            {"room_id", room_id},
            {"status", "ok"}
        };
        LOG(DEBUG) << "send to device: " << connection_id << "; \"" <<  ok.dump() << "\"";
        _msg_fn(connection_id, ok.dump(), WS_STRING_MESSAGE);
    };
    parseMessage(msg, create_func);
    if (con_room) {
        LOG(DEBUG) << "room_id: " << con_room->_room_id << "; [" << connection_id << "]";
    }
    return con_room;
}


bool DevicePeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    size_t connection_id = iter->first;
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    auto send_func = [=](const std::string &room_id, const Json &json) {
        /// Получить ссылку на комнату и идентификатор подключения
        SingleRoomMembers room_ch;
        { ///< LOCK
            LockQuard l(_mutex);
            room_ch = _room_controller->getRoom(room_id);
        }
        size_t operator_id = std::get<1>(room_ch);
        LOG(DEBUG) << "room_id: " << room_id << "; {" << connection_id << "," <<  operator_id << "}";
        /// Если оператор уже подключён - оправить ему сообщение
        if (operator_id) {
            LOG(DEBUG) << "send to operator: " << operator_id << "; \"" <<  msg << "\"";
            _msg_fn(operator_id, msg, WS_STRING_MESSAGE);
        }
    };
    parseMessage(msg, send_func);
    return false;
}


void DevicePeerWorker::sendCloseTo(const std::string &room_id, const SingleRoomMembers &room_ch) {
    size_t operator_id = std::get<1>(room_ch);
    if (operator_id) {
        Json j = {
            {"room_id", room_id}, {
                "msg", {
                    {"cmd", "close"}
                }
            }
        };
        _msg_fn(operator_id, j.dump(), WS_STRING_MESSAGE);
    }
}


DevicePeerWorker::DevicePeerWorker(std::mutex &mutex, const PSingleRoomController &room_controller)
    : BaseWorker(mutex, room_controller) {}


DevicePeerWorker::~DevicePeerWorker() {}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ClientPagePeerWorker::ClientPageConnectionRoom::~ClientPageConnectionRoom() {
    if (_room_controller) {
        _room_controller->deleteFromRoom(_room_id, std::make_tuple(static_cast<size_t>(0), _connection_id));
    }
}


PConnectionValue ClientPagePeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    PClientPageConnectionRoom con_room;
    auto create_func = [&](const std::string &room_id, const Json &json) {
        con_room = std::make_shared<ClientPageConnectionRoom>();
        con_room->_room_id = room_id;
        con_room->_room_controller = _room_controller;
        SingleRoomMembers room_ch;
        { ///< LOCK
            LockQuard l(_mutex);
            room_ch = _room_controller->getRoom(room_id);
        }
        size_t old_operator_id = std::get<1>(room_ch);
        /// Если оператор уже подключён - оправить ему команду на отключение
        if (old_operator_id) {
            LOG(DEBUG) << "send to OLD operator: " << old_operator_id << "; \"CLOSE\"";
            Json j  = {
                {"room_id", room_id}, {
                    "msg", {
                        {"cmd", "close_old"}
                    }
                }
            };
            _msg_fn(old_operator_id, j.dump(), WS_STRING_MESSAGE);
        }
        /// Связать ссылку на комнату с идентификатором подключения
        { ///< LOCK
            LockQuard l(_mutex);
            room_ch = _room_controller->addToRoom(room_id, std::make_tuple(static_cast<size_t>(0), connection_id));
        }
        Json dev_json = std::get<2>(room_ch);
        LOG(DEBUG) << "send to operator: " << connection_id << "; \"" <<  dev_json << "\"";
        _msg_fn(connection_id, dev_json.dump(), WS_STRING_MESSAGE);
    };
    parseMessage(msg, create_func);
    if (con_room) {
        LOG(DEBUG) << "room_id: " << con_room->_room_id << "; [" << connection_id << "]";
    }
    return con_room;
}


bool ClientPagePeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
    size_t connection_id = iter->first;
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    auto send_func = [=](const std::string &room_id, const Json &json) {
        SingleRoomMembers room_ch;
        { ///< LOCK
            LockQuard l(_mutex);
            room_ch = _room_controller->getRoom(room_id);
        }
        size_t robot_id = std::get<0>(room_ch);
        LOG(DEBUG) << "room_id: " << room_id << "; {" << robot_id << "," <<  connection_id << "}";
        /// Если робот уже подключён - оправить сообщение от оператора
        if (robot_id not_eq 0) {
            LOG(DEBUG) << "send to robot: " << robot_id << "; \"" <<  msg << "\"";
            _msg_fn(robot_id, msg, WS_STRING_MESSAGE);
        }
    };
    parseMessage(msg, send_func);
    return false;
}


void ClientPagePeerWorker::sendCloseTo(const std::string &room_id, const SingleRoomMembers &room_ch) {
    size_t robot_id = std::get<0>(room_ch);
    if (robot_id) {
        Json j = {
            {"room_id", room_id}, {
                "msg", {
                    {"cmd", "close"}
                }
            }
        };
        _msg_fn(robot_id, j.dump(), WS_STRING_MESSAGE);
    }
}


ClientPagePeerWorker::ClientPagePeerWorker(std::mutex &mutex, const PSingleRoomController &room_controller)
    : BaseWorker(mutex, room_controller) {}


ClientPagePeerWorker::~ClientPagePeerWorker() {}
