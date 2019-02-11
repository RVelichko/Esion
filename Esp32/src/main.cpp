/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Прошивка модуля счётчика.
 * \author Величко Ростислав
 * \date   13.11.2018
 */

/**
 * Для успешной прошивки предварительно необходимо добавить пользователя в группу USB порта.
 * Выяснить имя группы: ls -l /dev/ttyUSB0
 * Добавить в группу: sudo usermod -a -G dialout $USER
 */ 

#include <time.h>

#include <memory>
#include <cstdlib>
#include <functional>
#include <vector>
#include <string>
#include <cctype>


#include <Arduino.h>
#include <Esp.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/rtc_io.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPI.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#include "utils.hpp"
#include "ConfigureServer.hpp"
#include "SdController.hpp"
#include "CountersSender.hpp"


static const int THRESHOLD = 100; /* Greater the value, more the sensitivity */

static const uint32_t COUNT_SLEEP_TIME = 500;
static const int ANALOG_PIN = 27; ///< Пин измерения напряжения на пине ADC.

static const double BATTARY_FULL_VALUE = 4.8;

static const uint16_t STR_LEN = 64;
static const uint32_t DEFAULT_SERIAL_SPEED = 115200;

static const uint16_t MAX_ATTEMPS_CONNECTIONS = 1000;
static const uint32_t DEFAULT_LOCAL_PORT = 20000;
static const char DEFAULT_SERVICE_URL[] = "localhost";
static const char DEFAULT_SERVICE_POINT[] = "/rest/device";


RTC_DATA_ATTR bool __is_run = false;
RTC_DATA_ATTR uint8_t __pin_states = 0;

RTC_DATA_ATTR uint32_t __count1 = 0;
RTC_DATA_ATTR uint32_t __count2 = 0;
RTC_DATA_ATTR uint32_t __count3 = 0;
RTC_DATA_ATTR uint32_t __count4 = 0;
RTC_DATA_ATTR uint32_t __count5 = 0;
RTC_DATA_ATTR uint32_t __count6 = 0;
RTC_DATA_ATTR uint32_t __last_max_count = 0;

RTC_DATA_ATTR uint8_t MAX_COUNT_FOR_SEND = 20;
RTC_DATA_ATTR uint32_t SEND_SLEEP_TIME = 10000;


typedef StaticJsonBuffer<300> JsonBufferType;

class Esion;
typedef std::unique_ptr<Esion> PEsion;


/**
 * \brief Класс реализует основные функции WIFI контроллера.
 */ 
class Esion {
public:
    BatteryValue _bat_val;

private:
    int _service_timeout; ///< Время перезапуска отправки данных в минутах.
    CountersSender *_ws_sender; ///< Объект отправки на сервер данных по вебсокету.
    double _adc_level; ///< Уровень зарядки аккумуляторов.
    time_t _now;
    SdController* _sdc;
    
    /**
     * \brief Метод выполняет чтение точного сетевого времени.
     */ 
    time_t getInternetTime() {
        configTime(1 * 3600, 60 * 60, "pool.ntp.org", "time.nist.gov");
        #ifdef DEBUG
        Serial.println("Waiting for time");
        #endif
        do  {
            #ifdef DEBUG
            Serial.print(".");
            #endif
            delay(100);
        } while(not time(nullptr));
        #ifdef DEBUG
        Serial.println("");
        #endif
        return time(nullptr);
    }

