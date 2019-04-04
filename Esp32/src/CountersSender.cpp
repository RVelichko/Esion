
#include <ArduinoJson.h>

#include "CountersSender.hpp"
#include "utils.hpp"


static const uint16_t DEFAULT_RECONNECT_TIMEOUT = 1000; /// mlsecs


typedef StaticJsonDocument<500> JsonBufferType;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CountersSender::recvState(const String &srecv) {
    _is_recv = false;
    JsonBufferType jbuf;
    _is_err = deserializeJson(jbuf, srecv.c_str());
    #ifdef DEBUG
    Serial.println("MSG: \"" + srecv + "\"");              
    #endif
    if (not _is_err) {
        String room_id = jbuf["room_id"].as<char*>();
        String status = jbuf["status"].as<char*>();
        if (room_id == _room_id and status == "ok") {
            #ifdef DEBUG
            Serial.println("Recv OK");              
            #endif
            _is_recv = true;
        } else {
            _is_err = true;
        }
    } 
    if (_is_err) {
        #ifdef DEBUG
        Serial.println("Recv ERROR");              
        #endif
    }
}


CountersSender::CountersSender(const String& addr, uint16_t port, const String &path, const String& room_id, const String& json) 
    : _addr(addr) 
    , _port(port)
    , _path(path)
    , _room_id(room_id)
    , _json(json) 
    , _is_recv(false) 
    , _is_err(false)
{}


CountersSender::~CountersSender() {
    /// Отключиться от сервера.
    _wsocket.disconnect();
    #ifdef DEBUG
    Serial.println("Websock client is disconnected");
    #endif
}


void CountersSender::execute() {
    //_addr = "94.127.68.132";
    //_path = "/device";
    //_port = 20000;

    if (_wsocket.connect(_addr, _path, _port)) {
        #ifdef DEBUG
        Serial.println("Connected to \"" + _addr + ":" +  String(_port, DEC) + _path + "\"");
        #endif
        _wsocket.send(_json);
        #ifdef DEBUG
        Serial.println("Send \"" + _json + "\"");
        #endif
        while (not _is_err and not _is_recv) {
            String msg;
            if (_wsocket.getMessage(msg)) {
                recvState(msg);
            }
        }
    } else  {
        #ifdef DEBUG
        Serial.println("Can`t connect to \"" + _addr + ":" +  String(_port, DEC) + _path + "\"");
        #endif
        _is_err = true;
    }
}
