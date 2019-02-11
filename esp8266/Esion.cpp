/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Тестовая прошивка сервера.
 * \author Величко Ростислав
 * \date   11.12.2017
 */

//#define FS_NO_GLOBALS //allow spiffs to coexist with SD card, define BEFORE including FS.h

#include <time.h>

#include <memory>
#include <cstdlib>
#include <functional>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <iarduino_RTC.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>


/// Включить режим измерения напряжения на пине ADC.
ADC_MODE(ADC_VCC);


static const int STR_LEN = 64;
static const int WIFI_SERVER_PORT = 80;
static const int DEFAULT_SERIAL_SPEED = 115200;

static const int DEFAULT_LOCAL_PORT = 80;
static const char DEFAULT_SERVICE_URL[] = "undefined";

/// GPIO пины
static const int LED_PIN = 2;
static const int COUNTER_INPULS_PIN = 5;

static const int RTC_RST_PIN = 16;
static const int RTC_DAT_PIN = 3;
static const int RTC_CLK_PIN = 0;

static const int SD_CS_PIN = SS;

static const char PAGE[] =
    "<!DOCTYPE html><html>\n"\
    "<head><title>ESION</title><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script><script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script></head>\n"\
    "<style>\n"\
    "body{min-width:1000px;font-family:'Times New Roman',Times,serif;overflow-x: hidden;}\n"\
    "input{margin-bottom:3px;margin-top:0px;}\n"\
    "label{margin-left:3px;margin-right:3px;font-size:12pt;}\n"\
    ".clear{clear:both;}\n"\
    ".row{margin-right:-5px;margin-left:-5px;}\n"\
    ".row:before,.row:after{display:table;content:' ';}\n"\
    ".row:after {clear: both;}\n"\
    ".col-xs-1,.col-xs-2,.col-xs-3,.col-xs-4,.col-xs-5,.col-xs-6,.col-xs-7,.col-xs-8,.col-xs-9,.col-xs-10,.col-xs-11,.col-xs-12{position:relative;min-height:1px;padding-bottom:5px;padding-top:5px;padding-right:10px;padding-left:10px;float:left;}\n"\
    ".border{border:1px solid rgb(0,200,0);border-radius:3px;}\n"\
    ".chart,.msg{width:95%;margin:3px 0;padding:10px;font-size:14px;line-height:16px;text-align:left;color:#333333;border-radius:3px;}\n"\
    ".msg,.time{display:block;float:right;font-size:10px;line-height:17px;}\n"\
    ".msg-left{float:left;background:#e8e8e8;}\n"\
    ".msg-right{float:right;background:#c5eee1;}\n"\
    "</style><body><div class='col-xs-12'><div id='esion_container' class='border col-xs-12'>\n"\
    "<div class='row col-xs-12'><div id='esion_label' class='col-xs-10'><h2>ESION</h2></div>\n"\
    "<div id='esion_version' class='col-xs-2' style='text-align: right;'><h3>V 1.0</h3></div></div>\n"\
    "<div id='settings_container' class='col-xs-7'><div class='border col-xs-12'><div class='col-xs-12'>\n"\
    "<div id='wifi_container'><div class='row col-xs-12'><div class='row col-xs-10'>\n"\
    "<input id='wifi_ssid' class='col-xs-12' type='text' placeholder='wifi SSID'></div><div class='row col-xs-2'>\n"\
    "<button id='save_wifi_ssid' class='btn btn-success col-xs-12'>SET</button></div></div>\n"\
    "<div class='row col-xs-12'><div class='row col-xs-10'><input id='wifi_pswd' class='col-xs-12' type='text' placeholder='wifi PSWD'></div>\n"\
    "<div class='row col-xs-2'><button id='save_wifi_pswd' class='btn btn-success col-xs-12'>SET</button></div></div></div>\n"\
    "<div id='service_container'><div class='row col-xs-12'><div class='row col-xs-10'>\n"\
    "<input id='service_url' class='col-xs-12' type='text' placeholder='Service URL'></div><div class='row col-xs-2'>\n"\
    "<button id='save_service_url' class='btn btn-success col-xs-12'>SET</button></div></div></div>\n"\
    "<div class='clear'></div></div><div class='col-xs-12'><div id='sensor_1' class='row col-xs-12'>\n"\
    "<label>sencor 1: </label><label id='sensor_1_label'>0</label></div><div id='sensor_2' class='row col-xs-12'>\n"\
    "<label>sencor 2: </label><label id='sensor_2_label'>0</label></div><div class='clear'></div>\n"\
    "<div id='time' class='row col-xs-12'><label>time: </label><label id='time_label'>00:00:00</label></div>\n"\
    "<div id='battary' class='row col-xs-12'><label>battary: </label><label id='battary_label'>3.3 V</label></div>\n"\
    "<div class='clear'></div></div><div class='row col-xs-12'><div class='row col-xs-4'></div>\n"\
    "<div class='row col-xs-4'><button id='reset_page' class='btn btn-primary col-xs-12'>RESET PAGE</button></div>\n"\
    "<div class='row col-xs-4'></div></div></div></div><div class='col-xs-5'><div id='log_container' class='border col-xs-12'>\n"\
    "<div id='log_label' style='text-align:center;'><h3>Logging</h3></div><div id='esion_log' style='overflow-y:auto;'>\n"\
    "<div id='log' class='chart'></div><div class='clear'></div></div></div></div></div></div></body>\n"\
    "<script>\n"\
    "window.is_reset_page = false;\n"\
    "window.log_lines_max_num = 100;\n"\
    "function sendSettings() {\n"\
    "  var s = {\n"\
    "    wifi:{\n"\
    "      ssid:$('#wifi_ssid').val(),\n"\
    "      pswd:$('#wifi_pswd').val()\n"\
    "    },\n"\
    "    admin:{surl:$('#service_url').val()}\n"\
    "  };\n"\
    "  var xhr = new XMLHttpRequest();\n"\
    "  xhr.open('post', window.location + 'args');\n"\
    "  xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');\n"\
    "  xhr.send('json=' + JSON.stringify(s));\n"\
    "}\n"\
    "function setScrollLogHeight() {\n"\
    "  var h = $('#settings_container').height() - $('#log_label').height() - 50;\n"\
    "  $('#esion_log').css('max-height', h);\n"\
    "  console.log('# ' + h);\n"\
    "}\n"\
    "function setLogHeight() {\n"\
    "  var h = $('#settings_container').height();\n"\
    "  $('#log_container').css('height', h);\n"\
    "  console.log('# ' + h);\n"\
    "}\n"\
    "function updateLog() {\n"\
    "  if ($('#log .msg').length > window.log_lines_max_num) {\n"\
    "    $('#log .msg:last').remove();\n"\
    "  }\n"\
    "}\n"\
    "function addLeftLog(msg) {\n"\
    "  var tops = {\n"\
    "    year: 'numeric', month: 'numeric', day: 'numeric',\n"\
    "    hour: 'numeric', minute: 'numeric', second: 'numeric'\n"\
    "  };\n"\
    "  $('#log').prepend('<div class=\"msg msg-left\">' + msg + '</div>');\n"\
    "  updateLog();\n"\
    "  setScrollLogHeight();\n"\
    "}\n"\
    "function addRightLog(msg) {\n"\
    "  var tops = {\n"\
    "    year: 'numeric', month: 'numeric', day: 'numeric',\n"\
    "    hour: 'numeric', minute: 'numeric', second: 'numeric'\n"\
    "  };\n"\
    "  var t = new Date();\n"\
    "  $('#log').prepend('<div class=\"msg msg-right\">' + t.toLocaleString('ru-RU', tops) + ' | ' + msg + '</div>');\n"\
    "  updateLog();\n"\
    "  setScrollLogHeight();\n"\
    "}\n"\
    "function changeSetting(id_, prefix_) {\n"\
    "  var v = $(id_).val();\n"\
    "  if (v) {\n"\
    "    $(id_).attr('placeholder', v);\n"\
    "    sendSettings();\n"\
    "    $(id_).val('');\n"\
    "    addRightLog(prefix_ + ': \"' + v + '\"');\n"\
    "  }\n"\
    "}\n"\
    "function handleSettings() {\n"\
    "  $('#save_wifi_ssid').click(function() {\n"\
    "    if ( ! window.is_reset_page) {сhangeSetting('#wifi_ssid', 'ssid');}\n"\
    "  });\n"\
    "  $('#save_wifi_pswd').click(function() {\n"\
    "    if ( ! window.is_reset_page) {changeSetting('#wifi_pswd', 'pswd');}\n"\
    "  });\n"\
    "  $('#save_service_url').click(function() {\n"\
    "    if ( ! window.is_reset_page) {changeSetting('#service_url', 'service url');}\n"\
    "  });\n"\
    "  $('#reset_page').click(function() {\n"\
    "    window.is_reset_page = true;\n"\
    "    setTimeout(function() {location.reload();}, 3000);\n"\
    "  });\n"\
    "}\n"\
    "function setStatus(json_) {\n"\
    "  if (typeof json_.time !== 'undefined') {$('#time_label').html(json_.time);}\n"\
    "  if (typeof json_.sensor1 !== 'undefined') {$('#sensor_1_label').html(json_.sensor1);}\n"\
    "  if (typeof json_.sensor2 !== 'undefined') {$('#sensor_2_label').html(json_.sensor2);}\n"\
    "  if (typeof json_.battary !== 'undefined') {$('#battary_label').html(json_.battary + ' V');}\n"\
    "}\n"\
    "function setSettings(json_) {\n"\
    "  if (typeof json_.wifi !== 'undefined') {\n"\
    "    if (typeof json_.wifi.ssid !== 'undefined') {$('#wifi_ssid').attr('placeholder', json_.wifi.ssid);}\n"\
    "    if (typeof json_.wifi.pswd !== 'undefined') {$('#wifi_pswd').attr('placeholder', json_.wifi.pswd);}\n"\
    "  }\n"\
    "  if (typeof json_.admin !== 'undefined') {\n"\
    "    if (typeof json_.admin.service_url !== 'undefined') {$('#service_url').attr('placeholder', json_.service_url);}\n"\
    "  }\n"\
    "}\n"\
    "function setUpdateTimeout() {\n"\
    "  var timerId = setInterval(function() {\n"\
    "    console.log('Update status');\n"\
    "    $.getJSON(window.location + 'args?update=' + new Date().getTime())\n"\
    "    .done(function(json_) {\n"\
    "      if (typeof json_.status !== 'undefined' && typeof json_.status.updated !== 'undefined' && json_.status.updated === 1) {\n"\
    "        setStatus(json_.status);\n"\
    "        addLeftLog(JSON.stringify(json_));\n"\
    "      }\n"\
    "    });\n"\
    "  }, 10000);\n"\
    "}\n"\
    "function getInit() {\n"\
    "  $.getJSON(window.location + 'args?init=' + new Date().getTime())\n"\
    "    .done(function(json_) {\n"\
    "      if (typeof json_.status !== 'undefined') {setStatus(json_.status);}\n"\
    "      if (typeof json_.settings !== 'undefined') {setSettings(json_.settings);}\n"\
    "      console.log('Init status');\n"\
    "    });\n"\
    "}\n"\
    "$(window).resize(function() {\n"\
    "  setLogHeight();\n"\
    "  setScrollLogHeight();\n"\
    "});\n"\
    "$(document).ready(function() {\n"\
    "  setLogHeight();\n"\
    "  setScrollLogHeight();\n"\
    "  handleSettings();\n"\
    "  setUpdateTimeout();\n"\
    "});\n"\
    "</script>\n"\
    "</html>";

