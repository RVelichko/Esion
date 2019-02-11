#include <ArduinoJson.h>

#include "ConfigureServer.hpp"


typedef StaticJsonBuffer<300> JsonBufferType;


static const time_t CONFIGURE_TIMEOUT = 1200; ///< Количество секунд работы режима конфигурирования.

static const char PAGE[] = 
    "<!DOCTYPE html>"\
    "<html>"\
        "<head>"\
            "<title>Configure page</title>"\
            "<meta charset='utf-8'>"\
            "<meta name='viewport'content='width=device-width,initial-scale=1'>"\
        "</head>"\
        "<style>"\
            "body{min-width:1000px;font-family:'Roboto',Times,serif;overflow-x:hidden;}"\
            "input{margin-bottom:3px;margin-top:0px;}"\
            "label{margin-left:3px;margin-right:3px;font-size:14pt;}"\
            "*{box-sizing:border-box;}"\
            ".clear{clear:both;}"\
            ".right{text-align:right;}"\
            ".left{text-align:left;}"\
            ".row{margin-right:-5px;margin-left:-5px;}"\
            ".row:before,.row:after{display:table;content:' ';}"\
            ".row:after{clear: both;}"\
            ".col-xs-2{width:16.666666%;}"\
            ".col-xs-4{width:33.333333%;}"\
            ".col-xs-6{width:50%;}"\
            ".col-xs-10{width:83.333333%;}"\
            ".col-xs-12{width:100%;}"\
            ".col-xs-2,.col-xs-4,.col-xs-6,.col-xs-10,.col-xs-12{position:relative;min-height:1px;padding-bottom:5px;padding-top:5px;padding-right:10px;padding-left:10px;float:left;}"\
            ".border{border:1px solid #1b9c1a;border-radius:3px;}"\
            ".btn-primary:hover{background-color:#0d690c;border-color:#0d690c;}"\
            ".btn-primary:checked{background-color:#0d690c;border-color:#0d690c;}"\
            ".btn-primary{color:#fff;background-color:#1b9c1a;border-color:#0026ea;}"\
            ".btn{display:inline-block;padding:6px 12px;margin-bottom:0;font-size:16px;font-weight:400;line-height:1.42857143;text-align:center;white-space:nowrap;vertical-align:middle;touch-action:manipulation;cursor:pointer;user-select:none;background-image:none;border:1px solid transparent;border-radius:4px;}"\
        "</style>"\
        "<body>"\
            "<div class='col-xs-12'>"\
                "<div id='esion_container' class='border col-xs-12'>"\
                    "<div class='row col-xs-12'>"\
                        "<div id='esion_label' class='col-xs-10'>"\
                            "<h2>ESION</h2>"\
                        "</div>"\
                        "<div id='esion_version' class='col-xs-2 right'>"\
                            "<h3>V 1.0</h3>"\
                        "</div>"\
                    "</div>"\
                    "<div id='settings_container' class='col-xs-12'>"\
                        "<div class='border col-xs-12'>"\
                            "<div class='col-xs-12'>"\
                                "<div id='edit_service'>"\
                                    "<div class='row col-xs-12'>"\
                                        "<input id='server_ssid' class='col-xs-12' type='text' placeholder='Укажите wifi SSID.'>"\
                                    "</div>"\
                                    "<div class='row col-xs-12'>"\
                                        "<input id='server_pswd' class='col-xs-12' type='text' placeholder='Укажите wifi PASSWORD.'>"\
                                    "</div>"\
                                    "<div class='row col-xs-12'>"\
                                        "<input id='service_url' class='col-xs-12' type='text' placeholder='Укажите URL сервиса обслуживания.'>"\
                                    "</div>"\
                                "</div>"\
                            "</div>"\
                            "<div id='under_line' class='row col-xs-12'>"\
                                "<div id='esion_label' class='col-xs-6'>"\
                                    "<label>Идентификатор: </label>"\
                                    "<label id='id_label'>1234567890</label>"\
                                "</div>"\
                                "<div id='battery' class='col-xs-6 right'>"\
                                    "<label>Напряжение батареи: </label>"\
                                    "<label id='battery_label'>4.8 V</label>"\
                                "</div>"\
                            "</div>"\
                            "<div class='clear'></div>"\
                            "<div class='row col-xs-12'>"\
                                "<div class='row col-xs-4'></div>"\
                                "<div class='row col-xs-4'>"\
                                    "<button id='bt_save' class='btn btn-primary col-xs-12' onclick='saveSettings()'>Запомнить настройки</button>"\
                                "</div>"\
                                "<div class='row col-xs-4'></div>"\
                            "</div>"\
                        "</div>"\
                    "</div>"\
                    "<div id='author' class='col-xs-12 right'>"\
                    "<label style='font-size:10px'>ООО \"ИнфоТехСервис\":</label>"\
                    "<label style='font-size:10px'>(812)34-77-99-8</label>"\
                    "</dev>"\
                "</div>"\
            "</div>"\
        "</body>"\
        "<script>"\
            "function changeSetting(id_){"\
                "e=document.getElementById(id_);"\
                "if(e && e.value){"\
                    "e.placeholder=e.value;"\
                    "e.value='';"\
                "}}"\
            "function saveSettings(){"\
                "var xhr=new XMLHttpRequest();"\
                "xhr.open('POST',window.location+'/save',true);"\
                "xhr.setRequestHeader('Content-type','application/x-www-form-urlencoded');"\
                "xhr.send('json='+JSON.stringify({"\
                    "wifi:{"\
                        "ssid:document.getElementById('server_ssid').value,"\
                        "pswd:document.getElementById('server_pswd').value},"\
                    "url:document.getElementById('service_url').value}));"\
                    "changeSetting('service_url');"\
                    "changeSetting('server_ssid');"\
                    "changeSetting('server_pswd');}"\
        "</script>"\
    "</html>";
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PConfigureServer ConfigureServer::_conf_srv;


