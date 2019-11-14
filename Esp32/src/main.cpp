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

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include <Arduino.h>
#include <Esp.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/rtc_io.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPI.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "utils.hpp"
#include "ConfigureWebServer.hpp"
#include "SdController.hpp"
#include "CountersSender.hpp"
#include "Programmer.hpp"

static const int ENABLE_MEASURE_PIN = 25;       ///< Пин включения измерения напряжения.
static const int MEASURE_PIN = 27; //ADC2_CHANNEL_7;  ///< Пин измерения напряжения.

static const uint32_t DEFAULT_SERIAL_SPEED = 115200;

static const uint16_t MAX_ATTEMPS_CONNECTIONS = 100;
static const uint32_t DEFAULT_LOCAL_PORT = 20000;
static const char DEFAULT_SERVICE_URL[] = "localhost";
static const uint16_t DEFAULT_SERVICE_PORT = 8080;
static const char DEFAULT_SERVICE_POINT[] = "/device";

static const uint32_t RESET_TIMEOUT = 20;
static const uint32_t SEND_SLEEP_TIME = 10; ///< секунд
static const uint32_t CONTROL_SEND_SLEEP_TIME = 259200; ///< секунд


RTC_DATA_ATTR bool __is_run = false;
RTC_DATA_ATTR bool __is_send_timeout = false;
RTC_DATA_ATTR bool __is_control_send_timeout = true;
RTC_DATA_ATTR time_t __comtrol_send_timneout = 0;
RTC_DATA_ATTR uint8_t __pin_states = 0;

RTC_DATA_ATTR uint32_t __count1 = 0;
RTC_DATA_ATTR uint32_t __count2 = 0;
RTC_DATA_ATTR uint32_t __count3 = 0;
RTC_DATA_ATTR uint32_t __count4 = 0;
RTC_DATA_ATTR uint32_t __last_max_count = 0;

RTC_DATA_ATTR uint8_t MAX_COUNT_FOR_SEND = 100;

RTC_DATA_ATTR time_t __device_id = 0;
RTC_DATA_ATTR double __adc_level = 0; ///< Уровень зарядки аккумуляторов.
RTC_DATA_ATTR int __power_id = 0;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
hw_timer_t * __timer = NULL;


void IRAM_ATTR OnTimer() {
    ESP.restart();
}


void InitTimerInterrupt(int mlss = 1000) {
    #ifdef DEBUG
    Serial.println("INIT timer interrupt to " + String(mlss, DEC) + ".");
    #endif
    __timer = timerBegin(0, 80, true);
    timerAttachInterrupt(__timer, &OnTimer, true);
    int mkss = mlss * 1000;
    timerAlarmWrite(__timer, mkss, true);
    timerAlarmEnable(__timer);
}


