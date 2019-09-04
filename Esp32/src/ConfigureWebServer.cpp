
#include <ArduinoJson.h>

#include "ConfigureWebServer.hpp"

static const time_t CONFIGURE_TIMEOUT = 1800; ///< Количество секунд работы режима конфигурирования.


extern const uint8_t index_html_start[] asm("_binary_src_data_index_html_gz_start");
extern const uint8_t index_html_end[]   asm("_binary_src_data_index_html_gz_end");
extern const uint8_t favicon_ico_start[] asm("_binary_src_data_favicon_ico_start");
extern const uint8_t favicon_ico_end[]   asm("_binary_src_data_favicon_ico_end");
extern const uint8_t FontRoboto_css_start[] asm("_binary_src_data_FontRoboto_css_gz_start");
extern const uint8_t FontRoboto_css_end[]   asm("_binary_src_data_FontRoboto_css_gz_end");
extern const uint8_t app_css_start[] asm("_binary_src_data_css_app_4372d366_css_gz_start");
extern const uint8_t app_css_end[]   asm("_binary_src_data_css_app_4372d366_css_gz_end");
extern const uint8_t chunk_vendors_css_start[] asm("_binary_src_data_css_chunk_vendors_e30ee463_css_gz_start");
extern const uint8_t chunk_vendors_css_end[]   asm("_binary_src_data_css_chunk_vendors_e30ee463_css_gz_end");
extern const uint8_t flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_start[] asm("_binary_src_data_fonts_flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_start");
extern const uint8_t flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_end[]   asm("_binary_src_data_fonts_flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_end");
extern const uint8_t app_js_start[] asm("_binary_src_data_js_app_0e35ed97_js_gz_start");
extern const uint8_t app_js_end[]   asm("_binary_src_data_js_app_0e35ed97_js_gz_end");
extern const uint8_t chunk_vendors_js_start[] asm("_binary_src_data_js_chunk_vendors_6077a4fd_js_gz_start");
extern const uint8_t chunk_vendors_js_end[]   asm("_binary_src_data_js_chunk_vendors_6077a4fd_js_gz_end");


static const char favicon_str[] = "/favicon.ico";
static const char FontRoboto_str[] = "/FontRoboto.css";
static const char flUhRq6tzZclQEJ_Vdg_IuiaDsNc_str[] = "/fonts/flUhRq6tzZclQEJ-Vdg-IuiaDsNc.d7e60f9d.woff2";
static const char app_css_str[] = "/css/app.4372d366.css";
static const char chunk_vendors_css_str[] = "/css/chunk-vendors.e30ee463.css";
static const char app_js_str[] = "/js/app.0e35ed97.js";
static const char chunk_vendors_js_str[] = "/js/chunk-vendors.6077a4fd.js";


String ConfigureWebServer::_dev_id = "none";
double ConfigureWebServer::_bat_level = 0;


typedef StaticJsonDocument<1024> JsonBufferType;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PConfigureWebServer& ConfigureWebServer::getPtr(const String& srv_ssid, const String& srv_pswd) {
    static PConfigureWebServer cfg_srv;
    if (not cfg_srv) {
        cfg_srv.reset(new ConfigureWebServer(srv_ssid, srv_pswd));
    }
    return cfg_srv;
}


bool ConfigureWebServer::resetServer() {
    auto ptr = ConfigureWebServer::getPtr();
    if (ptr) {
        ptr.reset();
        return true;
    }
    return false;
}


void ConfigureWebServer::handleNotFound(AsyncWebServerRequest *request) {
    request->send(404);
}


void ConfigureWebServer::handleApply(AsyncWebServerRequest *request) {
    bool complete = false;
    if (request->params()) {
        auto val = request->getParam(0)->value();
        #ifdef DEBUG    
        Serial.println("Handle APPLY: " + val);
        #endif
        complete = ConfigureWebServer::parseSettings(val);
    }
    if (complete) {
        request->send(200, "application/json", "{\"status\",\"ok\"}");
    } else {
        request->send(500, "application/json", "{\"status\",\"err\"}");
    }
    //ConfigureWebServer::getPtr()->_is_complete = complete;
}


