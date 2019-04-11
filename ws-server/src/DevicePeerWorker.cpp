#include "Log.hpp"
#include "DevicePeerWorker.hpp"

using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;


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


DevicePeerWorker::DevicePeerWorker(std::mutex &mutex, const PSingleRoomController &room_controller, const PDbFacade& db)
    : BaseWorker(mutex, room_controller, db) 
{}


DevicePeerWorker::~DevicePeerWorker() 
{}
