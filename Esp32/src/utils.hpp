#pragma once

#include <string>
#include <memory>

#include <Arduino.h>
#include <ArduinoNvs.h>

//#define DEBUG
//#define DEBUG_WEBSOCKETS

static const int LED_PIN = 2; 


struct WifiConfig {
    String ssid;
    String pswd;
    
    WifiConfig() 
        : ssid({""})
        , pswd({""})
    {}
};


union BatteryValue {
    struct Bits {
        uint8_t _0:1;
        uint8_t _1:1;
    } _bits;
    uint8_t _val:2;
};


class Blink {

    bool _is_on;

    void init() {
        static bool is_init = false;
        if (not is_init) {
            pinMode(LED_PIN, OUTPUT);
            delay(10);
            is_init = true;
        }
    }
public:
    static Blink* get() {
        static Blink blk;
        return &blk;
    }

    Blink() 
        : _is_on(false) {
        init();
    }

    ~Blink() {
        off();
    }

    void on() {
        /// Зажечь светодиод.
        if (not _is_on) {
            _is_on = true;
            digitalWrite(LED_PIN, HIGH);
        }  
    }

    void off() {
        /// Погасить светодиод.  
        if (_is_on) {
            delay(10);
            digitalWrite(LED_PIN, LOW);
            _is_on = false;
        }
    }
};


class Sos {
    static constexpr int POINT_DT = 60;     
    static constexpr int LINE_DT = 500;     

    void blinkTime(int dt) {
        Blink::get()->on();
        delay(dt);
        Blink::get()->off();
    }

public:
    static Sos* get() {
        static Sos sos;
        return &sos;
    }

    Sos() 
    {}

    void enable() {
        blinkTime(POINT_DT);
        delay(POINT_DT);
        blinkTime(POINT_DT);
        delay(POINT_DT);
        blinkTime(POINT_DT);
        delay(POINT_DT);

        blinkTime(LINE_DT);
        delay(POINT_DT);
        blinkTime(LINE_DT);
        delay(POINT_DT);
        blinkTime(LINE_DT);
        delay(POINT_DT);

        blinkTime(POINT_DT);
        delay(POINT_DT);
        blinkTime(POINT_DT);
        delay(POINT_DT);
        blinkTime(POINT_DT);
        delay(LINE_DT);
    }

    ~Sos() 
    {}
};


/**
 * \brief Структура реализует разбор URL на составляющие.
 */ 
struct Url {
    Url(const String& url) {
        String s(url);
        int iprotocol = s.indexOf("://");
        if (iprotocol not_eq -1) {
            protocol = s.substring(0, iprotocol);
            s = s.substring(iprotocol + 3);
        }
        int iport = s.indexOf(":");
        int ipath = s.indexOf("/");
        int iquery = s.indexOf("?");
        if (iport not_eq -1) {
            host = s.substring(0, iport);
            String sp = s.substring(iport + 1);
            port = static_cast<uint16_t>(sp.toInt());
            if (ipath not_eq -1) {
                if (iquery not_eq -1) {
                    path = s.substring(ipath, iquery);
                    query = s.substring(iquery);
                } else {
                    path = s.substring(ipath);
                }
            }
        } else if (ipath not_eq -1) {
            host = s.substring(0, ipath);
            if (iquery not_eq -1) {
                path = s.substring(ipath, iquery);
                query = s.substring(iquery);
            } else {
                path = s.substring(ipath);
            }
        } else {
            host = s;
        }
        #ifdef DEBUG
        Serial.println("proto: \"" + protocol + "\"; host: \"" + host + "\"; port: \"" + String(port, DEC) + "\"; path: \"" + path + "\"; query: \"" + query + "\"");
        #endif
    }

    String protocol;
    String host;
    uint16_t port;
    String path;
    String query;
};


/**
 * \brief Класс обработки энергонезависимой памяти контроллера.
 */ 
struct Nvs {
    typedef std::unique_ptr<Nvs> PNvs;

    static Nvs* get() {
        static PNvs _nvs;
        if (not _nvs) {
            _nvs.reset(new Nvs());
        }
        return _nvs.get();
    }

    Nvs() {
        NVS.begin();
    }

    void setId(uint64_t id) {
        NVS.setInt("device_id", id);
    }

    uint64_t getId() {
        uint64_t uui64 = NVS.getInt("device_id");    
        return uui64;
    }

    void setConfig(const String& conf_dump) {
        NVS.setString("config", conf_dump);
    }

    String getConfig() {
        String conf = NVS.getString("config");
        return conf;
    }
};
