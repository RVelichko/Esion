#pragma once

#include <string>
#include <vector> 
#include <memory>

#include "mongo/client/dbclient.h"

#include "json.hpp"


namespace server {

typedef nlohmann::json Json;
typedef std::vector<Json> Jsons;
typedef mongo::BSONObj BsonObj;


static const char CONTROOLERS_COLLECTION_NAME[] = "test";
    
class DbFacade {
    typedef mongo::DBClientConnection DbConnection;
    typedef std::unique_ptr<DbConnection> PDbConnection;
    
    std::string _db_name;
    PDbConnection _dbc;
    std::string _coll_name;
    
    /**
     * \brief Метод возвращает имя базы и коллекции для выполнения операций.
     */ 
    std::string getMdbNs();

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
     * \brief Метод устанавливает коллекцию в запросах.
     */
    void setCollection(const std::string &coll_name);

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
     * \brief Метод возвращает N имеющихся устройств.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */ 
    Json getDevices(uint8_t num_objs = 10, uint8_t skip_objs = 0);

    /**
     * \brief Метод доавляет новое устройство или обновляет существующее.
     * \param dev Json с описание нового устройства.
     */ 
    bool insertDevice(const Json& dev);

    /**
     * \brief Метод обновляет информацию устройства.
     * \param dev Json с описание нового устройства.
     */ 
    bool updateDevice(const Json& dev);  

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
