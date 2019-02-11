#pragma once 

#include <string>
#include <memory>

#include <Arduino.h>
#include <WebSocketsClient.h>

class CountersSender;
typedef std::unique_ptr<CountersSender> PCountersSender;


/**
 * \brief Класс реализует функции отправки по websocket данных на сервер.
 */ 
class CountersSender {
    static void event(WStype_t type, uint8_t *payload, size_t length);

    static PCountersSender _cs;

    WebSocketsClient _wsocket;
    String _addr; 
    uint16_t _port;
    String _path;
    String _room_id;
    String _json;

    /**
     * \brief Метод вызывается Websocket клиентом при получении ответа с сервера. 
     *        Ответ в виде json: 
     *                  { "room_id":"service room id", "status":"ok" }
     *        Выполняет отправку сторожевому контроллеру команду на отключение.
     */ 
    void recvState(const String &srecv);

public:
    bool _is_recv;

    static CountersSender* get(const String& url, const String& room_id, const String& json);

    static void reset();

    explicit CountersSender(const String& addr, uint16_t port, const String &path, const String& room_id, const String& json);

    ~CountersSender();

    void update();
};