#pragma once

#include <string>
#include <memory>

#include <Arduino.h>
#include <ArduinoNvs.h>

static const double BATTARY_FULL_VALUE = 100.0;

static const int NUM_COUNTERS = 4; 

static const int BLUE_PIN = 2; ///< GPIO голубого светодиода.
static const int RED_PIN = 4;  ///< GPIO красного светодиода.


/**
 * \brief Структура для параметров подключения к внешнему wifi роутеру.
 */
struct WifiConfig {
    String ssid;
    String pswd;
    
    WifiConfig() 
        : ssid({""})
        , pswd({""})
    {}
};


/**
 * \brief Объединение полей структуры бит в байт для значения уровня напряжения питания.
 */
union BatteryValue {
    struct Bits {
        uint8_t _0:1;
        uint8_t _1:1;
    } _bits;
    uint8_t _val:2;
};


/**
 * \brief Класс управления состоянием светодиода.
 */
template <int LED_PIN>
class Blink {
    bool _is_on;

    void init() {
        static bool is_init = false;
        if (not is_init) {
            pinMode(LED_PIN, OUTPUT);
            delay(10);
            is_init = true;
        }
    }
public:
    static Blink* get() {
        static Blink blk;
        return &blk;
    }

    Blink() 
        : _is_on(false) {
        init();
    }

    ~Blink() {
        off();
    }

    /**
     * \brief Зажечь светодиод.
     */
    void on() {
        if (not _is_on) {
            _is_on = true;
            digitalWrite(LED_PIN, HIGH);
        }  
    }

    /**
     * \brief Погасить светодиод.
     */
    void off() {
        if (_is_on) {
            delay(10);
            digitalWrite(LED_PIN, LOW);
            _is_on = false;
        }
    }
};


/**
 * \brief Класс выполняющий светодиодную индикацию предупреждения или ошибки.
 */
class ErrorLights {
    static constexpr int POINT_DT = 60;     
    static constexpr int LINE_DT = 400;     

    template<int LED_PIN>
    void blinkTime(int dt) {
        Blink<LED_PIN>::get()->on();
        delay(dt);
        Blink<LED_PIN>::get()->off();
        delay(POINT_DT);
    }

public:
    static ErrorLights* get() {
        static ErrorLights el;
        return &el;
    }

    ErrorLights() 
    {}

    void error() {
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
        blinkTime<RED_PIN>(POINT_DT);
        blinkTime<BLUE_PIN>(POINT_DT);
    }

    void warning() {
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
        blinkTime<RED_PIN>(LINE_DT);
     }

    ~ErrorLights() 
    {}
};


/**
 * \brief Структура реализует разбор URL на составляющие.
 */ 
struct Url {
    /**
     * \brief Конструктор.
     * \param url  Полная строка url.
     */
    Url(const String& url)
        : port(0) {
        #ifdef DEBUG
        Serial.println("Input URL: \"" + url + "\"");
        #endif
        String s(url);
        int iprotocol = s.indexOf("://");
        if (iprotocol not_eq -1) {
            protocol = s.substring(0, iprotocol);
            s = s.substring(iprotocol + 3);
        }
        int iport = s.indexOf(":");
        int ipath = s.indexOf("/");
        int iquery = s.indexOf("?");
        if (iport not_eq -1) {
            host = s.substring(0, iport);
            String sp = s.substring(iport + 1);
            port = static_cast<uint16_t>(sp.toInt());
            if (ipath not_eq -1) {
                if (iquery not_eq -1) {
                    path = s.substring(ipath, iquery);
                    query = s.substring(iquery);
                } else {
                    path = s.substring(ipath);
                }
            }
        } else if (ipath not_eq -1) {
            host = s.substring(0, ipath);
            if (iquery not_eq -1) {
                path = s.substring(ipath, iquery);
                query = s.substring(iquery);
            } else {
                path = s.substring(ipath);
            }
        } else {
            host = s;
        }
        #ifdef DEBUG
        Serial.println("proto: \"" + protocol + "\"; host: \"" + host + "\"; port: \"" + String(port, DEC) + "\"; path: \"" + path + "\"; query: \"" + query + "\"");
        #endif
    }

