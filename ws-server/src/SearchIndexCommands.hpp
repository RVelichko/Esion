/**
 * \brief  Команды, обрабатываемые сервером поискового индекса.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   24.06.2019
 */

#pragma once

#include <functional>
#include <map>

#include "JsonCommand.hpp"
#include "DbFacade.hpp"
#include "IndexDbFacade.hpp"

namespace sindex {

typedef utils::Json Json;
typedef utils::JsonCommand JsonCommand;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef std::shared_ptr<IndexDbFacade> PIndexDbFacade;
typedef std::function<void(const std::string&)> SendFn;
typedef std::function<bool(const Json&, const PIndexDbFacade&, const SendFn&)> ExecuteFn;
typedef std::map<std::string, ExecuteFn> CommandsExecuters;


class BaseCommand : public JsonCommand {
protected:
    static std::mutex _xdb_mutex;
    static std::string _token;

    PIndexDbFacade _xdb;
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
    static std::string _login;
    static std::string _pswd;

    static bool executeByName(const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn);

    BaseCommand(const std::string& name, const Json& js, const PIndexDbFacade &xdb, const SendFn& snd_fn);
};


/*!
 * \brief  Авторизация:
 *         Rquest: {
 *           "cmd": {
 *             "name":"auth",
 *             "data": {
 *                "login":"<имя пользователя>,
 *                "pswd": <пароль>,
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"auth",
 *             "status":"< ok | err >",
 *             "token":"<уникальный номер подключения>"
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class AuthorizeCommand : public BaseCommand {
public:
    AuthorizeCommand(const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn);
    virtual ~AuthorizeCommand();
    virtual Json execute();
};


/*!
 * \brief  Добавление устройств:
 *         Rquest: {
 *           "cmd": {
 *             "name":"add_dev",
 *             "data": {
 *                "qstr":"<строка по которой производится индексация>,
 *                "_id": <идентификатор mongodb>,
 *                "token":"<уникальный номер подключения>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"add_dev",
 *             "status":"< ok | err >",
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class AddDeviceCommand : public BaseCommand {
public:
    AddDeviceCommand(const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn);
    virtual ~AddDeviceCommand();
    virtual Json execute();
};


/*!
 * \brief  Поиск устройств:
 *         Rquest: {
 *           "cmd": {
 *             "name":"find_devs",
 *             "data": {
 *                "qstr":"<поисковая строка>,
 *                "offest":"<смещение от начала в найденном списке>",
 *                "max":"<максимально количество результатов>"
 *                "token":"<уникальный номер подключения>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"find_devs",
 *             "status":"< ok | err >",
 *             "indexes":["<найденная строка>, ..."],
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class FindDevicesCommand : public BaseCommand {
public:
    FindDevicesCommand(const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn);
    virtual ~FindDevicesCommand();
    virtual Json execute();
};


/*!
 * \brief  Добавление события:
 *         Rquest: {
 *           "cmd": {
 *             "name":"add_ev",
 *             "data": {
 *                "qstr":"<строка по которой производится индексация>,
 *                "_id": <идентификатор mongodb>,
 *                "token":"<уникальный номер подключения>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"add_ev",
 *             "status":"< ok | err >",
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class AddEventCommand : public BaseCommand {
public:
    AddEventCommand(const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn);
    virtual ~AddEventCommand();
    virtual Json execute();
};


/*!
 * \brief  Поиск событий:
 *         Rquest: {
 *           "cmd": {
 *             "name":"find_evs",
 *             "data": {
 *                "qstr":"<поисковая строка>,
 *                "offest":"<смещение от начала в найденном списке>",
 *                "max":"<максимально количество результатов>"
 *                "token":"<уникальный номер подключения>"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"find_evs",
 *             "status":"< ok | err >",
 *             "indexes":["<найденная строка>, ..."],
 *             "desc":"<описание, при ок — этого поля не будет>"
 *           }
 *         }
 */
class FindEventsCommand : public BaseCommand {
public:
    FindEventsCommand(const Json& js, const PIndexDbFacade& xdb, const SendFn& snd_fn);
    virtual ~FindEventsCommand();
    virtual Json execute();
};
} /// sindex
