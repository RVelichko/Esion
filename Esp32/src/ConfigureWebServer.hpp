/**
 * \brief Класс реализующий режим конфигурирования. 
 */ 

#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "utils.hpp"


static const uint32_t SEND_SLEEP_TIME = 10; ///< секунд
static const uint64_t CONTROL_SEND_SLEEP_TIME = 259200; ///< секунд
static const uint16_t MAX_COUNT_FOR_SEND = 100;
static const time_t CONFIGURE_TIMEOUT = 1800; ///< Количество секунд работы режима конфигурирования.


class ConfigureWebServer;
typedef std::shared_ptr<ConfigureWebServer> PConfigureWebServer;
typedef std::shared_ptr<AsyncWebServer> PAsyncWebServer;


/**
 * \brief Структура хранения настроек контроллера, конфигурируемых по Wifi точке доступа.
 */ 
struct DeviceConfig {
    WifiConfig wc;      ///< Имя wifi сети для подключения при отправке счётчиков.
    String power_type;  ///< Тип питания контроллер.
    String service_url; ///< Пароль wifi.
    String wifi_ssid;   ///< Имя wifi сети.
    String wifi_pswd;   ///< Пароль для подключения к wifi точке доступа.
    String user_name;   ///< Данные пользователя контроллера.
    String address;     ///< Адрес расположения контроллера.
    String user_desc;   ///< Адрес и описание места, где установлен контроллер.
    std::array<CounterConfig, NUM_COUNTERS> counts;

    DeviceConfig() 
        : address("none")
    {}
};


class ConfigureWebServer {
    String _srv_ssid;         ///< Идентификатор конфигурационного wifi сервера.
    String _srv_pswd;         ///< Пароль конфигурационного wifi сервера.
    PAsyncWebServer _server;  ///< Объект асинхроннойго wifi сервера.
    bool _is_complete;        ///< Флаг завершения работы сервера конфигурирования.
    time_t _start_time;       ///< Время запуска сервера конфигурирования.

    static String _dev_id;    ///< Идентификатор данного контроллера.
    static double _bat_level; ///< Уровень заряда аккумулятора.

public:
    static PConfigureWebServer& getPtr(const String& srv_ssid = "esion", const String& srv_pswd = "password");

    /**
     * \brief Удаление объекта сервера.
     */
    static bool resetServer();

    /**
     * \brief Отображение страницы для не зарегистрированного запроса.
     */
    static void handleNotFound(AsyncWebServerRequest *request);

    /**
     * \brief Отображение index.html.
     */
    static void handleRoot(AsyncWebServerRequest *request);

    /**
     * \brief Выполнение действий по запросу на сохранение настроек от пользователя.
     */
    static void handleApply(AsyncWebServerRequest *request);

    /**
     * \brief Выполнение действий по запросу на завершение работы с сервером.
     */
    static void handleExit(AsyncWebServerRequest *request);

    /**
     * \brief Отправка клиенту информации о питании и индентификатор устройства.
     */
    static void handleAppInfo(AsyncWebServerRequest *request);

    /**
     * \brief Отправка клиенту общих настроек по счётчикам.
     */
    static void handleSettingsInfo(AsyncWebServerRequest *request);

    /**
     * \brief Отправка клиенту состояний счётиков.
     */
    static void handleCounter1(AsyncWebServerRequest *request);
    static void handleCounter2(AsyncWebServerRequest *request);
    static void handleCounter3(AsyncWebServerRequest *request);
    static void handleCounter4(AsyncWebServerRequest *request);

    /**
     * \brief Метод разбора полученного от клиента json с готовыми настройками.
     */
    static bool parseSettings(const String &js);

    ConfigureWebServer(const String& srv_ssid = "esion", const String& srv_pswd = "esion");
    ~ConfigureWebServer();

    /**
     * \brief Цикл выполнения операций сервера.
     */
    void execute(const String& dev_id, double bat_level);
};
