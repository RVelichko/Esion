
#include <ArduinoJson.h>

#include "CountersSender.hpp"
#include "utils.hpp"


static const uint16_t DEFAULT_RECONNECT_TIMEOUT = 1000; /// mlsecs


typedef StaticJsonDocument<500> JsonBufferType;


CountersSender* __cs = nullptr;


void Event(WStype_t type, uint8_t *payload, size_t length) {
    switch(type) {
        case WStype_ERROR: {
            #ifdef DEBUG
            Serial.println("WS type: ERROR.");
            #endif
            Sos::get()->enable();
        } break;
        case WStype_CONNECTED: {
            #ifdef DEBUG
            Serial.println("Connected complete.");
            #endif
            if (__cs) {
                __cs->sendData();
            }
        } break;
        case WStype_TEXT: {
            std::vector<char> arr(length + 1, '\0');
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<char>(payload[i]);
            }
            String rs = String(&arr[0]);
            #ifdef DEBUG
            Serial.println(String("Recv: " + rs).c_str());
            #endif
            if (__cs) {
                __cs->recvState(String(rs));
            }
        } break;

        case WStype_FRAGMENT_TEXT_START:
            #ifdef DEBUG
            Serial.println("WS event FRAGMENT_TEXT_START.");
            #endif
            break;
        case WStype_FRAGMENT_BIN_START:
            #ifdef DEBUG
            Serial.println("WS event FRAGMENT_BIN_START.");
            #endif
            break;
        case WStype_FRAGMENT:
            #ifdef DEBUG
            Serial.println("WS event FRAGMENT.");
            #endif
            break;
        case WStype_FRAGMENT_FIN:
            #ifdef DEBUG
            Serial.println("WS event FRAGMENT_FIN.");
            #endif
            break;
        case WStype_DISCONNECTED: 
            #ifdef DEBUG
            Serial.println("WS event DISCONNECTED.");
            #endif
            if (__cs) {
                __cs->setError();
            }
            break;
        case WStype_BIN:
            #ifdef DEBUG
            Serial.println("WS event BIN.");
            #endif
            break;
        default: 
            #ifdef DEBUG
            Serial.println("Undefined WS event.");
            #endif
            if (__cs) {
                __cs->setError();
            }
            break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


String CountersSender::getData() {
    return _json;
}


void CountersSender::sendData() {
    #ifdef DEBUG
    Serial.println("send: \"" + _json + "\"");
    #endif
    _wsocket.sendTXT(_json.c_str());
}


void CountersSender::setError() {
    _is_err = true;
}


void CountersSender::recvState(const String &srecv) {
    _is_recv = true;
    JsonBufferType jbuf;
    auto err = deserializeJson(jbuf, srecv.c_str());
    if (not err) {
        String room_id = jbuf["room_id"].as<char*>();
        String status = jbuf["status"].as<char*>();
        if (_room_id == room_id and status == "ok") {
            #ifdef DEBUG
            Serial.println("Recv OK");              
            #endif
        } 
    }
}


CountersSender::CountersSender(const String& addr, uint16_t port, const String &path, const String& room_id, const String& json) 
    : _addr(addr) 
    , _port(port)
    , _path(path)
    , _room_id(room_id)
    , _json(json) 
    , _is_recv(false) 
    , _is_err(false) {
    __cs = this;
}


CountersSender::~CountersSender() {
    /// Отключиться от сервера.
    _wsocket.disconnect();
    __cs = nullptr;
    #ifdef DEBUG
    Serial.println("Websock client is disconnected");
    #endif
}


void CountersSender::execute() {
    _wsocket.begin(_addr, _port, _path, "ws");
    _wsocket.onEvent(Event);
    _wsocket.setReconnectInterval(1000);
}


bool CountersSender::update() {
    _wsocket.loop();
    return not _is_err and not _is_recv;
}