    String protocol; ///< Строка протокола.
    String host;     ///< Строка имени хоста.
    uint16_t port;   ///< Строка номера порта.
    String path;     ///< Строка имени страницы на сервере.
    String query;    ///< Параметры запроса URL.
};


/**
 * \brief Структура экранирует двойные кавычки в строке. 
 */ 
struct EscapeQuotes {
private:
    String _out;

public:
    EscapeQuotes(const String &in) {
        int b = 0;
        int pos = 0;
        do {
            pos = in.indexOf('"', pos);
            if (pos not_eq -1) {
                _out += in.substring(b, pos);
                _out += "\\\"";
                b = (++pos); 
            } else {
                _out += in.substring(b);
            }
        } while (pos not_eq -1);
    }

    operator String () {
        return _out;
    }
};


/**
 * \brief Структура описывает подключённый прибор учёта.
 */ 
struct CounterConfig {
    String type;        ///< Тип счётчика воды [ холодна | горячая ].
    String unit;        ///< Юниты для учёта (литр).
    int unit_impl;      ///< Количество импульсов на юнит [ 1 | 10 ].
    String serial;      ///< Серийный номер счётчика.
    String desc;        ///< Описание.
    double start_mcubs; ///< Стартовое количество кубометров.

    CounterConfig() 
        : type("none")
        , unit_impl(1)
        , start_mcubs(0.0)
    {}
};


/**
 * \brief Класс обработки энергонезависимой памяти контроллера.
 */ 
struct Nvs {
    typedef std::unique_ptr<Nvs> PNvs;

    /**
     * \brief Метод возвращает статичный указатель объект обработчик постоянной памяти.
     */ 
    static Nvs* get() {
        static PNvs _nvs;
        if (not _nvs) {
            _nvs.reset(new Nvs());
        }
        return _nvs.get();
    }

    /**
     * \brief Метод выполняет преобразование числового идентификатора устройства в строковое представление.
     */ 
    static String idToStr(uint64_t did) {
        return String((unsigned long)((did & 0xFFFF0000) >> 16 ), DEC) + String((unsigned long)((did & 0x0000FFFF)), DEC);
    } 

    /**
     * \brief Конструктор инициализирует доступ к постоянной памяти.
     */ 
    Nvs() {
        NVS.begin();
    }

    ~Nvs() {
        NVS.close();
    }

    /**
     * \brief Метод выполняет установку флага.
     */ 
    void setFlag(const String &name, uint32_t flag) {
        NVS.setInt(name, flag);
    }

    /**
     * \brief Метод выполняет возврат флага.
     */ 
    uint32_t getFlag(const String &name) {
        return NVS.getInt(name);
    }

    /**
     * \brief Метод записывает текущий уровень напряжения питания контроллера.
     */
    void setPowerValue(float pwr) {
        NVS.setFloat("pwr", pwr);
    }

    /**
     * \brief Метод возвращает уровень напряжения питания контроллера.
     */
    float getPowerValue() {
        return NVS.getFloat("pwr");
    }

    /**
     * \brief Метод выполняет запись типа питания (должен выполняться 1 раз).
     */ 
    void setPwrId(int pwrid) {
        NVS.setInt("pwrid", pwrid);
        #ifdef DEBUG
        Serial.println("$ Set PWR id: " + String(pwrid, DEC));
        #endif
    }

    /**
     * \brief Метод возвращает тип питания.
     */ 
    int getPwrId() {
        int pwrid = NVS.getInt("pwrid");
        #ifdef DEBUG
        Serial.println("$ Set PWR id: " + String(pwrid, DEC));
        #endif
        return pwrid;
    }

    /**
     * \brief Метод выполняет запись идентификатора (должен выполняться 1 раз).
     */ 
    void setId(uint64_t id) {
        #ifdef DEBUG
        Serial.println("$ Set dev_id: " + idToStr(id));
        #endif
        NVS.setInt("id", id);
    }

    /**
     * \brief Метод возвращает идентификатор контроллера.
     */ 
    uint64_t getId() {
        uint64_t uui64 = NVS.getInt("id");    
        #ifdef DEBUG
        Serial.println("$ Get dev_id: " + idToStr(uui64));
        #endif
        return uui64;
    }