ConfigureServer* ConfigureServer::get(const String& srv_ssid, const String& srv_pswd) {
    if (not _conf_srv) {
        _conf_srv.reset(new ConfigureServer(srv_ssid, srv_pswd));
    }
    return _conf_srv.get();
}


bool ConfigureServer::resetServer() {
    if (not _conf_srv) {
        _conf_srv.reset();
        return true;
    }
    return false;
}


void ConfigureServer::handleRoot() {
    ConfigureServer *conf = get();
    if (conf) {
        Blink blk;
        conf->_server.sendContent(PAGE);
        #ifdef DEBUG
        Serial.println("Send content");
        #endif
    }
}


void ConfigureServer::handleSave() {
    ConfigureServer *conf = get();
    if (conf) {
        Blink blk;
        if (conf->_server.args()) {
            String aname = conf->_server.argName(0).c_str();
            if (aname == "json") {
                conf->parseSettings(conf->_server.arg(0));
            }
        }
    }
}


void ConfigureServer::handleNotFound(){
    ConfigureServer *conf = get();
    if (conf) {
        #ifdef DEBUG
        Serial.println("REST is not found.");
        #endif
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
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ConfigureServer::ConfigureServer(const String& srv_ssid, const String& srv_pswd)
    : _srv_ssid(srv_ssid)
    , _srv_pswd(srv_pswd)
    , _server(ESP32WebServer(80)) 
    , _is_complete(false) {
    //WiFi.mode(WIFI_AUTH_WPA2_PSK); // режим клиента
    //WiFi.config(IPAddress(192, 168, 1, 73), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0), IPAddress(192, 168, 1, 1));
    WiFi.softAP(_srv_ssid.c_str(), _srv_pswd.c_str(), 1, 0, 1);
    IPAddress ip = WiFi.softAPIP();
    #ifdef DEBUG
    Serial.println(String("Config server[ " + srv_ssid + "@" + srv_pswd + " ] IP: \"") + ip + String("\""));
    #endif
    _server.on("/", handleRoot);
    _server.on("/save", HTTP_POST, handleSave);
    _server.onNotFound(handleNotFound);
    _server.begin();
    _start_time = time(nullptr);
}


ConfigureServer::~ConfigureServer() {
    WiFi.disconnect(); // обрываем WIFI соединения
    WiFi.softAPdisconnect(); // отключаем отчку доступа(если она была
    WiFi.mode(WIFI_OFF); // отключаем WIFI
    delay(500);
}


void ConfigureServer::execute(void) {
    bool is_timeout = false;
    while (not _is_complete and not is_timeout) {
        _server.handleClient();
        auto cur_time = time(nullptr); 
        is_timeout = (CONFIGURE_TIMEOUT <= (cur_time - _start_time));
        //#ifdef DEBUG
        //Serial.println(String(static_cast<uint32_t>(_start_time), DEC) + "; " + String(static_cast<uint32_t>(cur_time), DEC));
        //#endif
    }
    #ifdef DEBUG
    if (is_timeout and not _is_complete) {
        Serial.println(String("Configure is timeout."));
    } else {
        Serial.println(String("Configure is complete."));
    }
    #endif
}


bool ConfigureServer::parseSettings(const String &j) {
    Blink blk;
    if (j.length()) {
        JsonBufferType jbuf;
        JsonObject& root = jbuf.parseObject(j.c_str());
        if (root.success()) {
            JsonObject &wifi = root["wifi"].as<JsonObject>();
            if (wifi.success()) {
                String ssid =  wifi["ssid"].as<char*>();
                if (ssid.length()) {
                    _wc.ssid = ssid;
                    #ifdef DEBUG
                    Serial.println("Read wifi ssid: \"" + _wc.ssid + "\"");
                    #endif
                }
                String pswd =  wifi["pswd"].as<char*>();
                if (pswd.length()) {
                    _wc.pswd = pswd;
                    #ifdef DEBUG
                    Serial.println("Read wifi pswd: \"" + _wc.pswd + "\"");
                    #endif
                }
            }
            String url = root["url"].as<char*>();
            if (url.length()) {
                _service_url = url;
            }
            _is_complete = true;
        }
        return true;
    }
    return false;
}
