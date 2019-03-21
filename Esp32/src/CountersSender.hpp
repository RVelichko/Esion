#pragma once 

#include <string>
#include <memory>

#include <Arduino.h>
#include <WebSocketsClient.h>


/**
 * \brief Класс реализует функции отправки по websocket данных на сервер.
 */ 
class CountersSender {
    WebSocketsClient _wsocket;
    String _addr; 
    uint16_t _port;
    String _path;
    String _room_id;
    String _json;
    bool _is_recv;
    bool _is_err;

public:
    explicit CountersSender(const String& addr, uint16_t port, const String &path, const String& room_id, const String& json);
    ~CountersSender();

    String getData();
    void sendData();
    void setError();

    void recvState(const String &srecv);

    void execute();

    bool update();
};