/**
 * \brief  Простой websocket обработчик подключения устройства к комнате 1x1 связи контроллера с веб страницей состояний счётчиков.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   17.09.2018
 */

#pragma once

#include "GeoRequester.hpp"
#include "DbFacade.hpp"
#include "BaseWorker.hpp"

namespace server {

typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef std::shared_ptr<GeoRequester> PGeoRequester;


/**
 * Клас, обрабатывающий подключения от устройства.
 * При обновлении, подключённым операторам отправляется новые данные устройства.
 *          Responce: {
 *            "resp": {
 *              "name":"get_devs",
 *              "status":"< ok | err >",
 *              "data":"[{<json устройства >}, … ]", /// при err — этого поля не будет.
 *              "desc":"<описание, при ок этого поля не будет>"
 *            }
 *          }
 *
  *         Json с описание устройства: {
 *            "dev_id" : "1561650574",
 *            "apmt" : 27,
 *            "coll" : "Санкт-Петербург Вязовая д.10",
 *            "coll_id" : "Debug Debug",
 *            "power_type" : "LiOn 3.8V",
 *            "user" : "Vishen Ruevit",
 *            "voltage" : 3.52353328947142,
 *            "desc" : "\"ГСК \"\"Автомобилист\"\" Договор N% 32/16 от 01/11/16\": N 20",
 *            "counters" : [
 *                {
 *                    "count" : 1653,
 *                    "cubic_meter" : [ // Заполняет сервер.
 *                        {
 *                            "cm" : 0.049,
 *                            "t" : 1561619939
 *                        }, ...
 *                    ],
 *                    "serial" : 303,
 *                    "type" : "Debug type",
 *                    "unit" : "Литр",
 *                    "unit_count" : 1,
 *                    "verify_date" : 1729294805, // Не реализовано.
 *                    "unit_type" : "Импульс", // Определяет сервер.
 *                    "desc" : "Отладочный канал N 2",
 *                }, ...
 *            ],
 *            "voltage_perc" : 92,             // Определяте сервер.
 *            "geo" : [ 59.96716, 30.273611 ], // Определяет сервер.
 *            "start_time" : 1561619939,       // Определяет сервер.
 *            "update_time" : 1564411326,      // Определяет сервер.
 *            "status" : "active",             // Определяет сервер.
 *          }
 */
class DevicePeerWorker : public BaseWorker {
    virtual bool parseMessage(const std::string &msg, const ConcreteFn &func);
    virtual PConnectionValue firstMessage(size_t connection_id, const std::string &msg);
    virtual bool lastMessage(const ConnectionValuesIter &iter, const std::string &msg);
    virtual void sendClose(size_t connection_id);

    PDbFacade _db; ///< Объект доступа к БД.
    PGeoRequester _geo_req;

public:
    DevicePeerWorker(std::mutex &mutex, const PDbFacade& db, const std::string& ymap_api_key);
    virtual ~DevicePeerWorker();
};
} /// server
