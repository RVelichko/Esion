/**
 * \brief  Команды от страницы управления пользователями.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include "JsonCommand.hpp"
#include "DbFacade.hpp"


static const size_t MAX_NUMBER_USERS_FOR_REPORT = 10000;


namespace server {

typedef utils::Json Json;
typedef utils::JsonCommand JsonCommand;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef std::function<void(const std::string&)> SendFn;
typedef std::function<bool(const Json&, std::mutex&, const SendFn&)> ExecuteFn;
typedef std::map<std::string, ExecuteFn> CommandsExecuters;
typedef std::pair<std::string, std::string> Auth;


class UsersAdminBaseCommand : public JsonCommand {
protected:
    static Auth _auth;
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
    static size_t _garb_timer;
    static PDbFacade _db;

    static bool executeByName(const Json& js, std::mutex& mutex, const SendFn& snd_fn);

    UsersAdminBaseCommand(const std::string& name, const Json& js, std::mutex& mutex, const SendFn& snd_fn);
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
class AdminAuthorizeCommand : public UsersAdminBaseCommand {
public:
    AdminAuthorizeCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~AdminAuthorizeCommand();

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
class AdminLogoutCommand : public UsersAdminBaseCommand {
public:
    AdminLogoutCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~AdminLogoutCommand();
    virtual Json execute();
};


/*!
 * \brief  Получение списка пользователей:
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_users"
 *             "data": {
 *                "token":"< идентификатор пользователя сервиса >",
 *                "filter": <строка фильтра, может отсутствовать>
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name":"get_users",
 *             "status":"< ok | err >",
 *             "users":["< Json c описанием пользователя >", ...]
 *             "desc":"< описание, при ок — этого поля не будет >"
 *           }
 *         }
 *
 *        Описание пользователя: {
 *           "name": [ "< Ф >", "< И >", "< О >" ],
 *           "auth": [ "login", "pswd" ],
 *           "addr": "< Фактический адрес >",
 *           "ury_addr": "< Юридический адрес >",
 *           "contacts": "< Контакты >",
 *           "obj_arrds" : [ "< Адрес объекта >" ],
 *           "desc": "< Описание >"
 *           "status": "< true | false >"
 *        }
 */
class GetUsersCommand : public UsersAdminBaseCommand {
public:
    GetUsersCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetUsersCommand();
    virtual Json execute();
};


/*!
 * \brief  Получение информации о пользователе:
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_user"
 *             "data": {
 *                "token": "< идентификатор пользователя сервиса >",
 *                "filter": "< строка фильтра, может отсутствовать >"
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name": "get_user",
 *             "status": "< ok | err >",
 *             "users": "< Json c описаниеv адреса >",
 *             "desc": "< описание, при ок — этого поля не будет >"
 *           }
 *         }
 *
 *        Описание пользователя: {
 *           "name": [ "< Ф >", "< И >", "< О >" ],
 *           "auth": [ "login", "pswd" ],
 *           "addr": "< Фактический адрес >",
 *           "ury_addr": "< Юридический адрес >",
 *           "contacts": "< Контакты >",
 *           "obj_arrds" : [ "< Адрес объекта >" ],
 *           "desc": "< Описание >"
 *           "status": "< true | false >"
 *        }
 */
class GetUserCommand : public UsersAdminBaseCommand {
public:
    GetUserCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetUserCommand();
    virtual Json execute();
};


/*!
 * \brief  Добавление информации о пользователе:
 *         Rquest: {
 *           "cmd": {
 *             "name":"add_user"
 *             "data": {
 *                "token": "< Идентификатор пользователя сервиса >",
 *                "user": "< Json c описанием пользователя >",
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name": "get_user",
 *             "status": "< ok | err >",
 *             "desc": "< Описание, при ок — этого поля не будет >"
 *           }
 *         }
 *
 *        Описание пользователя: {
 *           "name": [ "< Ф >", "< И >", "< О >" ],
 *           "auth": [ "login", "pswd" ],
 *           "addr": "< Фактический адрес >",
 *           "ury_addr": "< Юридический адрес >",
 *           "contacts": "< Контакты >",
 *           "obj_arrds" : [ "< Адрес объекта >" ],
 *           "desc": "< Описание >",
 *           "status": "< true | false >"
 *        }
 */
class AddUserCommand : public UsersAdminBaseCommand {
public:
    AddUserCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~AddUserCommand();
    virtual Json execute();
};


/*!
 * \brief  Обновление информации о пользователе:
 *         Rquest: {
 *           "cmd": {
 *             "name":"update_user"
 *             "data": {
 *                "token": "< Идентификатор пользователя сервиса >",
 *                "user": "< Json c описанием пользователя >",
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name": "get_user",
 *             "status": "< ok | err >",
 *             "desc": "< Описание, при ок — этого поля не будет >"
 *           }
 *         }
 *
 *        Описание пользователя: {
 *           "name": [ "< Ф >", "< И >", "< О >" ],
 *           "auth": [ "login", "pswd" ],
 *           "addr": "< Фактический адрес >",
 *           "ury_addr": "< Юридический адрес >",
 *           "contacts": "< Контакты >",
 *           "obj_arrds" : [ "< Адрес объекта >" ],
 *           "desc": "< Описание >"
 *           "status": "< true | false >"
 *        }
 */
class UpdateUserCommand : public UsersAdminBaseCommand {
public:
    UpdateUserCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~UpdateUserCommand();
    virtual Json execute();
};


/*!
 * \brief  Получение адресов обслуживаемых данным пользователем:
 *         Rquest: {
 *           "cmd": {
 *             "name":"get_addrs"
 *             "data": {
 *                "token": "< Идентификатор пользователя сервиса >",
 *                "user_id": "< Идентификатор пользователя >",
 *             }
 *           }
 *         }
 *
 *         Responce: {
 *           "resp": {
 *             "name": "get_user",
 *             "status": "< ok | err >",
 *             "desc": "< Описание, при ок — этого поля не будет >"
 *           }
 *         }
 */
class GetUserAddressesCommand : public UsersAdminBaseCommand {
public:
    GetUserAddressesCommand(const Json& js, std::mutex& mutex, const SendFn& snd_fn);
    virtual ~GetUserAddressesCommand();
    virtual Json execute();
};
} // server