class Esion {
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

    struct WifiConfig {
        String _ssid;
        String _pswd;

        WifiConfig()
            : _ssid({"Infotec-Service"})
            , _pswd({"Infotec-Service Best of the best"})
        {}
    };

    static std::shared_ptr<Esion> _esion;

    static int _counter;
    static char _time_str[STR_LEN];

public:
    union BatteryValue {
        struct Bits {
            uint8_t _0:1;
            uint8_t _1:1;
        } _bits;
        uint8_t _val:2;
    };

    static BatteryValue _bat_val;

    static Esion* getEsion() {
        if (not _esion) {
            _esion = std::make_shared<Esion>(DEFAULT_SERIAL_SPEED);
        }
        return _esion.get();
    }

private:
    static void handleRoot() {
        Esion *esion = getEsion();
        if (esion) {
            Blink blk;
            esion->getServer()->sendContent(PAGE);
            Serial.println("Send content");
        }
    }

    static void handleArgs() {
        Esion *esion = getEsion();
        if (esion) {
            Blink blk;
            ESP8266WebServer *server = esion->getServer();
            if (server and server->args() not_eq 0) {
                String aname = server->argName(0).c_str();
                if (aname == "init") {
                    esion->sendInitStatusJson();
                }
                if (aname == "update") {
                    esion->sendUpdateStatusJson();
                }
                if (aname == "json") {
                    esion->recvJson(server->arg(0).c_str());
                }
            }
        }
    }