void ConfigureWebServer::handleExit(AsyncWebServerRequest *request) {
    #ifdef DEBUG    
    Serial.println("Handle EXIT");
    #endif
    request->send(200, "application/json", "{\"status\",\"ok\"}");
    ConfigureWebServer::getPtr()->_is_complete = true;
}


void ConfigureWebServer::handleAppInfo(AsyncWebServerRequest *request) {
    size_t bl = static_cast<size_t>(_bat_level * 1000.0);
    String js = "{\"dev_id\":\"" + _dev_id + "\",\"power_percent\":" + String(static_cast<double>(bl) / 1000.0, DEC) + "}";
    #ifdef DEBUG    
    Serial.println("Handle app_info: \"" + js + "\"");
    #endif
    request->send(200, "application/json", js.c_str());
}


void ConfigureWebServer::handleSettingsInfo(AsyncWebServerRequest *request) {
    auto pwrid = Nvs::get()->getPwrId();
    String pwrt;
    if (pwrid == 1) {
        pwrt = "bat_lion";
    } else if (pwrid == 2) {
        pwrt = "bat_4aa";
    }
    auto service_url = Nvs::get()->getUrl();
    auto ssid = Nvs::get()->getSsid();
    auto pswd = Nvs::get()->getPswd();
    auto user = Nvs::get()->getUser();
    auto coll = Nvs::get()->getCollectionName();
    auto desc = Nvs::get()->getDescription();
    String js = "{" \
        "\"cloud_url\":\"" + service_url + "\"," \
        "\"ssid\":\"" + ssid + "\"," \
        "\"pswd\":\"" + pswd  + "\"," \
        "\"user\":\"" + user + "\"," \
        "\"address\":\"" + coll + "\","\
        "\"desc\":\"" + desc + "\"," \
        "\"power\":\"" + pwrt + "\"}";
    #ifdef DEBUG    
    Serial.println("Handle settings_info: \"" + js + "\"");
    #endif
    request->send(200, "application/json", js.c_str());
}


void SendCounter(int i, AsyncWebServerRequest *request) {
    CounterConfig count_cfg = Nvs::get()->getCounterConfig(i);
    String js = "{\"type\",\"none\"}";
    if (count_cfg.type not_eq "none") {
        uint32_t counter = Nvs::get()->getCounter(i);
        double mcubs = static_cast<double>(counter * count_cfg.unit_impl) / 1000.0;
        js = "{" \
            "\"type\":\"" + count_cfg.type + "\"," \
            "\"ser_num\":\"" + count_cfg.serial + "\"," \
            "\"unit\":\"" + count_cfg.unit + "\"," \
            "\"unit_impl\":" + count_cfg.unit_impl + "," \
            "\"desc\":\"" + count_cfg.desc + "\"," \
            "\"mcubs\":" + String(mcubs, DEC) + "}";
        #ifdef DEBUG    
        Serial.println("Handle counter[" + String(i, DEC) + "] | " + counter + ": \"" + js + "\"");
        #endif
    } else {
        #ifdef DEBUG    
        Serial.println("Handle counter[" + String(i, DEC) + "]: \"" + js + "\"");
        #endif
    }
    request->send(200, "application/json", js.c_str());
}


void ConfigureWebServer::handleCounter1(AsyncWebServerRequest *request) {
    SendCounter(0, request);
}


void ConfigureWebServer::handleCounter2(AsyncWebServerRequest *request) {
    SendCounter(1, request);
}


void ConfigureWebServer::handleCounter3(AsyncWebServerRequest *request) {
    SendCounter(2, request);
}


