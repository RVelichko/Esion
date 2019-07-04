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
typedef std::unique_ptr<GeoRequester> PGeoRequester;


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
 *          Json с описание устройства: {
 *            "dev_id":"<идентификатор устройства, (уникален) >",
 *            "coll":"<адрес расположения устройства — более подробный чем для объекта>",
 *            "user":"<владелец устройства>",
 *            "geo":"<геопозиция (пустая, если не указана) >",
 *            "update_time":"<время последнего подключения к серверу>",
 *            "power_type":"<тип питания устройства>",
 *            "voltage":"<текущее напряжение батареи автономного питания>",
 *            "desc":"<некоторое описание устройтсва>",
 *            "status":"<active | not_active>",
 *            "counters":[
 *              {
 *                "type":"<none | любая не пустая строка (при none остальных полей в этой части json может не быть) >",
 *                "count":"<количество обработанных импульсов>",
 *                "unit":"<единица измерения>",
 *                "unit_type":"<количество единиц измерения на импульс>",
 *                "unit_count":"<количество кубометров>",
 *                "serial":"<серийный монер счётчика>",
 *                "verify_date":"<дата последней поверки счётчика>",
 *                "desc":"<некоторое описание, например какая вода измерятеся>",
 *              },
 *              <таких записей всего 4, первая запись для счётчика устройства №1, вторая для №2 и т.д.  >
 *            ]
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
