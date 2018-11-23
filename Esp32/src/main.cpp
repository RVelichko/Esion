/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Прошивка модуля счётчика.
 * \author Величко Ростислав
 * \date   13.11.2018
 */

#include <Arduino.h>
#include <Esp.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/rtc_io.h>

#include <time.h>

#include <memory>
#include <cstdlib>
#include <functional>
#include <vector>

#include <WebSocketsClient.h>
#include <ArduinoJson.h>

#include <FS.h>
#include <SPI.h>
#include <SD.h>

#define DEBUG


static const int THRESHOLD = 100; /* Greater the value, more the sensitivity */

static const uint8_t LED_PIN = 2; 
static const uint32_t COUNT_SLEEP_TIME = 500;
static const int ANALOG_PIN = 27; ///< Пин измерения напряжения на пине ADC.

static const uint16_t STR_LEN = 64;
static const uint32_t DEFAULT_SERIAL_SPEED = 115200;
static const char DEFAULT_CONFIG_FILE[] = "config.js";
static const char DEFAULT_COUNTERS_FILE[] = "counters.js";

static const uint16_t MAX_ATTEMPS_CONNECTIONS = 1000;
static const uint32_t DEFAULT_LOCAL_PORT = 20000;
static const char DEFAULT_SERVICE_URL[] = "localhost";
static const char DEFAULT_SERVICE_POINT[] = "/rest/device";
static const uint16_t DEFAULT_WIFI_RECONNECT_TIMEOUT = 1000; /// mlsecs

RTC_DATA_ATTR bool __is_run = false;
RTC_DATA_ATTR uint8_t MAX_COUNT_FOR_SEND = 20;

RTC_DATA_ATTR uint32_t SEND_SLEEP_TIME = 10000;
RTC_DATA_ATTR uint8_t __pin_states = 0;
//RTC_DATA_ATTR time_t __timeout = 0;

RTC_DATA_ATTR uint32_t __count1 = 0;
RTC_DATA_ATTR uint32_t __count2 = 0;
RTC_DATA_ATTR uint32_t __count3 = 0;
RTC_DATA_ATTR uint32_t __count4 = 0;
RTC_DATA_ATTR uint32_t __count5 = 0;
RTC_DATA_ATTR uint32_t __count6 = 0;
RTC_DATA_ATTR uint32_t __last_max_count = 0;

typedef StaticJsonBuffer<300> JsonBufferType;

class Esion;
typedef std::unique_ptr<Esion> PEsion;

class SdController;
typedef std::unique_ptr<SdController> PSdController;

class WebSocketCountsSender;
typedef std::unique_ptr<WebSocketCountsSender> PWebSocketCountsSender;


struct Blink {
    Blink() {
        digitalWrite(LED_PIN, LOW); ///< Зажечь светодиод.
    }
    ~Blink() {
        digitalWrite(LED_PIN, HIGH); ///< Погасить светодиод.
    }
};


struct WifiConfig {
    String _ssid;
    String _pswd;
    
    WifiConfig() 
        : _ssid({""})
        , _pswd({""})
    {}
};


union BatteryValue {
    struct Bits {
        uint8_t _0:1;
        uint8_t _1:1;
    } _bits;
    uint8_t _val:2;
};


bool CheckMaxCounts() {
    uint32_t max = __count1;
    if (max < __count2) {
        max = __count2;
    }
    if (max < __count3) {
        max = __count3;
    }
    if (max < __count4) {
        max = __count4;
    }
    if (max < __count5) {
        max = __count5;
    }
    if (max < __count6) {
        max = __count6;
    }
    if (MAX_COUNT_FOR_SEND <= max - __last_max_count) {
        #ifdef DEBUG
        Serial.println("---> max = " + String(max, DEC) + " : last = " + String(__last_max_count, DEC)); 
        #endif
        __last_max_count = max;
        return true;
    }
    #ifdef DEBUG
    Serial.println("---> max = " + String(max, DEC)); 
    #endif
    return false;
}


class SdController {
    bool _is_inited_sd;

public:
    WifiConfig _wf_conf;
    String _service_addr;
    int _service_port;
    String _service_url;
    String _service_room_id;
    int _service_timeout;

