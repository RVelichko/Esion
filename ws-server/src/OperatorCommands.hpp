/**
 * \brief  Команды, обрабатываемые операторской частью сервера.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once

#include <functional>
#include <memory>

#include "JsonCommand.hpp"
#include "IndexDbFacade.hpp"
#include "DbFacade.hpp"

namespace server {

typedef utils::Json Json;
typedef utils::JsonCommand JsonCommand;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef sindex::IndexDbFacade IndexDbFacade;
typedef std::shared_ptr<IndexDbFacade> PIndexDbFacade;
typedef std::function<void(const std::string&)> SendFn;
typedef std::function<bool(const std::string&, const Json&, std::mutex&, const SendFn&)> ExecuteFn;
typedef std::map<std::string, ExecuteFn> CommandsExecuters;



static const size_t MAX_NUMBER_DEVICES_FOR_REPORT = 10000;

class BaseCommand : public JsonCommand {
protected:
    std::string _token; ///< Уникальный токен авторизации, доступен всем командам, управляется командой авторизации.
    std::mutex& _mutex;
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

    /*!
     * \brief Метод заполняет тег data в ответе.
     * \param js  Передаваемый JSON.
     */
    Json fillResponceData(const Json& js);

public:
    static PDbFacade _db;
    static PIndexDbFacade _xdb;
    static std::string _reports_path;

    static bool executeByName(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);

    BaseCommand(const std::string& token, const std::string& name, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
};


/*!
 * \brief  Авторизация:
 *         Rquest 1: {
 *           "cmd": {
 *             "name":"auth",
 *             "data": {
 *               "login":"<user name>", ///< Debug для теста.
 *               "pswd":"<password>"   ///< Debug для теста.
 *             }
 *           }
 *         }
 *
 *         Rquest 2: {
 *           "cmd": {
 *             "name":"auth",
 *             "data": {
 *               "token": "<Токен пользователя, полученный при авторизации.>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"auth",
 *             "status":"< ok | err >",
 *             "token":"< идентификатор пользователя сервиса >",
 *             "user_info": {
 *                  "person" : < Имя Фамилия и т.д. >
 *                  "desc" : < Информация о пользователе сервиса >
 *             },
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class AuthorizeCommand : public BaseCommand {
public:
    AuthorizeCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~AuthorizeCommand();

    /**
     * Перегруженный метод выполнения команды.
     * \return  TRUE - в случае успешного выполнения, FALSE - в случае ошибки.
     */
    virtual Json execute();
};


/*!
 * \brief  Выход из сервиса:
 *         Rquest 1: {
 *           "cmd": {
 *             "name":"logout"
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"auth",
 *             "status":"< ok | err >",
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class LogoutCommand : public BaseCommand {
public:
    LogoutCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~LogoutCommand();
    virtual Json execute();
};


/*! \brief  Запрос списка устройств:
 *          Rquest 1: {
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
 *          Rquest 2: {
 *            "cmd": {
 *              "name":"get_devs",
 *              "data": {
 *                "token":"< идентификатор пользователя сервиса >",
 *                "skip":"<количество пропускаемых записей в списке найденных устройств>",
 *                "num":"<количество возвращаемых устройств>",
 *                "geo":[ <longitude>, <latitude> ],
 *                "radius": <радиус поиска>
 *              }
 *            }
 *          }
 *
 *          Rquest 3: {
 *            "cmd": {
 *              "name":"get_devs",
 *              "data": {
 *                "token":"< идентификатор пользователя сервиса >",
 *                "skip":"<количество пропускаемых записей в списке найденных устройств>",
 *                "num":"<количество возвращаемых устройств>",
 *                "geo_poly":[ x, y, w, h ],
 *              }
 *            }
 *          }
 *
 *          Responce: {
 *            "resp": {
 *              "name":"get_devs",
 *              "status":"< ok | err >",
 *              "count":< общее количество записей в таблице устройств >
 *              "data":"[{<json устройства >}, … ]", /// при err — этого поля не будет.
 *              "desc":"<описание, при ок этого поля не будет>"
 *            }
 *          }
 *
 *          Json с описание устройства: {
 *            "dev_id":"<идентификатор устройства, (уникален) >",
 *            "coll":"<адрес расположения устройства — более подробный чем для объекта>",
 *            "user":"<владелец устройства>",
 *            "geo":[ <longitude>, <latitude> ],
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
    GetDevicesListCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetDevicesListCommand();
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
    ActivateDeviceCommands(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
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
 *             "count":<общее количество записей в таблице событий>
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
    GetEventsListCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetEventsListCommand();
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
    CreateReportCommand(const std::string& token, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~CreateReportCommand();
    virtual Json execute();
};
} /// server
