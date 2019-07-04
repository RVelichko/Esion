/**
 * \brief  Класс обёртка для доступа в БД Mongodb.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once

#include <string>
#include <vector> 
#include <memory>

#include <mongo/client/dbclient.h>

#include "json.hpp"


namespace server {

typedef nlohmann::json Json;
typedef mongo::BSONObj BsonObj;


static const char AUTH_COLLECTION_NAME[] = "users";
static const char CONTROOLERS_COLLECTION_NAME[] = "counters";
static const char EVENTS_COLLECTION_NAME[] = "events";

class DbFacade {
    typedef mongo::DBClientConnection DbConnection;
    typedef std::unique_ptr<DbConnection> PDbConnection;
    
    std::string _db_name;
    PDbConnection _dbc;
    
    /**
     * \brief Метод возвращает имя базы и коллекции для выполнения операций.
     * \param coll_name  Имя коллекции.
     */
    std::string getMdbNs(const std::string &coll_name);

public:
    /**
     * \brief Метод преобразует Json в Bson.
     */ 
    static BsonObj toBson(const Json& json);

    /**
     * \brief Метод преобразует Bson в Json.
     */ 
    static Json toJson(const BsonObj& json);
    
    DbFacade();
    virtual ~DbFacade();
    
    /**
     * \brief Метод возвращает количество записей в коллекции.
     * \param  db_name  Имя коллекции.
     * \param  coll_name  Имя коллекции.
     */
    size_t getCollectionCount(std::string db_name = "", std::string coll_name = "");

    /**
     * \brief Метод преобразует Bson в Json.
     * \param addr    Адрес для подключения к БД.
     * \param db_name Имя БД.
     * \param login   Логин для доступа к БД.
     * \param pswd    Пароль для доступа к БД.
     */ 
    bool connect(const std::string& addr, const std::string db_name, const std::string& login, const std::string& pswd);

    /**
     * \brief Метод выполняет отключение от БД.
     */ 
    void disconnect();
    
    /**
     * \brief Метод возвращает первого найденного пользователя.
     * \param user  Имя пользователя.
     * \param pswd  Пароль пользователя.
     */
    Json findUser(const std::string& user, const std::string& pswd);

    /**
     * \brief Метод возвращает N имеющихся устройств.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */ 
    Json getDevices(uint8_t num_objs = 10, uint8_t skip_objs = 0);

    Json getDevicesByStatus(const std::string& status);

    Json getDevicesByTimeUpdate(time_t time);

    Json getDevicesByGeo(double longitude, double latitude, double radius, size_t skip, size_t num);

    Json getDevicesByIds(std::vector<std::string> ids);

    /**
     * \brief Метод возвращает N имеющихся событий.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */
    Json getEvents(uint8_t num_objs = 10, uint8_t skip_objs = 0);

    /**
     * \brief Метод доавляет новое устройство или обновляет существующее.
     * \param dev Json с описание нового устройства.
     */ 
    bool insertDevice(const Json& dev);

    /**
     * \brief Метод обновляет информацию устройства.
     * \param dev_id Идентификатор удаляемого устройства.
     */ 
    bool removeDevice(const std::string& dev_id);  

    /**
     * \brief Метод возвращает информацию об устройстве.
     * \param dev_id Идентификатор требуемого устройства.
     */ 
    Json getDevice(const std::string& dev_id);
};
} /// server