    static void handleNotFound(){
        Esion *esion = getEsion();
        if (esion) {
            Blink blk;
            ESP8266WebServer *srv = esion->getServer();
            String message = "Page Not Found\n\n";
            message += "URI: ";
            message += srv->uri();
            message += "\nMethod: ";
            message += (srv->method() == HTTP_GET)?"GET":"POST";
            message += "\nArguments: ";
            message += srv->args();
            message += "\n";
            for (uint8_t i=0; i < srv->args(); i++){
                message += " " + srv->argName(i) + ": " + srv->arg(i) + "\n";
            }
            srv->send(404, "text/plain", message);
        }
    }

    static void handleCounterInterrupt() {
        Esion *esion = getEsion();
        if (esion) {
            if (digitalRead(COUNTER_INPULS_PIN)) {
                esion->saveCurTimeToStr();
                int c = esion->incCounter();
                Serial.print("Input counter [");
                Serial.print(c);
                Serial.print("]: ");
                Serial.println(esion->getTimeStr());
                /// Зажечь светодиод.
                digitalWrite(LED_PIN, LOW);
            } else {
                /// Погасить светодиод.
                digitalWrite(LED_PIN, HIGH);
            }
        }
    }

    bool _is_inited_sd;
    std::shared_ptr<iarduino_RTC> _rtc;
    std::shared_ptr<ESP8266WebServer> _server;
    double _adc_level;
    WifiConfig _wc;
    String _service_url;

