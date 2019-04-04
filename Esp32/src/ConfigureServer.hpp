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

class ConfigureServer {
    String _srv_ssid;
    String _srv_pswd;
    ESP32WebServer _server;
    bool _is_complete;
    time_t _start_time;

    WifiConfig _wc;
    String _service_url;

    static String _dev_id;
    static double _bat_level;

public:
    static String getPage(const String& device_id, const String& bat_level);

    static PConfigureServer& getPtr(const String& srv_ssid = "esion", const String& srv_pswd = "12345678");

    static bool resetServer();

    static void handleRoot();

    static void handleSave();

    static void handleNotFound();

    ConfigureServer(const String& srv_ssid = "esion", const String& srv_pswd = "12345678");
    ~ConfigureServer();

    void execute(const String& dev_id, double bat_level);

    bool parseSettings(const String &js);
};