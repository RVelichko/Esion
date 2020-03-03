
#include <ArduinoJson.h>

#include "CountersSender.hpp"
#include "utils.hpp"


static const uint16_t DEFAULT_RECONNECT_TIMEOUT = 30000; /// mlsecs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CountersSender::parseConfig(const JsonObject& jobj) {
    auto nvs = Nvs::get();
    if (nvs) {
        uint64_t ctrl_timeout = jobj["ctrl_timeout"].as<uint64_t>();
        if (ctrl_timeout) {
            nvs->setCtrlTime(ctrl_timeout);
        }
        uint32_t max_impls = jobj["max_impls"].as<uint32_t>();
        if (max_impls) {
            nvs->setMaxImpulses(max_impls);
        }
        uint32_t snd_timeout = jobj["snd_timeout"].as<uint32_t>();
        if (snd_timeout) {
            nvs->setSndTimeout(snd_timeout);
        }
        uint16_t cfg_time = jobj["cfg_time"].as<uint16_t>();
        if (cfg_time) {
            nvs->setCfgTime(cfg_time);
        }
    }
}


void CountersSender::recvState(const String &srecv) {
    _is_recv = false;
    JsonBufferType jbuf;
    _is_err = deserializeJson(jbuf, srecv.c_str());
    #ifdef DEBUG
    Serial.println("MSG: \"" + srecv + "\"");              
    #endif
    if (not _is_err) {
        String status = jbuf["status"].as<char*>();
        if (status == "ok") {
            #ifdef DEBUG
            Serial.println("Recv OK");              
            #endif
            _is_recv = true;
        } else if (status == "cfg") {
            // "cfg": {
            //    "ctrl_timeout": <uint64_t>,
            //    "max_impls": <uint32_t>,
            //    "snd_timeout": <uint32_t>,
            //    "cfg_time": <uint16_t>
            // }
            JsonObject jcfg = jbuf["cfg"];
            parseConfig(jcfg);
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
    //: _wsocket(new WebSocketClient(false))
    : _wsocket(new WebSocketClient(true))
    ,_addr(addr) 
    , _port(port)
    , _path(path)
    , _room_id(room_id)
    , _json(json) 
    , _is_recv(false) 
    , _is_err(false)
{}


CountersSender::~CountersSender() {
    /// Отключиться от сервера.
    if (_wsocket) {
        _wsocket->disconnect();
    }
    #ifdef DEBUG
    Serial.println("Websock client is disconnected");
    #endif
}


void CountersSender::execute() {
    #ifdef DEBUG
    Serial.println("Try connecting to \"" + _addr + ":" +  String(_port, DEC) + _path + "\"");
    #endif
    if (_wsocket) {
        //if (_wsocket->connect("192.168.1.142", "/device", 20000)) {
        if (_wsocket->connect(_addr, _path, _port)) {
            #ifdef DEBUG
            Serial.println("Connected to \"" + _addr + ":" +  String(_port, DEC) + _path + "\"");
            #endif
            _wsocket->send(_json);
            #ifdef DEBUG
            Serial.println("Wait recv");
            #endif
            while (not _is_err and not _is_recv) {
                #ifdef DEBUG
                Serial.print(".");
                #endif
                String msg;
                if (_wsocket->getMessage(msg)) {
                    #ifdef DEBUG
                    Serial.println(".");
                    #endif
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
}