void ResetTimerInterrupt() {
    timerAlarmDisable(__timer);
    #ifdef DEBUG
    Serial.println("RESET timer interrupt.");
    #endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ResetCounters();
void SaveCounters();
void RestoreCounters();
void SendTimeout();
bool CheckMaxCounts(uint32_t);


class Esion;
typedef std::shared_ptr<Esion> PEsion;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * \brief Класс реализует основные функции WIFI контроллера.
 */ 
class Esion {
    typedef std::unique_ptr<CountersSender> PCountersSender;

private:
    int _service_timeout; ///< Время перезапуска отправки данных в минутах.
    time_t _now;
    PCountersSender _cs;
    DeviceConfig _dev_conf;
    
public:
    /**
     * \brief Метод выполняет чтение точного сетевого времени.
     */ 
    static time_t getInternetTime() {
        WiFiUDP ntpUDP;
        NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
        timeClient.begin();
        timeClient.update();
        time_t t = static_cast<time_t>(timeClient.getEpochTime());
        #ifdef DEBUG
        Serial.println("Time: " + String(t, DEC));
        #endif
        return t;
    }

    /**
     * \brief Метод выполняет считывание значений напряжения питания.
     */ 
    static double updateAdcLEvel() {
        pinMode(ENABLE_MEASURE_PIN, OUTPUT);
        digitalWrite(ENABLE_MEASURE_PIN, HIGH);
        delay(100);
        int analog_value = analogRead(MEASURE_PIN);
        delay(100);
        double vbat = 0.0f;
        if (analog_value not_eq 0) {
            vbat = BATTARY_FULL_VALUE * double(analog_value) / 4096.0f;
            if (vbat < (BATTARY_FULL_VALUE * 0.7)) {
                ErrorLights::get()->warning();
            }
        }
        #ifdef DEBUG
        Serial.println("POWER: [" + String(analog_value, DEC) + "]: " + String(vbat, DEC) + " V");
        #endif
        digitalWrite(ENABLE_MEASURE_PIN, LOW);
        return vbat;
    }

public:
    static PEsion& getPtr() {
        static PEsion e;
        if (not e) {
            e.reset(new Esion());
        }
        return e;
    }

    Esion() {
        /// Проверить уровень заряда аккумулятора.
        //__adc_level = updateAdcLEvel();
        auto nvs = Nvs::get();
        /// Получить тип питания.
        //__power_id = nvs->getPwrId();
        /// Выполнить проверку уникального идентификатора.
        __device_id = nvs->getId();
        #ifdef DEBUG
        Serial.println("Start ESION [" + String(__device_id, DEC) + "]");
        #endif
        if (__device_id == 0) {
            __device_id = DEVICE_ID;
            Nvs::get()->setId(__device_id);
            Nvs::get()->setUrl(SERVICE_URL);
            #ifdef DEBUG
            Serial.println("INIT UNIQUE DEVICE ID: [" + String(__device_id, DEC) + "]");
            #endif
        } else {
            #ifdef DEBUG
            Serial.println("UNIQUE DEVICE ID: [" + String(__device_id, DEC) + "]");
            #endif
        }
        _dev_conf.wc.ssid = nvs->getSsid();
        _dev_conf.wc.pswd = nvs->getPswd();
        if (_dev_conf.wc.ssid.length()) {
            Blink<BLUE_PIN>::get()->on();
            /// Подключение к сети wifi.
            #ifdef DEBUG
            Serial.println("Connecting to WIFI: \"" + _dev_conf.wc.ssid + "\"|\"" + _dev_conf.wc.pswd + "\"");
            #endif
            Blink<BLUE_PIN>::get()->off();
            WiFi.begin(_dev_conf.wc.ssid.c_str(), _dev_conf.wc.pswd.c_str());
            for (uint16_t i = 0; ((i < MAX_ATTEMPS_CONNECTIONS) and (WiFi.status() not_eq WL_CONNECTED)); ++i) {
                Blink<BLUE_PIN>::get()->on();
                delay(50);
                #ifdef DEBUG
                Serial.print(".");
                #endif
                Blink<BLUE_PIN>::get()->off();
                delay(50);
            } 
            Blink<BLUE_PIN>::get()->on();
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
                ErrorLights::get()->error();
                #ifdef DEBUG
                Serial.println("ERROR: Can`t connet to WIFI.");
                #endif
            }
            delay(50);
            Blink<BLUE_PIN>::get()->off();
        } else {
            ErrorLights::get()->error();
            #ifdef DEBUG
            Serial.println("ERROR: WIFI is not configured.");
            #endif
        }
    }

    ~Esion() {
        WiFi.disconnect(); // обрываем WIFI соединения
        WiFi.softAPdisconnect(); // отключаем отчку доступа(если она была
        WiFi.mode(WIFI_OFF); // отключаем WIFI
    }
    
    void sendValues() {
        auto nvs = Nvs::get();
        if (__device_id and nvs) {
            auto service_url = nvs->getUrl();
            if (service_url.length()) {
                String time = ctime(&_now);
                String id = String(__device_id, DEC);
                std::array<uint32_t, 4> icounts({__count1, __count2, __count3, __count4});
                auto apmt = nvs->getApmt();
                auto user_name = nvs->getUser();
                auto desc = nvs->getDescription();
                auto addr = nvs->getAddress();
                auto coll_id = nvs->getCollectionName();
                __power_id = nvs->getPwrId();
                String powt = "none";
                if (__power_id == 1) {
                    powt = "LiOn [3.8V]";
                } else if (__power_id == 2) {
                    powt = "4AA [6V]";
                } 
                String data_sjson =
                    "{\"dev_id\":\"" + id + "\"," \
                    "\"apmt\":" + String(apmt, DEC) + "," \
                    "\"coll\":\"" + addr + "\"," \
                    "\"coll_id\":\"" + coll_id + "\"," \
                    "\"power_type\":\"" + powt + "\"," \
                    "\"voltage\":\"" + String(__adc_level, DEC) + "\"," \
                    "\"user\":\"" + user_name + "\"," \
                    "\"desc\":\"" + desc + "\"," \
                    "\"counters\":[";
                for (uint8_t i = 0; i < icounts.size(); ++i) {
                    auto count_cfg = nvs->getCounterConfig(i);
                    data_sjson +=
                        "{\"type\":\"" + count_cfg.type + "\"";
                    if (count_cfg.type not_eq "none") {
                        data_sjson +=
                            ",\"count\":" + String(icounts[i], DEC) + "," \
                            "\"unit\":\"" + count_cfg.unit + "\"," \
                            "\"unit_count\":" + count_cfg.unit_impl + "," \
                            "\"serial\":\"" + count_cfg.serial + "\"," \
                            "\"start_mcubs\":" + String(count_cfg.start_mcubs, DEC) + "," \
                            "\"desc\":\"" + count_cfg.desc + "\"";
                    }
                    if (i < icounts.size() - 1) {
                        data_sjson += "},";
                    } else {
                        data_sjson += "}";
                    }
                }
                data_sjson += "]}";
                #ifdef DEBUG
                Serial.println("Send: " + data_sjson);
                #endif
                /// Отправить данные на сервер.
                InitTimerInterrupt(RESET_TIMEOUT * 1000);
                Url U(service_url);
                String path = U.path;
                if (not path.length()) {
                    path = DEVICE_URL_PATH;
                }
                uint16_t port = U.port;
                if (not port) {
                    port = DEFAULT_SERVICE_PORT;
                }
                _cs.reset(new CountersSender(U.host, port, path, id, data_sjson));
                _cs->execute();
                ResetTimerInterrupt();
            } else {
                ErrorLights::get()->error();
                #ifdef DEBUG
                Serial.println("ERROR: Can`t send data. Service URL is not set.");
                #endif
            }
        } else {
            ErrorLights::get()->warning();
            #ifdef DEBUG
            Serial.println("WARNING: Device ID is not set.");
            #endif
        }
    }
}; 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ResetCounters() {
    auto nvs = Nvs::get();
    if (nvs) {
        nvs->setCounter(0, 0);
        nvs->setCounter(1, 0);
        nvs->setCounter(2, 0);
        nvs->setCounter(3, 0);
        nvs->setCounter(4, 0);
    }
}


