#include <ArduinoJson.h>

#include "SdController.hpp"
#include "ConfigureServer.hpp"


typedef StaticJsonDocument<1024> JsonBufferType;

static const time_t CONFIGURE_TIMEOUT = 1800; ///< Количество секунд работы режима конфигурирования.

String ConfigureServer::_dev_id = "0000000000";
double ConfigureServer::_bat_level = 0.0;


String ConfigureServer::getPage(const String& dev_id, const String& bat_level) {
    return  
    "<!DOCTYPE html>" \
    "<html>" \
        "<head>" \
            "<title>Configure page</title>" \
            "<meta charset='utf-8'>" \
            "<meta name='viewport' content='width=device-width, initial-scale=1'>" \
        "</head>" \
        "<style>" \
            "html{width:100%;height:100%;}" \
            "body{font-family:'Roboto',Times,serif;overflow-x: hidden;}" \
            "input{margin-bottom:3px;margin-top:0px;}" \
            "label{margin-left:3px;margin-right:3px;font-size:14pt;}" \
            "*{box-sizing:border-box;}" \
            ".clear{clear:both;}" \
            ".right{text-align:right;}" \
            ".left{text-align:left;}" \
            ".row{margin-right:-5px;margin-left:-5px;}" \
            ".row:before,.row:after{display:table;content:' ';}" \
            ".row:after{clear: both;}" \
            ".col-xs-2{width:16.666666%;}" \
            ".col-xs-4{width:33.333333%;}" \
            ".col-xs-6{width:50%;}" \
            ".col-xs-10{width:83.333333%;}" \
            ".col-xs-12{width:100%;}" \
            ".col-xs-2,.col-xs-4,.col-xs-6,.col-xs-10,.col-xs-12{position:relative;min-height:1px;padding-bottom:5px;padding-top:5px;padding-right:10px;padding-left:10px;float:left;}" \
            ".border{border:1px solid #1b9c1a;border-radius:3px;}" \
            ".btn-primary:hover{background-color:#0d690c;border-color:#0d690c;}" \
            ".btn-primary:checked{background-color:#0d690c;border-color:#0d690c;}" \
            ".btn-primary{color:#fff;background-color:#1b9c1a;border-color:#0026ea;}" \
            ".btn{display:inline-block;padding:6px 12px;margin-bottom:0;font-size:16px;font-weight:400;line-height:1.42857143;text-align:center;white-space:nowrap;vertical-align:middle;touch-action:manipulation;cursor:pointer;user-select:none;background-image:none;border:1px solid transparent;border-radius:4px;}" \
        "</style>" \
        "<body>" \
            "<div class='col-xs-12'>" \
                "<div id='esion_container' class='border col-xs-12'>" \
                    "<div class='row col-xs-12'><div id='esion_label' class='col-xs-10'><h2>ESION</h2></div><div id='esion_version' class='col-xs-2 right'><h3>V 2.0</h3></div></div>" \
                    "<div id='settings_container' class='col-xs-12'>" \
                        "<div class='border col-xs-12'>" \
                            "<div class='col-xs-12'>" \
                                "<div id='edit_service'>" \
                                    "<div class='row col-xs-12'>" \
                                        "<div class='row col-xs-2'><label style='font-weight:bold'>WiFi SSID</label></div>" \
                                        "<div class='row col-xs-10'><input id='server_ssid' class='col-xs-12' type='text' placeholder='Укажите wifi SSID.'></div>" \
                                    "</div>" \
                                    "<div class='row col-xs-12'>" \
                                        "<div class='row col-xs-2'><label style='font-weight:bold'>WiFi PSWD</label></div>" \
                                        "<div class='row col-xs-10'><input id='server_pswd' class='col-xs-12' type='text' placeholder='Укажите wifi PASSWORD.'></div>" \
                                    "</div>" \
                                    "<div class='row col-xs-12'>" \
                                        "<div class='row col-xs-2'><label style='font-weight:bold'>SRV URL</label></div>" \
                                        "<div class='row col-xs-10'><input id='service_url' class='col-xs-12' type='text' placeholder='Укажите URL сервиса обслуживания.'></div>" \
                                    "</div>" \
                                    "<div class='row col-xs-12'>" \
                                        "<div class='row col-xs-2'><label style='font-weight:bold'>Коллекция</label></div>" \
                                        "<div class='row col-xs-10'><input id='collection' class='col-xs-12' type='text' placeholder='Укажите имя коллекции для данного устройства.'></div>" \
                                    "</div>" \
                                    "<div class='row col-xs-12'>" \
                                        "<div class='row col-xs-2'><label style='font-weight:bold'>Пользователь</label></div>" \
                                        "<div class='row col-xs-10'><input id='user_name' class='col-xs-12' type='text' placeholder='Укажите ФИО владельца.'></div>" \
                                    "</div>" \
                                    "<div class='row col-xs-12'>" \
                                        "<div class='row col-xs-2'><label style='font-weight:bold'>Описание</label></div>" \
                                        "<div class='row col-xs-10'><input id='desc' class='col-xs-12' type='text' placeholder='Укажите адрес и/или подробности места установки.'></div>" \
                                    "</div>" \
                                    "<div class='col-xs-12'><hr></div>" \
                                    "<div class='col-xs-12'>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>№ 1. Тип:</label></div>" \
                                            "<div class='col-xs-2'><input id='type_1' class='col-xs-12' type='text' placeholder='Тип счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_1' class='col-xs-12' type='text' placeholder='Единица измерения.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит/Импл:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_impl_1' class='col-xs-12' type='text' placeholder='Импульсов на юнит.'></div>" \
                                        "</div>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Сер.Номер:</label></div>" \
                                            "<div class='col-xs-2'><input id='serial_1' class='col-xs-12' type='text' placeholder='Номер счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Описание:</label></div>" \
                                            "<div class='col-xs-6'><input id='desc_1' class='col-xs-12' type='text' placeholder='Описание счетчика.'></div>" \
                                        "</div>" \
                                    "</div>" \
                                    "<div class='col-xs-12'><hr></div>" \
                                    "<div class='col-xs-12'>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>№ 2. Тип:</label></div>" \
                                            "<div class='col-xs-2'><input id='type_2' class='col-xs-12' type='text' placeholder='Тип счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_2' class='col-xs-12' type='text' placeholder='Единица измерения.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит/Импл:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_impl_2' class='col-xs-12' type='text' placeholder='Импульсов на юнит.'></div>" \
                                        "</div>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Сер.Номер:</label></div>" \
                                            "<div class='col-xs-2'><input id='serial_2' class='col-xs-12' type='text' placeholder='Номер счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Описание:</label></div>" \
                                            "<div class='col-xs-6'><input id='desc_2' class='col-xs-12' type='text' placeholder='Описание счетчика.'></div>" \
                                        "</div>" \
                                    "</div>" \
                                    "<div class='col-xs-12'><hr></div>" \
                                    "<div class='col-xs-12'>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>№ 3. Тип:</label></div>" \
                                            "<div class='col-xs-2'><input id='type_3' class='col-xs-12' type='text' placeholder='Тип счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_3' class='col-xs-12' type='text' placeholder='Единица измерения.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит/Импл:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_impl_3' class='col-xs-12' type='text' placeholder='Импульсов на юнит.'></div>" \
                                        "</div>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Сер.Номер:</label></div>" \
                                            "<div class='col-xs-2'><input id='serial_3' class='col-xs-12' type='text' placeholder='Номер счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Описание:</label></div>" \
                                            "<div class='col-xs-6'><input id='desc_3' class='col-xs-12' type='text' placeholder='Описание счетчика.'></div>" \
                                        "</div>" \
                                    "</div>" \
                                    "<div class='col-xs-12'><hr></div>" \
                                    "<div class='col-xs-12'>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>№ 4. Тип:</label></div>" \
                                            "<div class='col-xs-2'><input id='type_4' class='col-xs-12' type='text' placeholder='Тип счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_4' class='col-xs-12' type='text' placeholder='Единица измерения.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Юнит/Импл:</label></div>" \
                                            "<div class='col-xs-2'><input id='unit_impl_4' class='col-xs-12' type='text' placeholder='Импульсов на юнит.'></div>" \
                                        "</div>" \
                                        "<div class='col-xs-12 row'>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Сер.Номер:</label></div>" \
                                            "<div class='col-xs-2'><input id='serial_4' class='col-xs-12' type='text' placeholder='Номер счетчика.'></div>" \
                                            "<div class='col-xs-2'><label style='font-weight:bold'>Описание:</label></div>" \
                                            "<div class='col-xs-6'><input id='desc_4' class='col-xs-12' type='text' placeholder='Описание счетчика.'></div>" \
                                        "</div>" \
                                    "</div>" \
                                    "<div class='col-xs-12'><hr></div>" \
                                "</div>" \
                            "</div>" \
                            "<div id='under_line' class='row col-xs-12'>" \
                                "<div id='esion_label' class='col-xs-4'><label>Идентификатор: </label><label id='id_label' style='font-weight:bold'>" + dev_id + "</label></div>" \
                                "<div id='battery' class='col-xs-4'><label>Тип питания: </label><label id='battery_type_label' style='font-weight:bold'>" + POWER_TYPE + "</label></div>" \
                                "<div id='battery' class='col-xs-4 right'><label>Напряжение батареи: </label><label id='battery_label' style='font-weight:bold'>" + bat_level + "V</label></div>" \
                            "</div>" \
                            "<div class='row col-xs-12'>" \
                                "<div class='row col-xs-4'></div>" \
                                "<div class='row col-xs-4'><button id='bt_save' class='btn btn-primary col-xs-12' onclick='saveSettings()' style='font-size:130%'>Запомнить настройки</button></div>" \
                                "<div class='row col-xs-4'></div>" \
                            "</div>" \
                        "</div>" \
                    "</div>" \
                    "<div id='author' class='col-xs-12 right'><label style='font-size:10px'>ООО \"ИнфоТеr-Сервис\":</label><label style='font-size:10px'>(812)34-77-99-8</label></dev>" \
                "</div>" \
            "</div>" \
        "</body>" \
        "<script>" \
            "function saveSettings() {" \
                "var xhr = new XMLHttpRequest();" \
                "xhr.open('POST', '/save', true);" \
                "xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');" \
                "xhr.send('json=' + JSON.stringify({" \
                    "wifi:{" \
                        "ssid:document.getElementById('server_ssid').value," \
                        "pswd:document.getElementById('server_pswd').value}," \
                    "url:document.getElementById('service_url').value," \
                    "coll:document.getElementById('collection').value," \
                    "user:document.getElementById('user_name').value," \
                    "desc:document.getElementById('desc').value," \
                    "counters:[{" \
                            "type:document.getElementById('type_1').value," \
                            "unit:document.getElementById('unit_1').value," \
                            "unit_impl:document.getElementById('unit_impl_1').value," \
                            "serial:document.getElementById('serial_1').value," \
                            "desc:document.getElementById('desc_1').value},{" \
                            "type:document.getElementById('type_2').value," \
                            "unit:document.getElementById('unit_2').value," \
                            "unit_impl:document.getElementById('unit_impl_2').value," \
                            "serial:document.getElementById('serial_2').value," \
                            "desc:document.getElementById('desc_2').value},{" \
                            "type:document.getElementById('type_3').value," \
                            "unit:document.getElementById('unit_3').value," \
                            "unit_impl:document.getElementById('unit_impl_3').value," \
                            "serial:document.getElementById('serial_3').value," \
                            "desc:document.getElementById('desc_3').value},{" \
                            "type:document.getElementById('type_4').value," \
                            "unit:document.getElementById('unit_4').value," \
                            "unit_impl:document.getElementById('unit_impl_4').value," \
                            "serial:document.getElementById('serial_4').value," \
                            "desc:document.getElementById('desc_4').value}]}));}" \
        "</script>" \
    "</html>";
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PConfigureServer& ConfigureServer::getPtr(const String& srv_ssid, const String& srv_pswd) {
    static PConfigureServer cfg_srv;
    if (not cfg_srv) {
        cfg_srv.reset(new ConfigureServer(srv_ssid, srv_pswd));
    }
    return cfg_srv;
}


void ConfigureServer::handleRoot() {
    Blink<BLUE_PIN>::get()->on();
    #ifdef DEBUG
    Serial.println("REST root.");
    #endif
    auto conf = getPtr();
    if (conf) {
        Blink<BLUE_PIN>::get()->off();
        conf->_server.sendContent(getPage(_dev_id, String(_bat_level, 4)));
        #ifdef DEBUG
        Serial.println("Send content");
        #endif
    }
}


void ConfigureServer::handleSave() {
    #ifdef DEBUG
    Serial.println("REST save.");
    #endif
    auto conf_srv = getPtr();
    if (conf_srv) {
        Blink<BLUE_PIN>::get()->on();
        if (conf_srv->_server.args()) {
            String aname = conf_srv->_server.argName(0).c_str();
            if (aname == "json") {
                auto args = conf_srv->_server.arg(0);
                #ifdef DEBUG
                Serial.println("Args: \"" + args + "\"");
                #endif
                conf_srv->_is_complete = ConfigureServer::parseSettings(args, conf_srv->_dev_conf);
                auto nvs = Nvs::get();
                if (nvs) {
                    nvs->setSsid(conf_srv->_dev_conf.wc.ssid);
                    nvs->setPswd(conf_srv->_dev_conf.wc.pswd);
                    nvs->setUrl(conf_srv->_dev_conf.service_url);
                    nvs->setCollectionName(conf_srv->_dev_conf.coll_name);
                    nvs->setUser(conf_srv->_dev_conf.user_name);
                    nvs->setDescription(conf_srv->_dev_conf.user_desc);
                    for (uint8_t i = 0; i < conf_srv->_dev_conf.counts.size(); ++i) {
                        nvs->setCounterConfig(i, conf_srv->_dev_conf.counts[i]);
                    }
                } else {
                    #ifdef DEBUG
                    Serial.println("ERROR: Can`t save configuration.");
                    #endif
                    ErrorLights::get()->error();
                }
            }
        }
        delay(5);
        Blink<BLUE_PIN>::get()->off();
    }
}


void ConfigureServer::handleNotFound(){
    #ifdef DEBUG
    Serial.println("REST is not found.");
    #endif
    auto conf = getPtr();
    if (conf) {
        String msg = "Page Not Found\n\n";
        msg += "URI: ";
        msg += conf->_server.uri();
        msg += "\nMethod: ";
        msg += (conf->_server.method() == HTTP_GET)?"GET":"POST";
        msg += "\nArguments: ";
        msg += conf->_server.args();
        msg += "\n";
        for (uint8_t i = 0; i < conf->_server.args(); i++){
            msg += " " + conf->_server.argName(i) + ": " + conf->_server.arg(i) + "\n";
        }
        conf->_server.send(404, "text/plain", msg);
    }
    ErrorLights::get()->error();
}


bool ConfigureServer::parseSettings(const String &jstr, DeviceConfig& dev_conf) {
    bool ret = false;
    if (jstr.length()) {
        JsonBufferType jbuf;
        auto err = deserializeJson(jbuf, jstr.c_str());
        if (not err) {
            JsonObject wifi = jbuf["wifi"];
            if (not wifi.isNull()) {
                String ssid =  wifi["ssid"].as<char*>();
                if (ssid.length()) {
                    dev_conf.wc.ssid = ssid;
                    #ifdef DEBUG    
                    Serial.println("Read wifi ssid: \"" + dev_conf.wc.ssid + "\"");
                    #endif
                }
                String pswd =  wifi["pswd"].as<char*>();
                if (pswd.length()) {
                    dev_conf.wc.pswd = pswd;
                    #ifdef DEBUG
                    Serial.println("Read wifi pswd: \"" + dev_conf.wc.pswd + "\"");
                    #endif
                }
            }
            String url = jbuf["url"].as<char*>();
            if (url.length()) {
                dev_conf.service_url = url;
                #ifdef DEBUG
                Serial.println("Read URL: \"" + dev_conf.service_url + "\"");
                #endif
            }
            String coll = jbuf["coll"].as<char*>();
            if (coll.length()) {
                dev_conf.coll_name = coll;
                #ifdef DEBUG
                Serial.println("Collection: \"" + dev_conf.coll_name + "\"");
                #endif
            }
            String user = jbuf["user"].as<char*>();
            if (user.length()) {
                dev_conf.user_name = user;
                #ifdef DEBUG
                Serial.println("User name: \"" + dev_conf.user_name + "\"");
                #endif
            }
            String desc = jbuf["desc"].as<char*>();
            if (desc.length()) {
                dev_conf.user_desc = desc;
                #ifdef DEBUG
                Serial.println("Read DESC: \"" + dev_conf.user_desc + "\"");
                #endif
            }
            JsonArray counters = jbuf["counters"].as<JsonArray>();
            if (not counters.isNull()) {
                for (uint8_t i = 0; i < counters.size(); ++i) {
                    String si = String(i, DEC);
                    String type = counters[i]["type"].as<char*>();
                    if (type.length() and type not_eq "none") {
                        dev_conf.counts[i].type = type;
                        #ifdef DEBUG
                        Serial.println("Read " + si + " type: \"" + type + "\"");
                        #endif
                        String unit = counters[i]["unit"].as<char*>();
                        if (unit.length()) {
                            dev_conf.counts[i].unit = unit;
                            #ifdef DEBUG
                            Serial.println("Read " + si + " unit: \"" + unit + "\"");
                            #endif
                        }
                        String unit_impl = counters[i]["unit_impl"].as<char*>();
                        if (unit_impl.length()) {
                            dev_conf.counts[i].unit_impl = unit_impl;
                            #ifdef DEBUG
                            Serial.println("Read " + si + " unit_impl: \"" + unit_impl + "\"");
                            #endif
                        }
                        String serial = counters[i]["serial"].as<char*>();
                        if (serial.length()) {
                            dev_conf.counts[i].serial = serial;
                            #ifdef DEBUG
                            Serial.println("Read " + si + " serial: \"" + serial + "\"");
                            #endif
                        }
                        String desc = counters[i]["desc"].as<char*>();
                        if (desc.length()) {
                            dev_conf.counts[i].desc = desc;
                            #ifdef DEBUG
                            Serial.println("Read " + si + " desc: \"" + desc + "\"");
                            #endif
                        }
                    }
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ConfigureServer::ConfigureServer(const String& srv_ssid, const String& srv_pswd)
    : _srv_ssid(srv_ssid)
    , _srv_pswd(srv_pswd)
    , _server(ESP32WebServer(80)) 
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
    _server.on("/esion", handleRoot);
    _server.on("/save", HTTP_POST, handleSave);
    _server.onNotFound(handleNotFound);
    _server.begin();
    _start_time = time(nullptr);
    #ifdef DEBUG
    Serial.println(String("Config server[ " + srv_ssid + "@" + srv_pswd + " ] IP: \"") + WiFi.softAPIP().toString() + String("\""));
    #endif
}


ConfigureServer::~ConfigureServer() {
    WiFi.disconnect(); // обрываем WIFI соединения
    WiFi.softAPdisconnect(); // отключаем отчку доступа(если она была
    WiFi.mode(WIFI_OFF); // отключаем WIFI
    delay(500);
    Blink<BLUE_PIN>::get()->off();
}


void ConfigureServer::execute(const String& dev_id, double bat_level) {
    _dev_id = dev_id;
    _bat_level = bat_level;
    bool is_timeout = false;
    while (not _is_complete and not is_timeout) {
        _server.handleClient();
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
