#include "Log.hpp"
#include "OperatorPeerWorker.hpp"

using namespace server;

typedef std::lock_guard<std::mutex> LockQuard;



OperatorPeerWorker::OperatorPageConnectionRoom::~OperatorPageConnectionRoom() {
    if (_room_controller) {
        _room_controller->deleteFromRoom(_room_id, std::make_tuple(static_cast<size_t>(0), _connection_id));
    }
}


PConnectionValue OperatorPeerWorker::firstMessage(size_t connection_id, const std::string &msg) {
    LOG(DEBUG) << "conid = " << connection_id << "; " << msg;
    POperatorPageConnectionRoom con_room;
    auto create_func = [&](const std::string &room_id, const Json &json) {
        con_room = std::make_shared<OperatorPageConnectionRoom>();
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


bool OperatorPeerWorker::lastMessage(const ConnectionValuesIter &iter, const std::string &msg) {
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


void OperatorPeerWorker::sendCloseTo(const std::string &room_id, const SingleRoomMembers &room_ch) {
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


OperatorPeerWorker::OperatorPeerWorker(std::mutex &mutex, const PSingleRoomController &room_controller, const PDbFacade& db)
    : BaseWorker(mutex, room_controller, db) 
{}


OperatorPeerWorker::~OperatorPeerWorker() 
{}