    /**
     * \brief Метод записывает идентификатор wifi сети.
     */
    void setSsid(const String& ssid) {
        NVS.setString("ssid", ssid);
        #ifdef DEBUG
        Serial.println("$ Set SSID: " + ssid);
        #endif
    }

    /**
     * \brief Метод возвращает идентификатор wifi сети.
     */
    String getSsid() {
        String ssid = NVS.getString("ssid");
        #ifdef DEBUG
        Serial.println("$ Get SSID: " + ssid);
        #endif
        return ssid;
    }

    /**
     * \brief Метод сохраняет пароль wifi сети.
     */
    void setPswd(const String& pswd) {
        NVS.setString("pswd", pswd);
        #ifdef DEBUG
        Serial.println("$ Set PSWD: " + pswd);
        #endif
    }

    /**
     * \brief Метод возвращает пароль wifi сети.
     */
    String getPswd() {
        String pswd = NVS.getString("pswd");
        #ifdef DEBUG
        Serial.println("$ Get PSWD: " + pswd);
        #endif
        return pswd;
    }

    /**
     * \brief Метод сохраняет почтовый адрес где находится контроллер.
     */
    void setAddress(const String& addr) {
        NVS.setString("addr", addr);
        #ifdef DEBUG
        Serial.println("$ Set Address: " + addr);
        #endif
    }

    /**
     * \brief Метод возвращает почтовый адрес где находится контроллер.
     */
    String getAddress() {
        String addr = NVS.getString("addr");
        #ifdef DEBUG
        Serial.println("$ Get Address: " + addr);
        #endif
        return addr;
    }

    /**
     * \brief Метод сохраняет идентификатор группы устройств.
     */
    void setCollectionName(const String& coln) {
        NVS.setString("coln", coln);
        #ifdef DEBUG
        Serial.println("$ Set Collection name: " + coln);
        #endif
    }

    /**
     * \brief Метод возвращает идентификатор группы устройств.
     */
    String getCollectionName() {
        String coln = NVS.getString("coln");
        #ifdef DEBUG
        Serial.println("$ Get Collection name: " + coln);
        #endif
        return coln;
    }

    /**
     * \brief Метод сохраняет имя владельца контроллера.
     */
    void setUser(const String& user) {
        #ifdef DEBUG
        Serial.println("$ Set user: " + user);
        #endif
        NVS.setString("usr", user);
    }

    /**
     * \brief Метод возвращает имя владельца контроллера.
     */
    String getUser() {
        String user = NVS.getString("usr");
        #ifdef DEBUG
        Serial.println("$ Get user: " + user);
        #endif
        return user;
    }

    /**
     * \brief Метод сохраняет описание.
     */
    void setDescription(const String& desc) {
        #ifdef DEBUG
        Serial.println("$ Set desc: " + desc);
        #endif
        NVS.setString("desc", desc);
    }

    /**
     * \brief Метод возвращает описани.
     */
    String getDescription() {
        String desc = NVS.getString("desc");
        #ifdef DEBUG
        Serial.println("$ Get desc: " + desc);
        #endif
        return desc;
    }

    /**
     * \brief Метод сохраняет адрес сервиса обслуживания.
     */
    void setUrl(const String& url) {
        NVS.setString("url", url);
        #ifdef DEBUG
        Serial.println("$ Set URL: " + url);
        #endif
    }

    /**
     * \brief Метод возвращает адрес сервиса обслуживания.
     */
    String getUrl() {
        String url = NVS.getString("url");
        #ifdef DEBUG
        Serial.println("$ Get URL: " + url);
        #endif
        return url;
    }

    /**
     * \brief Метод сохраняет значения счётчиков.
     */
    void setCounterConfig(uint8_t num, const CounterConfig& count_conf) {
        String cc = "none";
        if (count_conf.type not_eq "none") {
            cc = count_conf.type + "|" + count_conf.serial + "|" + count_conf.unit + "|" + 
                 String(count_conf.unit_impl, DEC) + "|" + count_conf.desc + "|" + 
                 String(count_conf.start_mcubs, DEC);
            NVS.setString("ccfg" + String(num, DEC), cc);
        }
        #ifdef DEBUG
        Serial.println("$ Set counters config: " + cc);
        #endif
    }