void ConfigureWebServer::handleCounter4(AsyncWebServerRequest *request) {
    SendCounter(3, request);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool ConfigureWebServer::parseSettings(const String &jstr) {
    bool ret = false;
    if (jstr.length()) {
        JsonBufferType jbuf;
        auto err = deserializeJson(jbuf, jstr.c_str());
        if (not err) {
            JsonObject jsets = jbuf["settings"];
            if (not jsets.isNull()) {
                String ssid = jsets["ssid"].as<char*>();
                if (ssid.length()) {
                    ssid = EscapeQuotes(ssid);
                    Nvs::get()->setSsid(ssid);
                    #ifdef DEBUG    
                    Serial.println("Read wifi ssid: \"" + ssid + "\"");
                    #endif
                }
                String pswd = jsets["pswd"].as<char*>();
                if (pswd.length()) {
                    pswd = EscapeQuotes(pswd);
                    Nvs::get()->setPswd(pswd);
                    #ifdef DEBUG
                    Serial.println("Read wifi pswd: \"" + pswd + "\"");
                    #endif
                }
                String url = jsets["url"].as<char*>();
                if (url.length()) {
                    url = EscapeQuotes(url);
                    Nvs::get()->setUrl(url);
                    #ifdef DEBUG
                    Serial.println("Read URL: \"" + url + "\"");
                    #endif
                }
                String address = jsets["address"].as<char*>();
                if (address.length()) {
                    address = EscapeQuotes(address);
                    Nvs::get()->setCollectionName(address);
                    #ifdef DEBUG
                    Serial.println("Address: \"" + address + "\"");
                    #endif
                }
                String user = jsets["user"].as<char*>();
                if (user.length()) {
                    user = EscapeQuotes(user);
                    Nvs::get()->setUser(user);
                    #ifdef DEBUG
                    Serial.println("User name: \"" + user + "\"");
                    #endif
                }
                String desc = jsets["desc"].as<char*>();
                if (desc.length()) {
                    desc = EscapeQuotes(desc);
                    Nvs::get()->setDescription(desc);
                    #ifdef DEBUG
                    Serial.println("Read DESC: \"" + desc + "\"");
                    #endif
                }
                String power = jsets["power"].as<char*>();
                if (power.length()) {
                    if (power == "bat_lion") {
                        Nvs::get()->setPwrId(1);
                    } else if (power == "bat_4aa") {
                        Nvs::get()->setPwrId(2);
                    } else {
                        Nvs::get()->setPwrId(0);
                    }
                    #ifdef DEBUG
                    Serial.println("Read PWR TYPE: \"" + power + "\"");
                    #endif
                }
            }
            //"counters":[{"count_type":"test","ser_num":"001","desc":"Тестовый счётчик N 001"}]
            JsonArray jcounters = jbuf["counters"].as<JsonArray>();
            if (not jcounters.isNull()) {
                for (uint8_t i = 0; i < jcounters.size(); ++i) {
                    String si = String(i, DEC);
                    String type = jcounters[i]["count_type"].as<char*>();
                    CounterConfig cconf;
                    if (type.length() and type not_eq "none") {
                        cconf.type = EscapeQuotes(type);
                        #ifdef DEBUG
                        Serial.println("Read " + si + " type: \"" + type + "\"");
                        #endif
                        String unit = jcounters[i]["unit"].as<char*>();
                        if (unit.length()) {
                            cconf.unit = EscapeQuotes(unit);
                            #ifdef DEBUG
                            Serial.println("Read " + si + " unit: \"" + unit + "\"");
                            #endif
                        } else {
                            cconf.unit = "Liter";
                        }
                        int unit_impl = jcounters[i]["unit_impl"].as<int>();
                        if (unit_impl not_eq 0) {
                            cconf.unit_impl = unit_impl;
                            #ifdef DEBUG
                            Serial.println("Read " + si + " unit_impl: \"" + unit_impl + "\"");
                            #endif
                        } else {
                            cconf.unit_impl = 1;
                        }
                        String serial = jcounters[i]["ser_num"].as<char*>();
                        if (serial.length()) {
                            cconf.serial = EscapeQuotes(serial);
                            #ifdef DEBUG
                            Serial.println("Read " + si + " serial: \"" + serial + "\"");
                            #endif
                        }
                        String desc = jcounters[i]["desc"].as<char*>();
                        if (desc.length()) {
                            cconf.desc = EscapeQuotes(desc);
                            #ifdef DEBUG
                            Serial.println("Read " + si + " desc: \"" + desc + "\"");
                            #endif
                        }
                    } else {
                        cconf.type = "none";
                        #ifdef DEBUG
                        Serial.println("Read " + si + " type: \"none\"");
                        #endif
                    }
                    Nvs::get()->setCounterConfig(i, cconf);
                }
            }
            ret = true;
        } else {
            #ifdef DEBUG
            Serial.println("ERR: Can`t parse settings. \"" + String(err.c_str()) + "\"");
            #endif
            ErrorLights::get()->error();
        }
    }
    return ret;
}


ConfigureWebServer::ConfigureWebServer(const String& srv_ssid, const String& srv_pswd) 
    : _srv_ssid(srv_ssid)
    , _srv_pswd(srv_pswd)
    , _server(new AsyncWebServer(80)) 
    , _is_complete(false) {
    Blink<BLUE_PIN>::get()->on();
    WiFi.mode(WIFI_AP);
    IPAddress local_IP(192,168,4,22);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    WiFi.softAP(_srv_ssid.c_str(), _srv_pswd.c_str(), 1, 0, 1);
    delay(10);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    delay(10);
    
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=UTF-8", index_html_start, index_html_end - index_html_start - 1);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    _server->on(favicon_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "image/x-icon", favicon_ico_start, favicon_ico_end - favicon_ico_start - 1); 
    });
    _server->on(FontRoboto_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", FontRoboto_css_start, FontRoboto_css_end - FontRoboto_css_start - 1);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    _server->on(flUhRq6tzZclQEJ_Vdg_IuiaDsNc_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "application/font-woff2", 
            flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_start, 
            flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_end - flUhRq6tzZclQEJ_Vdg_IuiaDsNc_d7e60f9d_woff2_start - 1); 
    });
    _server->on(app_css_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", app_css_start, app_css_end - app_css_start - 1);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    _server->on(chunk_vendors_css_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", chunk_vendors_css_start, chunk_vendors_css_end - chunk_vendors_css_start - 1);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    _server->on(app_js_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", app_js_start, app_js_end - app_js_start - 1);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });
    _server->on(chunk_vendors_js_str, HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", chunk_vendors_js_start, chunk_vendors_js_end - chunk_vendors_js_start - 1);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    _server->on("/settings_info", HTTP_GET, handleSettingsInfo);
    _server->on("/app_info", HTTP_GET, handleAppInfo);
    _server->on("/counter_0", HTTP_GET, handleCounter1);	
    _server->on("/counter_1", HTTP_GET, handleCounter2); 	
    _server->on("/counter_2", HTTP_GET, handleCounter3); 	
    _server->on("/counter_3", HTTP_GET, handleCounter4); 	
    _server->on("/apply", HTTP_POST, handleApply);
    _server->on("/exit", HTTP_GET, handleExit);
    _server->onNotFound(handleNotFound);
    _server->begin();

    _start_time = time(nullptr);
    #ifdef DEBUG
    Serial.println(String("Config server[ " + srv_ssid + "@" + srv_pswd + " ] IP: \"") + WiFi.softAPIP().toString() + String("\""));
    #endif
}


ConfigureWebServer::~ConfigureWebServer() {
    WiFi.disconnect(); // обрываем WIFI соединения
    WiFi.softAPdisconnect(); // отключаем отчку доступа(если она была
    WiFi.mode(WIFI_OFF); // отключаем WIFI
    delay(500);
    Blink<BLUE_PIN>::get()->off();
    #ifdef DEBUG
    Serial.println("Config server is stop");
    #endif
}


void ConfigureWebServer::execute(const String& dev_id, double bat_level) {
    _dev_id = dev_id;
    _bat_level = bat_level;
    bool is_timeout = false;
    int pwrid = Nvs::get()->getPwrId();
    #ifdef DEBUG
    Serial.println(String("Configure server [" + _dev_id + "]; bat = " + String(_bat_level, DEC) + "; pwid: " + String(pwrid, DEC) + " is started."));
    #endif
    while (not _is_complete and not is_timeout) {
        delay(1);
        //_server->handleClient();
        auto cur_time = time(nullptr); 
        is_timeout = (CONFIGURE_TIMEOUT <= (cur_time - _start_time));
    }
    #ifdef DEBUG
    if (is_timeout and not _is_complete) {
        Serial.println(String("Configure is timeout."));
    } else {
        Serial.println(String("Configure is complete."));
    }
    #endif
}
