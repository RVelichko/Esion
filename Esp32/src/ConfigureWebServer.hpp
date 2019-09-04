/**
 * \brief Класс реализующий режим конфигурирования. 
 */ 

#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "utils.hpp"


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
    String _srv_ssid;
    String _srv_pswd;
    PAsyncWebServer _server;
    bool _is_complete;
    time_t _start_time;

    static String _dev_id;
    static double _bat_level;

public:
    static PConfigureWebServer& getPtr(const String& srv_ssid = "esion", const String& srv_pswd = "password");

    static bool resetServer();


    static void handleNotFound(AsyncWebServerRequest *request);

    static void handleRoot(AsyncWebServerRequest *request);

    static void handleApply(AsyncWebServerRequest *request);

    static void handleExit(AsyncWebServerRequest *request);

    static void handleAppInfo(AsyncWebServerRequest *request);

    static void handleSettingsInfo(AsyncWebServerRequest *request);

    static void handleCounter1(AsyncWebServerRequest *request);
    static void handleCounter2(AsyncWebServerRequest *request);
    static void handleCounter3(AsyncWebServerRequest *request);
    static void handleCounter4(AsyncWebServerRequest *request);


    static bool parseSettings(const String &js);

    ConfigureWebServer(const String& srv_ssid = "esion", const String& srv_pswd = "esion");
    ~ConfigureWebServer();

    void execute(const String& dev_id, double bat_level);
};