    /**
     * \brief Метод выполняет считывание значений напряжения питания.
     */ 
    double updateAdcLEvel() {
        int analog_value = 0;
        analog_value = analogRead(ANALOG_PIN);
        if (analog_value == 0) {
            analog_value = 1;
        }
        double level = BATTARY_FULL_VALUE * ((double)(analog_value) / 4096.0);
        #ifdef DEBUG
        Serial.println("Bat > [" + String(analog_value, DEC) + "]: " + String(level, DEC) + " V");
        #endif
        return level;
    }

public:
    explicit Esion(SdController* sdc)
        : _adc_level(0.0) 
        , _sdc(sdc) {
        /// Проверить уровень заряда аккумулятора.
        _adc_level = updateAdcLEvel();
        delay(50);

        if (sdc->_wc.ssid.length()) {
            Blink blk;
            /// Подключение к сети wifi.
            #ifdef DEBUG
            Serial.println("Connecting to WIFI: \"" + sdc->_wc.ssid + "\"|\"" + sdc->_wc.pswd + "\"");
            #endif
            WiFi.begin(sdc->_wc.ssid.c_str(), sdc->_wc.pswd.c_str());
            for (uint16_t i = 0; i < MAX_ATTEMPS_CONNECTIONS && WiFi.status() not_eq WL_CONNECTED; ++i) {
                delay(100);
                #ifdef DEBUG
                Serial.print(".");
                #endif
            } 
            delay(50);
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
        } else {
            #ifdef DEBUG
            Serial.println("WIFI is not configured.");
            #endif
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
        String time = ctime(&_now);
        String data_sjson = String("{\"room_id\":\"" + _sdc->_service_room_id + "\"" +
                    ", \"msg\":{\"time\":\"" + time.substring(0, time.length() - 1) + "\"" +
                    ", \"bat\":" + String(_adc_level, DEC) + 
                    ", \"counts\":[" + counts + "]}}");
        #ifdef DEBUG
        Serial.println("Recv counters: " + counts);
        #endif
        /// Отправить данные на сервер.
        if (_sdc->_service_url.length()) {
            auto dev_id = Nvs::get()->getId();
            _ws_sender = CountersSender::get(_sdc->_service_url, 
                                             String((unsigned long)((dev_id & 0xFFFF0000) >> 16 ), DEC) + 
                                             String((unsigned long)((dev_id & 0x0000FFFF)), DEC), 
                                             data_sjson);
            if (_ws_sender) {
                Blink blk();
                for (uint16_t i = 0; i < MAX_ATTEMPS_CONNECTIONS and not _ws_sender->_is_recv; ++i) {
                    delay(10);
                    _ws_sender->update();
                }
            } else {
                #ifdef DEBUG
                Serial.println("ERROR: Can`t send data. Server is not responce.");
                #endif
            }
        } else {
            #ifdef DEBUG
            Serial.println("ERROR: Can`t send data. Service URL is not set.");
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
    auto sdc = SdController::get();
    if (sdc) {
        {
            Blink blk;
            sdc->getConfig(SEND_SLEEP_TIME, MAX_COUNT_FOR_SEND);
            sdc->saveCounters(__count1, __count2, __count3, __count4, __count5, __count6);
        }
        Esion e(sdc);
        e.sendCounters();
    }
}


uint32_t GetMaxCounter() {
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
    return max;
}


bool CheckMaxCounts(uint32_t max) {
    if (MAX_COUNT_FOR_SEND <= max - __last_max_count) {
        #ifdef DEBUG
        Serial.println("Check for send is TRUE."); 
        #endif
        return true;
    }
    #ifdef DEBUG
    Serial.println("---> max = " + String(max, DEC) + " : last = " + String(__last_max_count, DEC)); 
    #endif
    return false;
}


void WakeupReason() {
    uint32_t wakeup_reason = static_cast<uint32_t>(esp_sleep_get_wakeup_cause());
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wu_bit = static_cast<uint64_t>(esp_sleep_get_ext1_wakeup_status());
            #ifdef DEBUG
            Serial.println("Ext1 status: " + String(reinterpret_cast<uint32_t*>(&wu_bit)[1], DEC));
            #endif
            if (wu_bit & GPIO_SEL_26) {
                #ifdef DEBUG
                Serial.println("Press configure button.");
                #endif
                auto cs = ConfigureServer::get();
                cs->execute();
                cs->resetServer();
            } else if (wu_bit & GPIO_SEL_32) {
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
            __last_max_count = GetMaxCounter();
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
        SdController *sdc; 
        { ///< Blink
            Blink blk;
            delay(3000);
            #ifdef DEBUG
            Serial.println("Start. ==============================");
            #endif
            __is_run = true;
            sdc = SdController::get();
            /// Прочитать файл настроек.
            sdc->getConfig(SEND_SLEEP_TIME, MAX_COUNT_FOR_SEND);
            sdc->getCounts(__count1, __count2, __count3, __count4, __count5, __count6);
        }
        /// Отправить данные на сервер.
        Esion e(sdc);
        e.sendCounters();
    }

    esp_sleep_enable_ext1_wakeup(GPIO_SEL_26 | GPIO_SEL_39 | GPIO_SEL_36 | GPIO_SEL_35 | GPIO_SEL_34 | GPIO_SEL_33 | GPIO_SEL_32, ESP_EXT1_WAKEUP_ANY_HIGH);
    uint32_t max = GetMaxCounter();
    bool check = CheckMaxCounts(max);
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