    SdController() {
        /// Инициализация SD карты.
        #ifdef DEBUG
        Serial.print("SD initialization is ");
        #endif
        if (not SD.begin()) {
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

    ~SdController() {
        if (_is_inited_sd) {
            SD.end();
        }
    }

    /**
     * \brief Метод выполняет чтение SD карты и получения файла с настройками.
     */ 
    void getConfig() {
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
                String json_str;
                while (f.available()) {
                    char ch = f.read(); ///< Для строкового представления обязательно читать в чар переменную.
                    json_str += ch;
                }
                f.close();
                parseSettings(json_str);
            } else {
                #ifdef DEBUG
                Serial.println("ERROR: can`t open file `" + String(DEFAULT_CONFIG_FILE) + "`");
                #endif
            }
        }
    }

    void getCounts() {
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
                while (f.available()) {
                    char ch = f.read();
                    json_str += ch;
                }
                f.close();
                parseCounters(json_str);
            } else {
                #ifdef DEBUG
                Serial.println("WARNING: can`t open file `" + String(DEFAULT_COUNTERS_FILE) + "`");
                #endif
            }
        }
    }


    /**
     * \brief Метод выполняет разбор конфигурационного JSON.
     *        Пример конфигурации:
     *          {
     *              "settings":{
     *                  "wifi":{
     *                      "ssid":"Infotec-Service",
     *                      "pswd":"Infotec-Service Best ofthe best"
     *                  },
     *                  "service":{
     *                      "address":"127.0.0.1",
     *                      "port":20000,
     *                      "rest":"/rest/device",
     *                      "room_id":"esion_1",
     *                      "timeout":1 
     *                  }
     *              },
     *              "max_count":1000
     *          }
     */  
    bool parseSettings(const String &sjson) {
        Blink blk;
        if (sjson.length() not_eq 0) {
            JsonBufferType json_buf;
            JsonVariant var = json_buf.parse(const_cast<char*>(sjson.c_str()));
            if (var.is<JsonObject>()) {
                #ifdef DEBUG
                Serial.println(String("Config is parsed...").c_str());
                #endif
                JsonObject& root = var;
                JsonObject& settings = root["settings"].as<JsonObject>();
                if (settings.success()) {
                    JsonObject &wifi = settings["wifi"].as<JsonObject>();
                    if (wifi.success()) {
                        String ssid =  wifi["ssid"].as<char*>();
                        if (ssid.length() not_eq 0) {
                            _wf_conf._ssid = ssid;
                            #ifdef DEBUG
                            Serial.println(String("Read wifi ssid: " + _wf_conf._ssid).c_str());
                            #endif
                        } else {
                            #ifdef DEBUG
                            Serial.println(String("ERROR: can`t find WIFI SSID").c_str());
                            #endif
                        }
                        String pswd =  wifi["pswd"].as<char*>();
                        if (pswd.length() not_eq 0) {
                            _wf_conf._pswd = pswd;
                            #ifdef DEBUG
                            Serial.println("Read wifi pswd: " + _wf_conf._pswd);
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
                    JsonObject& service = settings["service"].as<JsonObject>();
                    if (service.success()) {
                        _service_addr = service["address"].as<char*>();
                        _service_port = service["port"].as<int>();
                        _service_url = service["rest"].as<char*>();
                        _service_room_id = service["room_id"].as<char*>();
                        _service_timeout = service["timeout"].as<int>();
                        #ifdef DEBUG
                        Serial.println(String("Rrad service: " + _service_addr + ":" +
                                       String(_service_port, DEC) + _service_url + "?" + _service_room_id + "@" +
                                       String(_service_timeout, DEC)).c_str());
                        #endif
                    } else {
                        #ifdef DEBUG
                        Serial.println("ERROR: can`t find config service");
                        #endif
                    }
                    SEND_SLEEP_TIME = settings["send_timeout"].as<uint32_t>();
                    if (not SEND_SLEEP_TIME) {
                        SEND_SLEEP_TIME = 10;
                    }
                    #ifdef DEBUG
                    Serial.println("Read send_timeout: " + String(SEND_SLEEP_TIME, DEC));
                    #endif
                    MAX_COUNT_FOR_SEND = settings["max_count"].as<uint8_t>();
                    if (not MAX_COUNT_FOR_SEND) {
                        MAX_COUNT_FOR_SEND = 10;
                    }
                    #ifdef DEBUG
                    Serial.println("Read max_count: " + String(MAX_COUNT_FOR_SEND, DEC));
                    #endif
                } else {
                    #ifdef DEBUG
                    Serial.println("ERROR: can`t find config settings");
                    #endif
                }
            } else {
                #ifdef DEBUG
                Serial.println("ERROR: can`t parse config json");
                #endif
            }
            return true;
        }
        return false;
    }

    /**
     * \brief Метод выполняет разбор файла с сцётчиками JSON.
     *        Пример конфигурации:
     *          {
     *              "counters":[0,0,0,0,0,0]
     *          }
     */  
    bool parseCounters(const String &sjson) {
        Blink blk;
        if (sjson.length() not_eq 0) {
            JsonBufferType json_buf;
            JsonVariant var = json_buf.parse(const_cast<char*>(sjson.c_str()));
            if (var.is<JsonObject>()) {
                #ifdef DEBUG
                Serial.println(String("Config is parsed...").c_str());
                #endif
                JsonObject& root = var;
                JsonArray& counters = root["counters"].as<JsonArray>();
                if (counters.success()) {
                    __count1 = counters[0].as<uint32_t>();
                    __count2 = counters[1].as<uint32_t>();
                    __count3 = counters[2].as<uint32_t>();
                    __count4 = counters[3].as<uint32_t>();
                    __count5 = counters[4].as<uint32_t>();
                    __count6 = counters[5].as<uint32_t>();
                    #ifdef DEBUG
                    Serial.println("Count 1: " + String(__count1, DEC)); 
                    Serial.println("Count 2: " + String(__count2, DEC)); 
                    Serial.println("Count 3: " + String(__count3, DEC)); 
                    Serial.println("Count 4: " + String(__count4, DEC)); 
                    Serial.println("Count 5: " + String(__count5, DEC)); 
                    Serial.println("Count 6: " + String(__count6, DEC)); 
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

    void saveCounters() {
        if (_is_inited_sd) {
            String counts;
            counts += "{\"counters\":[";
            counts += String(__count1, DEC) + ",";
            counts += String(__count2, DEC) + ",";
            counts += String(__count3, DEC) + ",";
            counts += String(__count4, DEC) + ",";
            counts += String(__count5, DEC) + ",";
            counts += String(__count6, DEC);
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
                #ifdef DEBUG
                Serial.println("ERROR: Can`t open counters file for writing.");
                #endif
            }
        }
    }
};


PSdController __sd_ctrl;


/**
 * \brief Класс реализует функции отправки по websocket данных на сервер.
 */ 
class WebSocketCountsSender {
    static PWebSocketCountsSender _ws_sender;
    
    static void event(WStype_t type, uint8_t *payload, size_t length) {
        switch(type) {
            case WStype_CONNECTED: {
                #ifdef DEBUG
                Serial.println("Connected complete.");
                #endif
                _ws_sender->_wsocket.sendTXT(_ws_sender->_json.c_str());
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
                _ws_sender->recvState(String(rs));
            } break;
            case WStype_DISCONNECTED: 
            case WStype_BIN:
            default: 
                break;
        }
    }

    WebSocketsClient _wsocket;
    String _addr; 
    int _port;
    String _url;
    String _room_id;
    String _json;

    /**
     * \brief Метод вызывается Websocket клиентом при получении ответа с сервера. 
     *        Ответ в виде json: 
     *                  { "room_id":"service room id", "status":"ok" }
     *        Выполняет отправку сторожевому контроллеру команду на отключение.
     */ 
    void recvState(const String &srecv) {
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

public:
    bool _is_recv;

    static WebSocketCountsSender* get(const String& addr, int port, const String &url, const String& room_id, const String& json) {
        if (not _ws_sender) {
            _ws_sender.reset(new WebSocketCountsSender(addr, port, url, room_id, json));
        }
        return _ws_sender.get();
    }

    static void reset() {
        _ws_sender.reset();
    }

    explicit WebSocketCountsSender(const String& addr, int port, const String &url, const String& room_id, const String& json) 
        : _addr(addr) 
        , _port(port)
        , _url(url)
        , _room_id(room_id)
        , _json(json) 
        , _is_recv(false) {
        #ifdef DEBUG
        Serial.println("Create websock client: " + _addr + ":" + String(_port, DEC) + _url);
        #endif
        _wsocket.begin(_addr, _port, _url);
        _wsocket.onEvent(event);
        _wsocket.setReconnectInterval(DEFAULT_WIFI_RECONNECT_TIMEOUT);
    }

    ~WebSocketCountsSender() {
        /// Отключиться от сервера.
        _wsocket.disconnect();
        #ifdef DEBUG
        Serial.println(String("Websock client is disconnected").c_str());
        #endif
    }

    void update() {
        _wsocket.loop();
    }
};


PWebSocketCountsSender WebSocketCountsSender::_ws_sender;


/**
 * \brief Класс реализует основные функции WIFI контроллера.
 */ 
class Esion {
public:
    BatteryValue _bat_val;

private:
    int _service_timeout; ///< Время перезапуска отправки данных в минутах.
    WebSocketCountsSender *_ws_sender; ///< Объект отправки на сервер данных по вебсокету.
    double _adc_level; ///< Уровень зарядки аккумуляторов.
    time_t _now;
    SdController* _sdc;
    
    /**
     * \brief Метод выполняет чтение точноего сетевого времени.
     */ 
    time_t getInternetTime() {
        configTime(1 * 3600, 60 * 60, "pool.ntp.org", "time.nist.gov");
        #ifdef DEBUG
        Serial.println("\nWaiting for time");
        #endif
        while(not time(nullptr)) {
            #ifdef DEBUG
            Serial.print(".");
            #endif
            delay(1000);
        }
        #ifdef DEBUG
        Serial.println("");
        #endif
        return time(nullptr);
    }

    /**
     * \brief Метод выполняет считывание значений напряжения питания.
     */ 
    double updateAdcLEvel() {
        int iadc_level = analogRead(ANALOG_PIN);
        if (not iadc_level) {
            iadc_level = 1;
        }
        double adc_level = (static_cast<double>(iadc_level) / 4095.0) * 3.3;
        #ifdef DEBUG
        Serial.println("Bat > " + String(adc_level, DEC) + " V");
        #endif
        return adc_level;
    }

public:
    explicit Esion(SdController* sdc)
        : _adc_level(0.0) 
        , _sdc(sdc) {
        /// Проверить уровень заряда аккумулятора.
        _adc_level = updateAdcLEvel();
        delay(50);
        
        { ///<  Blink
            Blink blk;
            /// Подключение к сети wifi.
            #ifdef DEBUG
            Serial.println("Connecting to WIFI: \"" + sdc->_wf_conf._ssid + "\"|\"" + sdc->_wf_conf._pswd + "\"");
            #endif
            WiFi.begin(sdc->_wf_conf._ssid.c_str(), sdc->_wf_conf._pswd.c_str());
            for (uint16_t i = 0; i < MAX_ATTEMPS_CONNECTIONS && WiFi.status() not_eq WL_CONNECTED; ++i) {
                delay(100);
                #ifdef DEBUG
                Serial.print(".");
                #endif
            } 
            if (WiFi.status() == WL_CONNECTED) {
                #ifdef DEBUG
                Serial.println("WIFI is connected");
                #endif
                /// Получить текущее интернет время.
                _now = getInternetTime();
                #ifdef DEBUG
                Serial.println("Time is: " + String(ctime(&_now)));
                #endif
            } else {
                #ifdef DEBUG
                Serial.println("ERROR: Can`t connet to WIFI.");
                #endif
            }
            delay(50);
        }

    }
    
    void sendCounters() {
        String counts;
        counts += String(__count1, DEC) + ",";
        counts += String(__count2, DEC) + ",";
        counts += String(__count3, DEC) + ",";
        counts += String(__count4, DEC) + ",";
        counts += String(__count5, DEC) + ",";
        counts += String(__count6, DEC);
        String data_sjson = String("{\"room_id\":\"" + _sdc->_service_room_id + "\"" +
                    ", \"msg\":{\"time\":" + String(static_cast<uint32_t>(_now)) +
                    ", \"bat\":" + String(_adc_level, DEC) + 
                    ", \"counts\":[" + counts + "]}}");
        #ifdef DEBUG
        Serial.println("Recv counters: " + counts);
        #endif
        /// Отправить данные на сервер.
        _ws_sender = WebSocketCountsSender::get(_sdc->_service_addr, _sdc->_service_port, _sdc->_service_url, _sdc->_service_room_id, data_sjson);
        if (_ws_sender) {
            for (uint16_t i = 0; i < MAX_ATTEMPS_CONNECTIONS and not _ws_sender->_is_recv; ++i) {
                delay(10);
                _ws_sender->update();
            }
        } else {
            #ifdef DEBUG
            Serial.println("ERROR: Can`t send data? Server is not responce.");
            #endif
        }
    }
}; 


void SendTimeout() {
    #ifdef DEBUG
    Serial.println("SEND ============================"); 
    Serial.println("Count 1: " + String(__count1, DEC)); 
    Serial.println("Count 2: " + String(__count2, DEC)); 
    Serial.println("Count 3: " + String(__count3, DEC)); 
    Serial.println("Count 4: " + String(__count4, DEC)); 
    Serial.println("Count 5: " + String(__count5, DEC)); 
    Serial.println("Count 6: " + String(__count6, DEC)); 
    Serial.println("---------------------------------"); 
    #endif
    __sd_ctrl.reset(new SdController());
    if (__sd_ctrl) {
        __sd_ctrl->getConfig();
        __sd_ctrl->saveCounters();
        Esion e(__sd_ctrl.get());
        e.sendCounters();
    }
}


void WakeupReason() {
    uint32_t wakeup_reason = static_cast<uint32_t>(esp_sleep_get_wakeup_cause());
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wu_bit = static_cast<uint64_t>(esp_sleep_get_ext1_wakeup_status());
            #ifdef DEBUG
            Serial.println("Ext1 status: " + String(reinterpret_cast<uint32_t*>(&wu_bit)[1], DEC));
            #endif
            if (wu_bit & GPIO_SEL_32) {
                ++__count1;
                #ifdef DEBUG
                Serial.println("----------> PIN 32: " + String(__count1, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_33) {
                ++__count2;
                #ifdef DEBUG
                Serial.println("----------> PIN 33: " + String(__count2, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_34) {
                ++__count3;
                #ifdef DEBUG
                Serial.println("----------> PIN 34: " + String(__count3, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_35) {
                ++__count4;
                #ifdef DEBUG
                Serial.println("----------> PIN 35: " + String(__count4, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_36) {
                ++__count5;
                #ifdef DEBUG
                Serial.println("----------> PIN 36: " + String(__count5, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_39) {
                ++__count6;
                #ifdef DEBUG
                Serial.println("----------> PIN 39: " + String(__count6, DEC));
                #endif
            }
        } break;
        case ESP_SLEEP_WAKEUP_TIMER: 
            SendTimeout();
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
        case ESP_SLEEP_WAKEUP_ULP:
        default: 
            Serial.println("Wake up reason: " + String(wakeup_reason, DEC));
            break;
    }
}


void setup() {
    delay(100);
    #ifdef DEBUG
    Serial.begin(DEFAULT_SERIAL_SPEED);
    #endif

    if (__is_run) {
        WakeupReason();
    } else {
        delay(3000);
        #ifdef DEBUG
        Serial.println("Start. ==============================");
        #endif
        __is_run = true;
        __sd_ctrl.reset(new SdController());
        /// Прочитать файл настроек.
        __sd_ctrl->getConfig();
        __sd_ctrl->getCounts();
    }

    esp_sleep_enable_ext1_wakeup(GPIO_SEL_39 | GPIO_SEL_36, ESP_EXT1_WAKEUP_ANY_HIGH);
    bool check = CheckMaxCounts();
    if (check) {
        esp_sleep_enable_timer_wakeup(SEND_SLEEP_TIME * 1000);
    }

    #ifdef DEBUG
    Serial.println("To sleep. -----------------------------");
    Serial.flush();
    #endif
    esp_deep_sleep_start();
}


void loop() {
}


