#pragma once

#include <string>
#include <memory>

#include <Arduino.h>
#include <ArduinoNvs.h>

#define DEBUG

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


struct Blink {
    Blink() {
        /// Зажечь светодиод.  
        digitalWrite(LED_PIN, LOW);
    }
    ~Blink() {
        /// Погасить светодиод.  
        digitalWrite(LED_PIN, HIGH);
    }
};


/**
 * \brief Структура реализует разбор URL на составляющие.
 */ 
struct Url {
    Url(const String& url) {
        String s(url);
        int pr = s.indexOf("://");
        if (pr not_eq -1) {
            protocol = s.substring(0, pr);
            s = s.substring(pr + 3);
        }
        int pp = s.indexOf(":");
        int pt = s.indexOf("/");
        int pq = s.indexOf("?");
        if (pp not_eq -1) {
            String sp = s.substring(0, pp);
            port = static_cast<uint16_t>(sp.toInt());
            if (pt not_eq -1) {
                host = s.substring(pp + 1, pt);
                if (pq not_eq -1) {
                    path = s.substring(pt, pq);
                    query = s.substring(pq);
                } else {
                    path = s.substring(pt);
                }
            } else {
                host = s.substring(pp + 1);
            }
        }
        if (pt not_eq -1) {
            host = s.substring(0, pt);
            if (pq not_eq -1) {
                path = s.substring(pt, pq);
                query = s.substring(pq);
            } else {
                path = s.substring(pt);
            }
        } else {
            host = s;
        }
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