void SaveCounters() {
    auto nvs = Nvs::get();
    if (nvs) {
        if (__count1) {
            nvs->setCounter(0, __count1);
        }
        if (__count2) {
            nvs->setCounter(1, __count2);
        }
        if (__count3) {
            nvs->setCounter(2, __count3);
        }
        if (__count4) {
            nvs->setCounter(3, __count4);
        }
        if (__last_max_count) {
            nvs->setCounter(4, __last_max_count);
        }
        #ifdef DEBUG
        Serial.println("SAVE ----------------------------"); 
        Serial.println("Power: " + String(__adc_level, DEC)); 
        Serial.println("Count 1: " + String(__count1, DEC)); 
        Serial.println("Count 2: " + String(__count2, DEC)); 
        Serial.println("Count 3: " + String(__count3, DEC)); 
        Serial.println("Count 4: " + String(__count4, DEC)); 
        Serial.println("Max: " + String(__last_max_count, DEC)); 
        Serial.println("---------------------------------"); 
        #endif

    }
}


void RestoreCounters() {
    auto nvs = Nvs::get();
    if (nvs) {
        __count1 = nvs->getCounter(0);
        __count2 = nvs->getCounter(1);
        __count3 = nvs->getCounter(2);
        __count4 = nvs->getCounter(3);
        __last_max_count = nvs->getCounter(4);
        __adc_level = Esion::updateAdcLEvel();
        #ifdef DEBUG
        Serial.println("RESTORE -------------------------"); 
        Serial.println("Power: " + String(__adc_level, DEC)); 
        Serial.println("Count 1: " + String(__count1, DEC)); 
        Serial.println("Count 2: " + String(__count2, DEC)); 
        Serial.println("Count 3: " + String(__count3, DEC)); 
        Serial.println("Count 4: " + String(__count4, DEC)); 
        Serial.println("Max: " + String(__last_max_count, DEC)); 
        Serial.println("---------------------------------"); 
        #endif
    }
}


void SendTimeout() {
    __is_control_send_timeout = true;
    #ifdef DEBUG
    Serial.println("SEND Counters ==================="); 
    #endif
    SaveCounters();
    Esion::getPtr()->sendValues();
}


uint32_t GetMaxCounter() {
    uint32_t max = __count1 + __count2 + __count3 + __count4;
    return max;
}


