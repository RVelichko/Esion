/**
 * \brief  Команды, обрабатываемые операторской частью сервера.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once

#include <functional>

#include "JsonCommand.hpp"
#include "DbFacade.hpp"

namespace server {

typedef utils::Json Json;
typedef utils::JsonCommand JsonCommand;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef std::function<void(const std::string&)> SendFn;
typedef std::function<bool(size_t, const Json&, std::mutex&, const PDbFacade&, const SendFn&)> ExecuteFn;
typedef std::map<std::string, ExecuteFn> CommandsExecuters;


class BaseCommand : public JsonCommand {
protected:
    size_t _token; ///< Уникальный токен авторизации, доступен всем командам, управляется командой авторизации.
    std::mutex& _mutex;
    PDbFacade _db;
    SendFn _snd_fn;

    /*!
     * \brief Метод создаёт ответ с описанием ошибки.
     * \param desc  Описание ошибки.
     * \return  Возвращает сформированный JSON.
     */
    Json getErrorResponce(const std::string& desc);

    /*!
     * \brief Метод удвляет _id из переданного JSON.
     * \param js  Корректируемый JSON.
     */
    void eraseMongoId(Json& js);

public:
    static bool executeByName(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);

    BaseCommand(size_t token, const std::string& name, const Json& js,
                std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
};


/*!
 * \brief  Авторизация:
 *         Rquest: {
 *           "cmd": {
 *             "name":"auth",
 *             "data": {
 *               "user":"<user name>", ///< debug для теста.
 *               "pswd":"<password>"   ///< debug для теста.
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"auth",
 *             "status":"< ok | err >",
 *             "token":"< идентификатор пользователя сервиса >",
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class AuthorizeCommand : public BaseCommand {
public:
    AuthorizeCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~AuthorizeCommand();

    /**
     * Перегруженный метод выполнения команды.
     * \return  TRUE - в случае успешного выполнения, FALSE - в случае ошибки.
     */
    virtual Json execute();
};


/*! \brief  Запрос списка устройств:
 *          Rquest: {
 *            "cmd": {
 *              "name":"get_devs",
 *              "data": {
 *                "token":"< идентификатор пользователя сервиса >",
 *                "skip":"<количество пропускаемых записей в списке найденных устройств>",
 *                "num":"<количество возвращаемых устройств>",
 *                "filter":"<любая строка, которой могут соответствовать строки в БД>"
 *              }
 *            }
 *          }
 *
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
class GetDevicesListCommand : public BaseCommand {
public:
    GetDevicesListCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~GetDevicesListCommand();
    virtual Json execute();
};


/*!
 * \brief  Отправка геопозиции для устройства:
 *         Rquest: {
 *           "cmd": {
 *             "name":"set_dev_geo",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "dev_id":"<идентификатор устройства, (уникален) >",
 *               "geo":"<строка геолокации>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"set_dev_geo",
 *             "status":"< ok | err >",
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class DeviceGeopositionCommand : public BaseCommand {
public:
    DeviceGeopositionCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~DeviceGeopositionCommand();
    virtual Json execute();
};


/*!
 * \brief  Активация / деактивация устройства:
 *         Rquest: {
 *           "cmd": {
 *             "name":"set_dev_status",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "dev_id":"<идентификатор устройства, (уникален) >",
 *               "status":"<activate | deactivate>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"set_dev_status",
 *             "status":"< ok | err >",
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class ActivateDeviceCommands : public BaseCommand {
public:
    ActivateDeviceCommands(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~ActivateDeviceCommands();
    virtual Json execute();
};


/*!
 * \brief  Запрос списка событий:
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_events",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "num":"<количество возвращаемых событий>",
 *               "dev_id":"<идентификатор устройства, (уникален) >",
 *               "filter":"<любая строка, которой могут соответствовать строки в БД>",
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"get_events",
 *             "status":"< ok | err >",
 *             "data":"[{<json события >}, … ]", /// при err — этого поля не будет.
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 *
 *         Json описывающий событие: {
 *           "ev_id":"<уникальный идентийикатор>",
 *           "dev_id":"<уникальный идентийикатор устройсва на которов произошло событие>",
 *           "coll":"<адрес где произошло событие>",
 *           "user":"<владелец устройства, для которого произошло событие>",
 *           "geo":"<геопозиция (пустая, если не указана) >",
 *           "time":"<время события>",
 *           "priority":"<тип питания устройства>",
 *           "status":"<active | not_active>"
 *           "desc":"<описание события>"
 *         }
 */
class GetEventsListCommand : public BaseCommand {
public:
    GetEventsListCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~GetEventsListCommand();
    virtual Json execute();
};


/*!
 * \brief  Отправка геопозиции для события:
 *         Rquest: {
 *           "cmd": {
 *             "name":"set_event_geo",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "ev_id":"<идентификатор события, (уникален) >",
 *               "geo":"<строка геолокации>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"set_event_geo",
 *             "status":"< ok | err >",
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class EventGeopositionCommand : public BaseCommand {
public:
    EventGeopositionCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~EventGeopositionCommand();
    virtual Json execute();
};


/*!
 * \brief  Команда на формирование отчёта.
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_report",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "coll":"< адрес для которого необходимо сформировать отчёт >",
 *             }
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_report",
 *             "status":"< ok | err >",
 *             "report_url":"<url с файлом отчёта>"
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class CreateReportCommand : public BaseCommand {
public:
    CreateReportCommand(size_t token, const Json& js, std::mutex& mutex, const PDbFacade& db, const SendFn& snd_fn);
    virtual ~CreateReportCommand();
    virtual Json execute();
};
} /// server
