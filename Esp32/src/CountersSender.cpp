
#include <ArduinoJson.h>

#include "CountersSender.hpp"
#include "utils.hpp"


static const uint16_t DEFAULT_WIFI_RECONNECT_TIMEOUT = 1000; /// mlsecs


typedef StaticJsonBuffer<300> JsonBufferType;


PCountersSender CountersSender::_cs;


void CountersSender::event(WStype_t type, uint8_t *payload, size_t length) {
    switch(type) {
        case WStype_CONNECTED: {
            #ifdef DEBUG
            Serial.println("Connected complete.");
            #endif
            _cs->_wsocket.sendTXT(_cs->_json.c_str());
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
            _cs->recvState(String(rs));
        } break;
        case WStype_DISCONNECTED: 
        case WStype_BIN:
        default: 
            break;
    }
}


void CountersSender::recvState(const String &srecv) {
    _is_recv = true;
    JsonBufferType json_buf;
    JsonObject& root = json_buf.parseObject(srecv.c_str());
    if (root.success()) {
        String room_id = root["room_id"].as<char*>();
        String status = root["status"].as<char*>();
        if (_room_id == room_id and status == "ok") {
            #ifdef DEBUG
            Serial.println("Recv OK");              
            #endif
        } 
    }
}


CountersSender* CountersSender::get(const String& url, const String& room_id, const String& json) {
    if (not _cs) {
        Url U(url);
        _cs.reset(new CountersSender(U.host, U.port, U.path, room_id, json));
    }
    return _cs.get();
}


void CountersSender::reset() {
    if (_cs) {
        _cs.reset();
    }
}


CountersSender::CountersSender(const String& addr, uint16_t port, const String &path, const String& room_id, const String& json) 
    : _addr(addr) 
    , _port(port)
    , _path(path)
    , _room_id(room_id)
    , _json(json) 
    , _is_recv(false) {
    #ifdef DEBUG
    Serial.println("Create websock client: " + _addr + ":" + String(_port, DEC) + _path);
    #endif
    _wsocket.begin(_addr, _port, _path);
    _wsocket.onEvent(event);
    _wsocket.setReconnectInterval(DEFAULT_WIFI_RECONNECT_TIMEOUT);
}


CountersSender::~CountersSender() {
    /// Отключиться от сервера.
    _wsocket.disconnect();
    #ifdef DEBUG
    Serial.println("Websock client is disconnected");
    #endif
}


void CountersSender::update() {
    _wsocket.loop();
}