bool CheckMaxCounts(uint32_t max) {
    if (MAX_COUNT_FOR_SEND <= max - __last_max_count) {
        #ifdef DEBUG
        Serial.println("Check for send is TRUE."); 
        #endif
        Blink<BLUE_PIN>::get()->on();
        delay(10);
        Blink<BLUE_PIN>::get()->off();
        return true;
    }
    #ifdef DEBUG
    Serial.println("---> [" + String(MAX_COUNT_FOR_SEND, DEC) + "] max = " + String(max, DEC) + " : last = " + String(__last_max_count, DEC)); 
    #endif
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void WakeupReason() {
    uint32_t wakeup_reason = static_cast<uint32_t>(esp_sleep_get_wakeup_cause());
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wu_bit = static_cast<uint64_t>(esp_sleep_get_ext1_wakeup_status());
            #ifdef DEBUG
            Serial.println("Ext1 status: " + String(reinterpret_cast<uint32_t*>(&wu_bit)[1], DEC));
            #endif
            if (wu_bit & GPIO_SEL_26) {
                /// Запустить веб сервер установки параметров контроллера.
                #ifdef DEBUG
                Serial.println("Press configure button.");
                #endif
                auto cs = ConfigureWebServer::getPtr();
                if (cs) {
                    /// Проверить уровень заряда аккумулятора.
                    //__adc_level = Esion::updateAdcLEvel();
                    __device_id = Nvs::get()->getId();
                    if (__device_id == 0) {
                        __device_id = DEVICE_ID;
                        Nvs::get()->setId(__device_id);
                        Nvs::get()->setUrl(SERVICE_URL);
                        #ifdef DEBUG
                        Serial.println("INIT UNIQUE DEVICE ID: [" + String(__device_id, DEC) + "]");
                        #endif
                    }
                    __power_id = Nvs::get()->getPwrId();
                    SaveCounters();
                    cs->execute(String(__device_id, DEC), __adc_level);
                    cs.reset();
                    auto e = Esion::getPtr();
                    e->sendValues();
                    e.reset();
                }
            } else if (wu_bit & GPIO_SEL_34) {
                ++__count1;
                #ifdef DEBUG
                Serial.println("C1---------> PIN 34: " + String(__count1, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_35) {
                ++__count2;
                #ifdef DEBUG
                Serial.println("C2---------> PIN 35: " + String(__count2, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_36) {
                ++__count3;
                #ifdef DEBUG
                Serial.println("C3---------> PIN 36: " + String(__count3, DEC));
                #endif
            } else if (wu_bit & GPIO_SEL_39) {
                ++__count4;
                #ifdef DEBUG
                Serial.println("C4---------> PIN 39: " + String(__count4, DEC));
                #endif
            }
            /// Моргнуть светодиодом на очередной импульс.
            if ((wu_bit & GPIO_SEL_34) or 
                (wu_bit & GPIO_SEL_35) or 
                (wu_bit & GPIO_SEL_36) or 
                (wu_bit & GPIO_SEL_39)) {
                Blink<RED_PIN>::get()->on();
                delay(5);
                Blink<RED_PIN>::get()->off();
            }
        } break;
        case ESP_SLEEP_WAKEUP_TIMER: 
            __last_max_count = GetMaxCounter(); ///< Запомнить последнее максимальное значение счётчиков.
            SendTimeout(); ///< Отправить данные на сервер.
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
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    delay(100);
    #ifdef DEBUG
    Serial.begin(DEFAULT_SERIAL_SPEED);
    #endif

    if (__is_run) {
        WakeupReason();
    } else {
        /// Выполнить прошивку входного контроллера при первом запуске прошивки.
        Nvs *nvs = Nvs::get();
        uint32_t is_tiny_flashed = nvs->getFlag("flashed");
        if (not is_tiny_flashed) {
            #ifdef DEBUG
            Serial.println("INIT Programmer.");
            #endif
            Programmer prg;
            Serial.println("Start load.");
            prg.load(CODE, sizeof(CODE));
            delay(300);
            Serial.println("Flash is compette.");
            nvs->setFlag("flashed", ATTINY84_ID);
            delay(100);
        }
        Blink<BLUE_PIN>::get()->on();
        delay(100);
        #ifdef DEBUG
        Serial.println("Start. ============================== [" + String(DEVICE_ID, DEC) + "]");
        #endif
        __is_run = true;
        Blink<BLUE_PIN>::get()->off();
        //ResetCounters();
        /// Прочитать счётчики после перезапуски контроллера.
        RestoreCounters();
    }
    /// Установить обработчики прерываний на обработку входных импульсов.
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_26 | GPIO_SEL_39 | GPIO_SEL_36 | GPIO_SEL_35 | GPIO_SEL_34, ESP_EXT1_WAKEUP_ANY_HIGH);
    if (CheckMaxCounts(GetMaxCounter())) {
        /// Установить обработчик прерывания по таймеру.
        esp_sleep_enable_timer_wakeup(SEND_SLEEP_TIME * 1000000);
    } else if (__is_control_send_timeout) {
        /// Запустить обязательную отправку через 3 дня.
        __is_control_send_timeout = false;
        esp_sleep_enable_timer_wakeup(CONTROL_SEND_SLEEP_TIME * 1000000);
    }
    #ifdef DEBUG
    Serial.println("To sleep. -----------------------------");
    Serial.flush();
    #endif
    esp_deep_sleep_start();
}


void loop() {
}