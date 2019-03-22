#pragma once 

#include <string>
#include <memory>

#include <Arduino.h>
#include <WebSocketClient.h>


/**
 * \brief Класс реализует функции отправки по websocket данных на сервер.
 */ 
class CountersSender {
    WebSocketClient _wsocket; ///< Объект обслуживания websocket протокола [RFC6455].
    String _addr;             ///< Адрес сервера обслуживания.
    uint16_t _port;           ///< Порт подуключения сокета.
    String _path;             ///< REST точка подключения.
    String _room_id;          ///< Идентификатор комнаты, к которой привязывается данное устройство.
    String _json;             ///< Отправляемые на сервер обслуживания данные
    bool _is_recv;
    bool _is_err;

public:
    explicit CountersSender(const String& addr, uint16_t port, const String &path, const String& room_id, const String& json);
    ~CountersSender();

    /**
     * \brief Метод реализует функции отправки по websocket данных на сервер.
     * \param srecv  Принятые от сервера данные в виде строки.
     */ 
    void recvState(const String &srecv);

    /**
     * \brief Метод выполняет подключение к серверу обслуживания и отправлку конфигурации и счётчиков.
     */ 
    void execute();

    /**
     * \brief Метод выполняет чтение буфера принятых данных.
     */ 
    bool update();
};