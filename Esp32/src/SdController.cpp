#include <ArduinoJson.h>

#include "SdController.hpp"


typedef StaticJsonDocument<500> JsonBufferType;

static const char DEFAULT_CONFIG_FILE[] = "config.js";
static const char DEFAULT_COUNTERS_FILE[] = "counters.js";


PSdController& SdController::getPtr() {
    static PSdController sdc;
    if (not sdc) {
        sdc.reset(new SdController());
    }
    return sdc;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


SdController::SdController() {
    _service_timoeut = 1;
    _max_count_for_send = 0;
    _send_sleep_time = 0;
    Blink::get()->on();
    #ifdef DEBUG
    Serial.print("SD initialization is ");
    #endif
    if (not SD.begin()) {
        Sos::get()->enable();
        #ifdef DEBUG
        Serial.println(String("ERROR: Fat type is: " + SD.cardType()).c_str());
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("TRUE");
        #endif
        _is_inited_sd = true;
    }
}

SdController::~SdController() {
    if (_is_inited_sd) {
        SD.end();
        _is_inited_sd = false;
    }
    Blink::get()->off();
}


void SdController::getConfig(uint32_t &send_sleep_time, uint8_t &max_send_count) {
    if (_is_inited_sd) {
        #ifdef DEBUG
        Serial.println("Read config file...");
        #endif
        File f = SD.open(String("/") + DEFAULT_CONFIG_FILE, FILE_READ);
        if (f) {
            size_t fsize = f.size();
            #ifdef DEBUG
            Serial.println("file size: " + String(fsize, DEC));
            #endif
            parseSettings(f, send_sleep_time, max_send_count);
            f.close();
        } else {
            Sos::get()->enable();
            #ifdef DEBUG
            Serial.println("ERROR: can`t open file `" + String(DEFAULT_CONFIG_FILE) + "`");
            #endif
        }
    }
}


void SdController::getCounts(uint32_t &count1,
                             uint32_t &count2,
                             uint32_t &count3,
                             uint32_t &count4,
                             uint32_t &count5,
                             uint32_t &count6) {
    if (_is_inited_sd) {
        #ifdef DEBUG
        Serial.println("Read counters file...");
        #endif
        File f = SD.open(String("/") + DEFAULT_COUNTERS_FILE, FILE_READ);
        if (f) {
            size_t fsize = f.size();
            #ifdef DEBUG
            Serial.println("file size: " + String(fsize, DEC));
            #endif
            String json_str;
            parseCounters(f, count1, count2, count3, count4, count5, count6);
            f.close();
        } else {
            #ifdef DEBUG
            Serial.println("WARNING: can`t open file `" + String(DEFAULT_COUNTERS_FILE) + "`");
            #endif
        }
    }
}


bool SdController::parseSettings(fs::File &f, uint32_t &send_sleep_time, uint8_t &max_send_count) {
    if (f) {
        JsonBufferType jbuf;
        auto err = deserializeJson(jbuf, f);
        if (not err) {
            #ifdef DEBUG
            Serial.println(String("Config is parsed...").c_str());
            #endif
            JsonObject settings = jbuf["settings"].as<JsonObject>();
            if (not settings.isNull()) {
                JsonObject wifi = settings["wifi"].as<JsonObject>();
                if (not wifi.isNull()) {
                    String ssid =  wifi["ssid"].as<char*>();
                    if (ssid.length() not_eq 0) {
                        _wc.ssid = ssid;
                        #ifdef DEBUG
                        Serial.println(String("Read wifi ssid: " + _wc.ssid).c_str());
                        #endif
                    } else {
                        #ifdef DEBUG
                        Serial.println(String("ERROR: can`t find WIFI SSID").c_str());
                        #endif
                    }
                    String pswd =  wifi["pswd"].as<char*>();
                    if (pswd.length() not_eq 0) {
                        _wc.pswd = pswd;
                        #ifdef DEBUG
                        Serial.println("Read wifi pswd: " + _wc.pswd);
                        #endif
                    } else {
                        #ifdef DEBUG
                        Serial.println("ERROR: can`t find WIFI PSWD");
                        #endif
                    }
                } else {
                    #ifdef DEBUG
                    Serial.println("ERROR: can`t find config wifi");
                    #endif
                }
                JsonObject service = settings["service"].as<JsonObject>();
                if (not service.isNull()) {
                    _service_addr = service["address"].as<char*>();
                    _service_port = service["port"].as<int>();
                    _service_rest = service["rest"].as<char*>();
                    _service_timoeut = service["timeout"].as<int>();
                    #ifdef DEBUG
                    Serial.println(String("Read service: " + getUrl() + "?timeout=" + String(_service_timoeut, DEC)).c_str());
                    #endif
                } else {
                    #ifdef DEBUG
                    Serial.println("ERROR: can`t find config service");
                    #endif
                }
                send_sleep_time = settings["send_timeout"].as<int>();
                if (not send_sleep_time) {
                    send_sleep_time = 10;
                }
                _send_sleep_time = send_sleep_time;
                #ifdef DEBUG
                Serial.println("Read send_timeout: " + String(send_sleep_time, DEC));
                #endif
                max_send_count = settings["max_count"].as<int>();
                if (not max_send_count) {
                    max_send_count = 10;
                }
                _max_count_for_send = max_send_count;
                #ifdef DEBUG
                Serial.println("Read max_count: " + String(max_send_count, DEC));
                #endif
            } else {
                #ifdef DEBUG
                Serial.println("ERROR: can`t find config settings");
                #endif
            }
        } else {
            Sos::get()->enable();
            #ifdef DEBUG
            Serial.println("ERROR: can`t parse config json");
            #endif
        }
        return true;
    }
    return false;
}


String SdController::getUrl() {
    return _service_addr + ":" + String(_service_port, DEC) + _service_rest;
}


void SdController::saveSettings(const String& wifi_ssid, const String& wifi_pswd, const String srv_url, int send_timeout, int max_count) {
    if (_is_inited_sd) {
        String conf;
        conf += "{\"settings\":{\n\t\"wifi\":{\n";
        conf += "\t\t\"ssid\":\"" + wifi_ssid + "\",\n";
        conf += "\t\t\"pswd\":\"" + wifi_pswd + "\"\n\t},\n";
        Url U(srv_url);
        conf += "\t\"service\":{\n";
        conf += "\t\t\"address\":\"" + U.host + "\",\n";
        conf += "\t\t\"port\":" + String(U.port, DEC) + ",\n";
        conf += "\t\t\"rest\":\"" + U.path + "\",\n";
        conf += "\t\t\"timeout\":" + String(_service_timoeut, DEC) +  "\n\t},\n";
        conf += "\t\"send_timeout\":" + String(send_timeout, DEC) +  ",\n";
        conf += "\t\"max_count\":" + String(max_count, DEC) + "\n\t}\n}";
        File f = SD.open(String("/") + DEFAULT_CONFIG_FILE, FILE_WRITE);
        if (f) {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(conf.c_str());
            size_t len = conf.length();
            #ifdef DEBUG
            Serial.println("Write to file \"" + String(DEFAULT_CONFIG_FILE) + "\": " + String(len, DEC)  + ": "+ conf);
            #endif
            f.write(p, len);
            f.close();
        } else {
            Sos::get()->enable();
            #ifdef DEBUG
            Serial.println("ERROR: Can`t open config file for writing.");
            #endif
        }
    }
}


bool SdController::parseCounters(fs::File &f,     
                                 uint32_t &count1,
                                 uint32_t &count2,
                                 uint32_t &count3,
                                 uint32_t &count4,
                                 uint32_t &count5,
                                 uint32_t &count6) {
    if (f) {
        JsonBufferType jbuf;
        auto err = deserializeJson(jbuf, f);
        if (not err) {
            #ifdef DEBUG
            Serial.println(String("Config is parsed...").c_str());
            #endif
            JsonArray counters = jbuf["counters"].as<JsonArray>();
            if (not counters.isNull()) {
                count1 = counters[0].as<uint32_t>();
                count2 = counters[1].as<uint32_t>();
                count3 = counters[2].as<uint32_t>();
                count4 = counters[3].as<uint32_t>();
                count5 = counters[4].as<uint32_t>();
                count6 = counters[5].as<uint32_t>();
                #ifdef DEBUG
                Serial.println("Count 1: " + String(count1, DEC)); 
                Serial.println("Count 2: " + String(count2, DEC)); 
                Serial.println("Count 3: " + String(count3, DEC)); 
                Serial.println("Count 4: " + String(count4, DEC)); 
                Serial.println("Count 5: " + String(count5, DEC)); 
                Serial.println("Count 6: " + String(count6, DEC)); 
                #endif
            } else {
                #ifdef DEBUG
                Serial.println(String("ERROR: can`t find config counters").c_str());
                #endif
            }
        } else {
            #ifdef DEBUG
            Serial.println(String("ERROR: can`t parse counters json").c_str());
            #endif
        }
        return true;
    }
    return false;
}


void SdController::saveCounters(uint32_t count1,
                                uint32_t count2,
                                uint32_t count3,
                                uint32_t count4,
                                uint32_t count5,
                                uint32_t count6) {
    if (_is_inited_sd) {
        String counts;
        counts += "{\"counters\":[";
        counts += String(count1, DEC) + ",";
        counts += String(count2, DEC) + ",";
        counts += String(count3, DEC) + ",";
        counts += String(count4, DEC) + ",";
        counts += String(count5, DEC) + ",";
        counts += String(count6, DEC);
        counts += "]}";
        File f = SD.open(String("/") + DEFAULT_COUNTERS_FILE, FILE_WRITE);
        if (f) {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(counts.c_str());
            size_t len = counts.length();
            #ifdef DEBUG
            Serial.println("Write to file \"" + String(DEFAULT_COUNTERS_FILE) + "\": " + String(len, DEC)  + ": "+ counts);
            #endif
            f.write(p, len);
            f.close();
        } else {
            Sos::get()->enable();
            #ifdef DEBUG
            Serial.println("ERROR: Can`t open counters file for writing.");
            #endif
        }
    }
}
