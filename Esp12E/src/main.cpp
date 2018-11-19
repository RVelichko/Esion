/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Тестовая прошивка сервера.
 * \author Величко Ростислав
 * \date   11.12.2017
 */

#include <Arduino.h>
//#define FS_NO_GLOBALS //allow spiffs to coexist with SD card, define BEFORE including FS.h

#include <time.h>

#include <memory>
#include <cstdlib>
#include <functional>

#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


/// Включить режим измерения напряжения на пине ADC.
ADC_MODE(ADC_VCC);


static const uint16_t STR_LEN = 64;
static const uint32_t DEFAULT_SERIAL_SPEED = 115200;

static const char DEFAULT_CONFIG_FILE[] = "config.js";

static const uint32_t DEFAULT_LOCAL_PORT = 20000;
static const char DEFAULT_SERVICE_URL[] = "localhost";
static const char DEFAULT_SERVICE_POINT[] = "/rest/device";
static const uint16_t DEFAULT_WIFI_RECONNECT_TIMEOUT = 1000; /// mlsecs
 
/// GPIO пины
static const uint8_t LED_PIN = 2; 
static const uint8_t COUNTER_INPULS_PIN = 5;

static const uint16_t RTC_RST_PIN = 16;
static const uint16_t RTC_DAT_PIN = 3; 
static const uint16_t RTC_CLK_PIN = 0; 

static const uint16_t SD_CS_PIN = SS;

static const uint8_t WIRE_ADDRESS = 8;
static const uint8_t COUNTERS_NUM = 6;


typedef StaticJsonBuffer<300> JsonBufferType;

class Esion;
typedef std::unique_ptr<Esion> PEsion;


/**
 * \brief Класс реализует основные функции WIFI контроллера.
 */ 
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
            : _ssid({""})
            , _pswd({""})
        {}
    };

    /**
     * @brief Описание команд при объмене со сторожевым таймером.
     */ 
    enum Commands {
        ESP_OK = 100, ///< Оповещает сторожевой контроллер о готовности принять счётчики.
        ESP_ERR = 101, ///< Оповещает сторожевой контроллер об ошибке.
        ESP_TIMEOUT = 102, ///< НАстраивает сторожевой контроллер на задержку до следующей трансляции счётичиков.
        ESP_RECV = 103 ///< Оповещает сторожевой контроллер об успешной трансляции на сервер данных.
    };

    /**
     * \brief Класс реализует функции отправки по websocket данных на сервер.
     */ 
    class WebSocketCountsSender;
    typedef std::unique_ptr<WebSocketCountsSender> PWebSocketCountsSender;

    class WebSocketCountsSender {
        static PWebSocketCountsSender _ws_sender;
        
        static void event(WStype_t type, uint8_t *payload, size_t length) {
            switch(type) {
                case WStype_DISCONNECTED:
                    Serial.println(String("Disconnecting from " + _ws_sender->_addr + ":" + String(_ws_sender->_port, DEC) + _ws_sender->_url).c_str());
                    break;
                case WStype_CONNECTED: {
                    Serial.println("Connected complete.");
                    _ws_sender->_wsocket.sendTXT(_ws_sender->_esion->getSendData().c_str());
                } break;
                case WStype_TEXT: {
                    std::vector<char> arr(length + 1, '\0');
                    for (size_t i = 0; i < length; ++i) {
                        arr[i] = static_cast<char>(payload[i]);
                    }
                    String rs = String(&arr[0]);
                    Serial.println(String("Recv: " + rs).c_str());
                    _ws_sender->_esion->recvState(String(rs));
                } break;
                case WStype_BIN:
                    break;
                default: break;
            }
        }

        Esion *_esion;
        WebSocketsClient _wsocket;
        String _addr; 
        int _port;
        String _url;

    public:
        static WebSocketCountsSender* get(Esion *esion, const String& addr, int port, const String &url) {
            if (not _ws_sender) {
                _ws_sender.reset(new WebSocketCountsSender(esion, addr, port, url));
            }
            return _ws_sender.get();
        }

        static void reset() {
            _ws_sender.reset();
        }

        explicit WebSocketCountsSender(Esion *esion, const String& addr, int port, const String &url) 
            : _esion(esion)
            , _addr(addr) 
            , _port(port)
            , _url(url) {
            Serial.println(String("Create websock client: " + _addr + ":" + String(_port, DEC) + _url).c_str());
            _wsocket.begin(_addr, _port, _url);
            _wsocket.onEvent(event);
            _wsocket.setReconnectInterval(DEFAULT_WIFI_RECONNECT_TIMEOUT);
        }

        ~WebSocketCountsSender() {
            /// Отключиться от сервера.
            _wsocket.disconnect();
            Serial.println(String("Websock client is disconnected").c_str());
        }

        void update() {
            _wsocket.loop();
        }
    };
    
    static PEsion _esion;

