/**
 * \brief Класс реализующий обслуживание SD карты. 
 */ 

#pragma once

#include <string>
#include <memory> 

#include "utils.hpp"

class SdController;
typedef std::shared_ptr<SdController> PSdController;


class SdController {
    bool _is_inited_sd;
    Blink* _blk;

public:
    WifiConfig _wc;
    String _service_addr;
    int _service_port;
    String _service_rest;
    String _service_room_id;
    int _service_timeout;

    static PSdController& getPtr();

    SdController();
    ~SdController();

    /**
     * \brief Метод выполняет чтение SD карты и получения файла с настройками.
     */ 
    void getConfig(uint32_t &send_sleep_time, uint8_t &max_send_count);

    /**
     * \brief Метод возвращает полный URL сервера обслуживания.
     */ 
    String getUrl();

    /**
     * \brief Метод выполняет чтение SD карты и получения файла со счётчиками.
     */ 
    void getCounts(uint32_t &count1,
                   uint32_t &count2,
                   uint32_t &count3,
                   uint32_t &count4,
                   uint32_t &count5,
                   uint32_t &count6);

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
     *                  },
     *                  "send_timeout":10,
     *                  "max_count":1000
     *              }
     *          }
     */  
    bool parseSettings(fs::File &f, uint32_t &send_sleep_time, uint8_t &max_send_count);

    /**
     * \brief Метод выполняет разбор файла со сцётчиками JSON.
     *        Пример конфигурации:
     *          {
     *              "counters":[0,0,0,0,0,0]
     *          }
     */  
    bool parseCounters(fs::File &f,     
                       uint32_t &count1,
                       uint32_t &count2,
                       uint32_t &count3,
                       uint32_t &count4,
                       uint32_t &count5,
                       uint32_t &count6);

    void saveCounters(uint32_t count1,
                      uint32_t count2,
                      uint32_t count3,
                      uint32_t count4,
                      uint32_t count5,
                      uint32_t count6);
};