    ESP8266WebServer* getServer() {
        return _server.get();
    }

    bool parseSettings(const String &json_str_) {
        Blink blk;
        if (json_str_.length() not_eq 0) {
            StaticJsonBuffer<200> json_buf;
            JsonObject& root = json_buf.parseObject(json_str_.c_str());
            if (root.success()) {
                JsonObject& settings = root["settings"].asObject();
                if (settings.success()) {
                    JsonObject &wifi = settings["wifi"].asObject();
                    if (wifi.success()) {
                        String ssid =  wifi["ssid"].asString();
                        if (ssid.length() not_eq 0) {
                            _wc._ssid = ssid;
                            Serial.print("Read wifi ssid: ");
                            Serial.println(_wc._ssid.c_str());
                        }
                        String pswd =  wifi["pswd"].asString();
                        if (pswd.length() not_eq 0) {
                            _wc._pswd = pswd;
                            Serial.print("Read wifi pswd: ");
                            Serial.println(_wc._pswd.c_str());
                        }
                    }
                    JsonObject& admin = settings["admin"].asObject();
                    if (admin.success()) {
                        String service_url =  admin["service_url"].asString();
                        if (service_url.length() not_eq 0) {
                            _service_url = service_url;
                            Serial.print("Read service url: ");
                            Serial.println(_service_url.c_str());
                        }
                    }
                }
            }
            return true;
        }
        return false;
    }

    void getConfig() {
        Serial.println("Read config file...");
        if (_is_inited_sd) {
            File file = SD.open("config.js", FILE_READ);
            int fsize = file.size();
            Serial.print("disk size: ");
            Serial.println(fsize);
            if (fsize) {
                String json_str;
                while (file.available()) {
                    json_str += file.read();
                }
                file.close();
                parseSettings(json_str);
            } else {
                Serial.println("ERROR: can`t open file `config.json`");
            }
        }
    }

    time_t getInternetTime() {
        configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        Serial.println("\nWaiting for time");
        while(not time(nullptr)) {
            Serial.print(".");
            delay(1000);
        }
        Serial.println("");
        return time(nullptr);
    }

    void saveCurTimeToStr() {
        if (_rtc) {
            strncpy(_time_str, _rtc->gettime("d.m.Y-H:i:s, D"), sizeof(_time_str));
        }
    }

    int incCounter() {
        ++_counter;
        return _counter;
    }

    char* getTimeStr() {
        return _time_str;
    }

    void sendInitStatusJson() {
        Blink blk;
        StaticJsonBuffer<200> json_buf;
        JsonObject& root = json_buf.createObject();
        JsonObject& status = root.createNestedObject("status");
        saveCurTimeToStr();
        status["tine"] = getTimeStr();
        status["sensor1"] = _counter;
        status["sensor2"] = 0;
        status["battary"] = _adc_level;
        status["updated"] = 1;
        JsonObject& settings = root.createNestedObject("settings");
        JsonObject& wifi = settings.createNestedObject("wifi");
        wifi["ssid"] = _wc._ssid;
        wifi["pswd"] = _wc._pswd;
        JsonObject& admin = settings.createNestedObject("admin");
        admin["service_url"] = _service_url;
        String json_str;
        root.printTo(json_str);
        _server->send(200, "text/json", json_str);
        Serial.print("Init status JSON: ");
        Serial.println(json_str);
    }

