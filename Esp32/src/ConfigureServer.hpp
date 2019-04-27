/**
 * \brief Класс реализующий режим конфигурирования. 
 */ 

#pragma once

#include <memory>
#include <string>

#include <Arduino.h>
#include <ESP32WebServer.h>
#include <WiFi.h>

#include "utils.hpp"


class ConfigureServer;
typedef std::shared_ptr<ConfigureServer> PConfigureServer;


/**
 * \brief Структура хранения настроек контроллера, конфигурируемых по Wifi точке доступа.
 */ 
struct DeviceConfig {
    WifiConfig wc;      ///< Имя wifi сети для подключения при отправке счётчиков.
    String service_url; ///< Пароль wifi.
    String coll_name;   ///< Имя коллекции, в котороую входит данный контроллер.
    String user_name;   ///< Данные пользователя контроллера.
    String user_desc;   ///< Адрес и описание места, где установлен контроллер.
    std::array<CounterConfig, NUM_COUNTERS> counts;

    DeviceConfig() 
        : coll_name("none")
    {}
};

class ConfigureServer {
    String _srv_ssid;
    String _srv_pswd;
    ESP32WebServer _server;
    bool _is_complete;
    time_t _start_time;

    DeviceConfig _dev_conf;

    static String _dev_id;
    static double _bat_level;

public:
    static String getPage(const String& device_id, const String& bat_level);

    static PConfigureServer& getPtr(const String& srv_ssid = "esion", const String& srv_pswd = "esionpassword");

    static bool resetServer();

    static void handleRoot();

    static void handleSave();

    static void handleNotFound();

    static bool parseSettings(const String &js, DeviceConfig& conf);

    ConfigureServer(const String& srv_ssid = "esion", const String& srv_pswd = "esionpassword");
    ~ConfigureServer();

    void execute(const String& dev_id, double bat_level);
};