public:
    union BatteryValue {
        struct Bits {
            uint8_t _0:1;
            uint8_t _1:1;
        } _bits;
        uint8_t _val:2;
    };

    static BatteryValue _bat_val;

    static Esion* get() {
        if (not _esion) {
            _esion.reset(new Esion(DEFAULT_SERIAL_SPEED));
        }
        return _esion.get();
    }

    static void onWireReceave(int how_many) {
        Blink blk;
        Esion* e = Esion::get();
        if (e) {
            uint16_t buf[COUNTERS_NUM] = {0};
            uint8_t rc = 0;
            while (1 < Wire.available()) {
                reinterpret_cast<uint8_t*>(buf)[rc] = Wire.read();
                ++rc;
            }
            e->recvCounters(buf);
        }
    }

    //static void noWireRequest() {
    //
    //}

private:
    bool _is_inited_sd;
    WifiConfig _wf_conf;
    String _service_addr;
    int _service_port;
    String _service_url;
    String _service_room_id;
    int _service_timeout; ///< Время перезапуска отправки данных в минутах.
    WebSocketCountsSender *_ws_sender; ///< Объект отправки на сервер данных по вебсокету.
    String _data_sjson; ///< Отправляемые на сервер данные.
    double _adc_level; ///< Уровень зарядки аккумуляторов.
    time_t _now;
    
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
     *              }
     *          }
     */  
    bool parseSettings(const String &sjson) {
        Blink blk;
        if (sjson.length() not_eq 0) {
            JsonBufferType json_buf;
            JsonVariant var = json_buf.parse(const_cast<char*>(sjson.c_str()));
            if (var.is<JsonObject>()) {
                Serial.println(String("Config is parsed...").c_str());
                JsonObject& root = var;
                JsonObject& settings = root["settings"].as<JsonObject>();
                if (settings.success()) {
                    JsonObject &wifi = settings["wifi"].as<JsonObject>();
                    if (wifi.success()) {
                        String ssid =  wifi["ssid"].as<char*>();
                        if (ssid.length() not_eq 0) {
                            _wf_conf._ssid = ssid;
                            Serial.println(String("Read wifi ssid: " + _wf_conf._ssid).c_str());
                        } else {
                            Serial.println(String("ERROR: can`t find WIFI SSID").c_str());
                        }
                        String pswd =  wifi["pswd"].as<char*>();
                        if (pswd.length() not_eq 0) {
                            _wf_conf._pswd = pswd;
                            Serial.println(String("Read wifi pswd: " + _wf_conf._pswd).c_str());
                        } else {
                            Serial.println(String("ERROR: can`t find WIFI PSWD").c_str());
                        }
                    } else {
                        Serial.println(String("ERROR: can`t find config wifi").c_str());
                    }
                    JsonObject& service = settings["service"].as<JsonObject>();
                    if (service.success()) {
                        _service_addr = service["address"].as<char*>();
                        _service_port = service["port"].as<int>();
                        _service_url = service["rest"].as<char*>();
                        _service_room_id = service["room_id"].as<char*>();
                        _service_timeout = service["timeout"].as<int>();
                        Serial.println(String("Rrad service: " + _service_addr + ":" +
                                       String(_service_port, DEC) + _service_url + "?" + _service_room_id + "@" +
                                       String(_service_timeout, DEC)).c_str());
                    } else {
                        Serial.println(String("ERROR: can`t find config service").c_str());
                    }
                } else {
                    Serial.println(String("ERROR: can`t find config settings").c_str());
                }
            } else {
                Serial.println(String("ERROR: can`t parse config json").c_str());
            }
            return true;
        }
        return false;
    }
    
    /**
     * \brief Метод выполняет чтение SD карты и получения файла с настройками.
     */ 
    void getConfig() {
        if (_is_inited_sd) {
            Serial.println("Read config file...");
            File file = SD.open(DEFAULT_CONFIG_FILE, FILE_READ);
            int fsize = file.size();
            Serial.println(String("disk size: " + fsize).c_str());
            if (fsize) {
                String json_str;
                while (file.available()) {
                    char ch = file.read(); ///< Для строкового представления обязательно читать в чар переменную.
                    json_str += ch;
                }
                file.close();
                parseSettings(json_str);
            } else {
                Serial.println("ERROR: can`t open file `" + String(DEFAULT_CONFIG_FILE) + "`");
            }
        }
    }

    /**
     * \brief Метод выполняет чтение точноего сетевого времени.
     */ 
    time_t getInternetTime() {
        configTime(1 * 3600, 60 * 60, "pool.ntp.org", "time.nist.gov");
        Serial.println("\nWaiting for time");
        while(not time(nullptr)) {
            Serial.print(".");
            delay(1000);
        }
        Serial.println("");
        return time(nullptr);
    }

    /**
     * \brief Метод выполняет считывание значений напряжения питания.
     */ 
    double updateAdcLEvel() {
        //int adc_level = analogRead(A0);
        double adc_level = floor((ESP.getVcc() / 100.0) + 0.5) / 10.0;
        Serial.println(String("Bat > " + String(adc_level, DEC) + " V").c_str());
        return adc_level;
    }

    /**
     * \brief Метод отправляет сторожевому контроллеру сообщение об окончании операций.
     */ 
    void wireSendPwdown() {
        Serial.println("SEND POWER DOWN");
        /// Проинформировать о завершении операций.
        Wire.beginTransmission(WIRE_ADDRESS);
        Wire.write(ESP_RECV);
        Wire.endTransmission();
    }

    /**
     * \brief Метод отправляет сторожевому контроллеру сообщение об ошибке при выполнении операций.
     */ 
    void wireSendError() {
        Serial.println("SEND ERROR");
    }