    void sendUpdateStatusJson() {
        Blink blk;
        StaticJsonBuffer<200> json_buf;
        JsonObject& root = json_buf.createObject();
        JsonObject& status = root.createNestedObject("status");
        time_t now = getInternetTime();
        status["tine"] = ctime(&now);
        status["sensor1"] = _counter;
        status["sensor2"] = 0;
        status["battary"] = _adc_level;
        status["updated"] = 1;
        String json_str;
        root.printTo(json_str);
        _server->send(200, "text/json", json_str);
        Serial.print("Update status JSON: ");
        Serial.println(json_str);
    }

    void recvJson(const String &json_str_) {
        /// Выделить принятые параметры.
        if (parseSettings(json_str_)) {
            Blink blk;
            /// Обновить файл конфигурации.
            File file = SD.open("config.js", FILE_WRITE);
            if (file) {
                file.print(json_str_);
                file.close();
                Serial.print("Recv JSON: ");
                Serial.println(json_str_);
            }
        }
    }

public:
    explicit Esion(int serial_speed_)
        : _is_inited_sd(false)
        , _adc_level(0)
        , _service_url(DEFAULT_SERVICE_URL) {
        /// Инициализация порта.
        Serial.begin(serial_speed_);
        while (not Serial) {
            delay(10);
        }

        /// Инициализация SD карты.
        Serial.print("SD initialization is ");
        if (not SD.begin()) {
            Serial.println(SD.fatType());
            Serial.print("ERROR: ");
        } else {
            Serial.println("TRUE");
            _is_inited_sd = true;
        }

        /// Прочитать файл настроек.
        getConfig();
        delay(100);

        /// Подключение к сети wifi.
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(_wc._ssid.c_str());

        WiFi.begin(_wc._ssid.c_str(), _wc._pswd.c_str());

        while (WiFi.status() not_eq WL_CONNECTED) {
            delay(1000);
            Serial.print(".");
        }
        Serial.println("WiFi connected");

        /// Получить текущее интернет время.
        time_t now = getInternetTime();
        Serial.println(ctime(&now));

        /// Инициализация пина светодиода.
        pinMode(LED_PIN, OUTPUT);
        Blink blk;
        Serial.print("Init led pin: ");
        Serial.println(LED_PIN);

        /// Запустить сервер.
        _server = std::make_shared<ESP8266WebServer>(DEFAULT_LOCAL_PORT);
        Serial.print("Server started by port: ");
        Serial.println(DEFAULT_LOCAL_PORT);
        /// Привязать обработчики к серверу.
        _server->on("/", handleRoot);
        _server->on("/args", handleArgs);
        _server->onNotFound(handleNotFound);
        _server->begin();

        /// Показать IP адрес.
        Serial.print("Use this URL to connect: ");
        Serial.print("http://");
        Serial.print(WiFi.localIP());
        Serial.println("/");
        delay(100);

        /// Инициализация входа для генератора импульсов.
        pinMode(COUNTER_INPULS_PIN, INPUT);
        attachInterrupt(COUNTER_INPULS_PIN, &Esion::handleCounterInterrupt, CHANGE);
        Serial.print("Init counter pin: ");
        Serial.println(COUNTER_INPULS_PIN);
        delay(100);
    }

    void update() {
        /// Проверить уровень заряда аккумулятора.
        //int adc_level = analogRead(A0);
        double adc_level = floor((ESP.getVcc() / 100.0) + 0.5) / 10.0;
        if (adc_level not_eq _adc_level) {
            Serial.print("Bat > ");
            Serial.print(adc_level);
            Serial.println(" V");
            _adc_level = adc_level;
        }

        /// Проверить подключение клиента.
        String aname;
        if (_server) {
            _server->handleClient();
        }
        delay(100);
    }
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::shared_ptr<Esion> Esion::_esion;
int Esion::_counter = 0;
char Esion::_time_str[STR_LEN] = {0};
Esion::BatteryValue Esion::_bat_val = {0};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
    Esion::getEsion();
}


void loop() {
    Esion *e = Esion::getEsion();
    if (e) {
        e->update();
    }
}


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
            "function changeSetting(id_){e=document.getElementById(id_);if(e && e.value){e.placeholder = e.value;e.value = '';}}"\
            "function saveSettings(){var xhr=new XMLHttpRequest();xhr.open('POST',window.location+'/save',true);xhr.setRequestHeader('Content-type','application/x-www-form-urlencoded');xhr.send('json='+JSON.stringify({wifi:{ssid:document.getElementById('server_ssid').value,pswd:document.getElementById('server_pswd').value}}));changeSetting('server_ssid');changeSetting('server_pswd');}"\
        "</script>"\
    "</html>";
