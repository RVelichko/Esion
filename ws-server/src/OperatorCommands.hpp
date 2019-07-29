/**
 * \brief  Команды, обрабатываемые операторской частью сервера.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include "JsonCommand.hpp"
#include "DbFacade.hpp"


static const size_t MAX_NUMBER_DEVICES_FOR_REPORT = 10000;
static const size_t DEFAULT_GARBAGE_TIMEOUT = 60 * 60 * 24 * 30; ///< Количество секунд для удаления старых записей.
static const size_t OLD_TOKENS_TIMEOUT = 60 * 60; ///< Количество секунд для удаления старых токенов.

namespace server {

typedef utils::Json Json;
typedef utils::JsonCommand JsonCommand;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef std::function<void(const std::string&)> SendFn;
typedef std::function<bool(const Json&, std::mutex&, const SendFn&)> ExecuteFn;
typedef std::map<std::string, ExecuteFn> CommandsExecuters;
typedef std::map<std::string, std::pair<std::string, std::string>> AuthMap;


class BaseCommand : public JsonCommand {
protected:
    static AuthMap _auth;
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

    /*!
     * \brief Метод выполняет проверку токена на валидность.
     * \param token  Проверяемый токен.
     */
    bool checkToken(const std::string& token);

    /*!
     * \brief Метод возвращает строку идентификатора коллекции.
     * \param token  Токен пользователя коллеции.
     */
    std::string getCollectionId(const std::string& token);

public:
    //static std::string _token; ///< Уникальный токен авторизации, доступен всем командам, управляется командой авторизации.
    static size_t _garb_timer;
    static PDbFacade _db;

    static bool executeByName(const Json& js, std::mutex& mutex, const SendFn& snd_fn);

    BaseCommand(const std::string& name, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
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
    AuthorizeCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
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
 *             "name":"logout",
 *             "data" {
 *               "token":"< идентификатор пользователя сервиса >"
 *             }
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
    LogoutCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~LogoutCommand();
    virtual Json execute();
};


/*!
 * \brief  Получение уникальных адресов:
 *         Rquest 1: {
 *           "cmd": {
 *             "name":"get_uniq_addrs"
 *             "data": {
 *                "token":"< идентификатор пользователя сервиса >",
 *                "filter": <строка фильтра, может отсутствовать>
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"auth",
 *             "status":"< ok | err >",
 *             "uniq_addrs":[<Json c описаниеv адреса>, ...]
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 *
 *        Описание адреса: {
 *           "geo": [latitude, longitute],
 *           "address":[<строка адреса>, ... ],
 *           "device_count" : <количество устройств для данного адреса>
 *        }
 */