public:
    explicit Esion(int serial_speed_)
        : _is_inited_sd(false)
        , _service_url(DEFAULT_SERVICE_URL) 
        , _adc_level(0.0) {
        /// Инициализация порта.
        Serial.begin(serial_speed_);
        while (not Serial) {
            delay(10);
        }
            
        /// Проверить уровень заряда аккумулятора.
        _adc_level = updateAdcLEvel();
        delay(50);

        /// Инициализация SD карты.
        Serial.print("SD initialization is ");
        if (not SD.begin()) {
            Serial.println(String("ERROR: Fat type is: " + SD.fatType()).c_str());
        } else {
            Serial.println("TRUE");
            _is_inited_sd = true;
        }
       
        /// Прочитать файл настроек.
        getConfig();
        delay(50);
        
        { ///<  Blink
            Blink blk;
            /// Подключение к сети wifi.
            Serial.println(String("\nConnecting to WIFI: " + _wf_conf._ssid).c_str());

            WiFi.begin(_wf_conf._ssid.c_str(), _wf_conf._pswd.c_str());
            
            while (WiFi.status() not_eq WL_CONNECTED) {
                delay(1000);
                Serial.print(".");
            } 
            Serial.println("WIFI is connected");
            delay(50);
        }

        /// Получить текущее интернет время.
        _now = getInternetTime();
        Serial.println(String("Time is: " + String(ctime(&_now))).c_str());
        delay(50);

        /// Получить данные счётчиков.
        Wire.begin();
        Wire.onReceive(onWireReceave);
        //Wire.onRequest(noWireRequest);
        /// Проинформировать о готовности принять счётчики.
        Serial.println("Send to attiny ESP_OK");
        Wire.beginTransmission(WIRE_ADDRESS); // transmit to device #8
        Wire.write(ESP_OK);        // sends five bytes
        Wire.endTransmission();    // stop transmitting
    }
    
    void recvCounters(uint16_t *buf) {
        String counts;
        counts += String(buf[0], DEC) + ",";
        counts += String(buf[1], DEC) + ",";
        counts += String(buf[2], DEC) + ",";
        counts += String(buf[3], DEC) + ",";
        counts += String(buf[4], DEC) + ",";
        counts += String(buf[5], DEC);
        _data_sjson = String("{\"room_id\":\"" + _service_room_id + "\"" +
                    ", \"msg\":{\"time\":" + String(static_cast<uint32_t>(_now)) +
                    ", \"bat\":" + String(_adc_level, DEC) + 
                    ", \"counts\":[" + counts + "]}}");
        Serial.println("Recv counters: " + counts);

        /// Отправить данные на сервер.
        _ws_sender = WebSocketCountsSender::get(this, _service_addr, _service_port, _service_url);
        delay(50);
    }


    void update() {
        if (_ws_sender) {
            _ws_sender->update();
        }
    }

    String getSendData() {
        return _data_sjson;
    }

    /**
     * \brief Метод вызывается Websocket клиентом при получении ответа с сервера. 
     *        Ответ в виде json: 
     *                  { "room_id":"service room id", "status":"ok" }
     *        Выполняет отправку сторожевому контроллеру команду на отключение.
     */ 
    void recvState(const String &srecv) {
        JsonBufferType json_buf;
        JsonObject& root = json_buf.parseObject(srecv.c_str());
        if (root.success()) {
            String room_id = root["room_id"].as<char*>();
            String status = root["status"].as<char*>();
            if (_service_room_id == room_id and status == "ok") {
                wireSendPwdown();
                return;
            } 
        }
        wireSendError();
    }
}; 


Esion::PWebSocketCountsSender Esion::WebSocketCountsSender::_ws_sender;
PEsion Esion::_esion;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
    Esion::get();
}
 
 
void loop() {
    Esion *e = Esion::get();
    if (e) {
        e->update();
    }
}