    /**
     * \brief Метод возвращает значения счётчиков.
     */
    CounterConfig getCounterConfig(uint8_t num) {
        String cc = NVS.getString("ccfg" + String(num, DEC));
        CounterConfig count_conf;
        if (cc.length() and cc.substring(0, 4) not_eq "none") {
            int i = cc.indexOf("|");
            if (i not_eq -1 and i < cc.length()) {
                count_conf.type = cc.substring(0, i);
                cc = cc.substring(i + 1);
                i = cc.indexOf("|");
                if (i not_eq -1 and i < cc.length()) {
                    count_conf.serial = cc.substring(0, i);
                    cc = cc.substring(i + 1);
                    i = cc.indexOf("|");
                    if (i not_eq -1 and i < cc.length()) {
                        count_conf.unit = cc.substring(0, i);
                        cc = cc.substring(i + 1);
                        i = cc.indexOf("|");
                        if (i not_eq -1 and i < cc.length()) {
                            count_conf.unit_impl = cc.substring(0, i).toInt();
                            cc = cc.substring(i + 1);
                            i = cc.indexOf("|");
                            if (i not_eq -1 and i < cc.length()) {
                                count_conf.desc = cc.substring(0, i);
                                count_conf.start_mcubs = cc.substring(i + 1).toDouble();
                            }
                        }
                    }
                }
            }
        }
        #ifdef DEBUG
        Serial.println("$ Get counters config: " + cc);
        #endif
        return count_conf;
    }

    /**
     * \brief Метод выполняет запись номера кватриты.
     * \param apmt  Номер квартиры.
     */ 
    void setApmt(uint32_t apmt) {
        #ifdef DEBUG
        Serial.println("$ Set apmt: " + String(apmt, DEC));
        #endif
        NVS.setInt("apmt", apmt);
    }

    /**
     * \brief Метод возвращает номер квартиры.
     */ 
    uint32_t getApmt() {
        uint32_t apmt = NVS.getInt("apmt");
        #ifdef DEBUG
        Serial.println("$ Get apmt: " + String(apmt, DEC));
        #endif
        return apmt;
    }

    /**
     * \brief Метод выполняет запись счётчика в отдельную собственную область.
     * \param count_num  Номер счётчика.
     * \param counter  Значение счётчика.
     */ 
    void setCounter(uint8_t count_num, uint32_t counter) {
        NVS.setInt(("ctr" + String(count_num, DEC)).c_str(), counter);
    }

    /**
     * \brief Метод возвращает конфигурацию данного контроллера.
     * \param count_num  Номер счётчика.
     */ 
    uint32_t getCounter(uint8_t count_num) {
        return NVS.getInt(("ctr" + String(count_num, DEC)).c_str());
    }

    /**
     * \brief Метод сохраняет период времени контрольного выхода в эфир.
     */
    void setCtrlTime(uint16_t ctrl_timeout) {
        NVS.setInt("ctrltm", ctrl_timeout);
    }

    /**
     * \brief Метод возвращает период времени контрольного выхода в эфир.
     */
    uint16_t getCtrlTime() {
        return NVS.getInt("ctrltm");
    }

    /**
     * \brief Метод сохраняет максимально число импульсов для выхода в эфир.
     */
    void setMaxImpulses(uint32_t max_impls) {
        NVS.setInt("minmcubs", max_impls);
    }

    /**
     * \brief Метод возвращает максимально число импульсов для выхода в эфир.
     */
    uint32_t getMaxImpulses() {
        return NVS.getInt("minmcubs");
    }

    /**
     * \brief Метод сохраняет таймаут после перехода в состояние выхода в эфир.
     */
    void setSndTimeout(uint32_t snd_timeout) {
        NVS.setInt("sndtm", snd_timeout);
    }

    /**
     * \brief Метод возвращает таймаут после перехода в состояние выхода в эфир.
     */
    uint32_t getSndTimeout() {
        return NVS.getInt("sndtm");
    }

    /**
     * \brief Метод сохраняет время работы сервера конфигурирования.
     */
    void setCfgTime(uint16_t cfg_time) {
        NVS.setInt("cfgtm", cfg_time);
    }

    /**
     * \brief Метод возвращает время работы сервера конфигурирования.
     */
    uint16_t getCfgTime() {
        return NVS.getInt("cfgtm");
    }
};
