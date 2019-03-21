/**
 * \brief  Простой websocket сервер комнаты 1x1 связи контроллера с веб страницей состояний счётчиков.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   17.09.2018
 */

#pragma once

#include <memory>

#include "json.hpp"
#include "Log.hpp"
#include "Timer.hpp"
#include "RoomController.hpp"
#include "WebSocketServer.hpp"


namespace device {

typedef nlohmann::json Json;
typedef Json RoomDataType; ///< Тип хранимых данных - json настройки устройств
typedef utils::RoomData<RoomDataType> RoomJsonData; ///< Тип структуры хелпера для типизации комнаты 1 : 1
typedef typename RoomJsonData::SingleMembers SingleRoomMembers; ///< Тип структуры комнаты 1 : 1
typedef utils::RoomController<RoomDataType, SingleRoomMembers> SingleRoomController; ///< Тип контроллера комнат
typedef std::shared_ptr<SingleRoomController> PSingleRoomController; ///< Тип умного указателя на контроллер комнат

typedef wsocket::Worker Worker; ///< Класс реализующий необходимые функции обработки данным сервером.
typedef wsocket::ConnectionValue ConnectionValue; ///< Структура с данными текущего подключения.
typedef wsocket::ConnectionValuesIter ConnectionValuesIter;
typedef wsocket::PConnectionValue PConnectionValue;


/**
 * Структура подключения с идентификатором комнаты
 */
typedef wsocket::ConnectionValue ConnectionValue;
struct ConnectionRoom : ConnectionValue {
    std::string _room_id;                   ///< Идентификатор комнаты
    PSingleRoomController _room_controller; ///< Контроллер комнаты нужен для удаления идентификатора подключения из комнаты
};


/**
 * Базовый клас, предоставляющий обобщённый набор функций
 */
class BaseWorker : public Worker {
protected:
    typedef std::function<void(const std::string&, const Json&)> ConcreteFn;

    PSingleRoomController _room_controller; ///< Контроллер комнаты типа 1 : 1

    virtual void sendCloseTo(const std::string &room_id, const SingleRoomMembers &room_ch) = 0;
    virtual void sendClose(size_t connection_id);
    virtual void onError(size_t connection_id, const boost::system::error_code &ec);
    virtual void onClose(size_t connection_id, int status, const std::string &reason);

    void parseMessage(const std::string &msg, const ConcreteFn &func);

    std::string getRoomId(size_t connection_id);

public:
    BaseWorker(std::mutex &mutex, const PSingleRoomController &room_controller);
    virtual ~BaseWorker();
};


/**
 * Клас, обрабатывающий подключения от устройства.
 */
class DevicePeerWorker : public BaseWorker {
    struct DeviceConnectionRoom : ConnectionRoom {
        virtual ~DeviceConnectionRoom();
    };
    typedef std::shared_ptr<DeviceConnectionRoom> PDeviceConnectionRoom;

    virtual PConnectionValue firstMessage(size_t connection_id, const std::string &msg);
    virtual bool lastMessage(const ConnectionValuesIter &iter, const std::string &msg);
    virtual void sendCloseTo(const std::string &room_id, const SingleRoomMembers &room_ch);

public:
    DevicePeerWorker(std::mutex &mutex, const PSingleRoomController &room_controller);
    virtual ~DevicePeerWorker();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/**
 * Клас, обрабатывающий подключения клиентских страниц.
 */
class ClientPagePeerWorker : public BaseWorker {
    struct ClientPageConnectionRoom : ConnectionRoom {
        virtual ~ClientPageConnectionRoom();
    };
    typedef std::shared_ptr<ClientPageConnectionRoom> PClientPageConnectionRoom;

    virtual PConnectionValue firstMessage(size_t connection_id, const std::string &msg);
    virtual bool lastMessage(const ConnectionValuesIter &iter, const std::string &msg);
    virtual void sendCloseTo(const std::string &room_id, const SingleRoomMembers &room_ch);

public:
    ClientPagePeerWorker(std::mutex &mutex, const PSingleRoomController &room_controller);
    virtual ~ClientPagePeerWorker();
};
} /// Device