class UniqueAddressesCommand : public BaseCommand {
public:
    UniqueAddressesCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~UniqueAddressesCommand();
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
 *                "sort": {
 *                  "field": "<название поля по которому сортируется.>"
 *                  "direction": "<направление сортировки [ asc | desc ] >"
 *                }
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
 *          Rquest 4: {
 *            "cmd": {
 *              "name":"get_devs",
 *              "data": {
 *                "token":"< идентификатор пользователя сервиса >",
 *                "skip":"<количество пропускаемых записей в списке найденных устройств>",
 *                "num":"<количество возвращаемых устройств>",
 *                "date_time":< Дата, по которой необходимо найти устройства >,
 *                "date_type": < start_time (время запуска устройства) | update_time (время последнего обновления) >
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
 *            "start_time":"< время запуска устройства >",
 *            "update_time":"< время последнего подключения к серверу >",
 *            "power_type":"<тип питания устройства>",
 *            "voltage":"<текущее напряжение батареи автономного питания>",
 *            "voltage_perc":"< значение от 0 до 100 >",
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
    GetDevicesListCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
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
    ActivateDeviceCommands(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~ActivateDeviceCommands();
    virtual Json execute();
};


/*!
 * \brief  Запрос списка событий:
 *         Rquest 1: {
 *           "cmd": {
 *             "name":"get_events",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "skip":"<количество пропускаемых записей в найденном списке>",
 *               "num":"<количество возвращаемых событий>",
 *               "dev_id":"<идентификатор устройства, (уникален) >",
 *               "filter":"<любая строка, которой могут соответствовать строки в БД>",
 *               "sort": {
 *                 "field": "<название поля по которому сортируется.>"
 *                 "direction": "<направление сортировки [ asc | desc ] >"
 *               }
 *             }
 *           }
 *         }
 *
 *         Rquest 2: {
 *           "cmd": {
 *             "name":"get_events",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "skip":"<количество пропускаемых записей в списке найденных устройств>",
 *               "num":"<количество возвращаемых устройств>",
 *               "geo_poly":[ x, y, w, h ],
 *             }
 *           }
 *         }
 *
 *         Rquest 3: {
 *           "cmd": {
 *             "name":"get_devs",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "skip":"<количество пропускаемых записей в списке найденных устройств>",
 *               "num":"<количество возвращаемых устройств>",
 *               "date_time":< Дата события >,
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
 *           "desc":"<описание события>"
 *         }
 */
class GetEventsListCommand : public BaseCommand {
public:
    GetEventsListCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetEventsListCommand();
    virtual Json execute();
};


/*!
 * \brief  Команда на формирование отчёта.
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_devices_report",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "coll":"< адрес для которого необходимо сформировать отчёт >",
 *               "encoding": "< название кодировки в которой необходимо представить отчёт >"
 *             }
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_devices_report",
 *             "status":"< ok | err >",
 *             "progress":"<текущий прогресс генерации>"
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_devices_report",
 *             "status":"< ok | err >",
 *             "report_url":"<url с файлом отчёта>"
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class CreateDevicesReportCommand : public BaseCommand {
public:
    CreateDevicesReportCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~CreateDevicesReportCommand();
    virtual Json execute();
};


/*!
 * \brief  Команда на формирование отчёта.
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_events_report",
 *             "data": {
 *               "token":"< идентификатор пользователя сервиса >",
 *               "coll":"< адрес для которого необходимо сформировать отчёт >",
 *               "encoding": "< название кодировки в которой необходимо представить отчёт >"
 *             }
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_events_report",
 *             "status":"< ok | err >",
 *             "progress":"<текущий прогресс генерации>"
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_events_report",
 *             "status":"< ok | err >",
 *             "report_url":"<url с файлом отчёта>"
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class CreateEventsReportCommand : public BaseCommand {
public:
    CreateEventsReportCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~CreateEventsReportCommand();
    virtual Json execute();
};


/*!
 * \brief  Команда возвращает колечество устройств с критическим событием.
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_critical",
 *             "data": {
 *               "filter":"<любая строка, которой могут соответствовать строки в БД>",
 *               "token":"< идентификатор пользователя сервиса >",
 *             }
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_events_report",
 *             "status":"< ok | err >",
 *             "num": < количество анйденных вхождений >
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class GetCriticalNumberCommand : public BaseCommand {
public:
    GetCriticalNumberCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetCriticalNumberCommand();
    virtual Json execute();
};


/*!
 * \brief  Команда возвращает описание устройства по идентификатору.
 *         Rquest: {
 *           "cmd": {
 *             "name": "get_device",
 *             "data": {
 *               "dev_id":"<идентификатор устройства, (уникален) >",
 *               "token":"< идентификатор пользователя сервиса >",
 *             }
 *           }
 *         }
 *         Responce: {
 *           "resp": {
 *             "name":"get_device",
 *             "status":"< ok | err >",
 *             "device": < полный JSON устройства. >
 *             "desc":"<описание, при ок этого поля не будет>"
 *           }
 *         }
 */
class GetDeviceCommand : public BaseCommand {
public:
    GetDeviceCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetDeviceCommand();
    virtual Json execute();
};
} /// server
