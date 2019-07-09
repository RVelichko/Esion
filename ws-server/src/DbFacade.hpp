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
     * \brief Метод возвращает первого найденного пользователя.
     * \param token  Токен авторизации пользователя.
     */
    Json findUser(const std::string& token);

    /**
     * \brief Метод возвращает первого найденного пользователя.
     * \param jusr  JSON с описание пользователя.
     */
    bool insertUser(const Json& jusr);

    /**
     * \brief Метод возвращает N имеющихся устройств.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */ 
    Json getDevices(size_t num_objs = 10, size_t skip_objs = 0);

    /**
     * \brief Метод возвращает N имеющихся устройств по точке и радиусу геолокации.
     * \param longitude  Долгота.
     * \param latitude  Широта.
     * \param radius  Радиус в геокоординатах.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */
    Json getDevicesByGeo(double longitude, double latitude, double radius, size_t skip, size_t num);

    /**
     * \brief Метод возвращает N имеющихся устройств по полигону геолокации.
     * \param x  Долгота лево.
     * \param y  Широта низ.
     * \param w  Долгота право.
     * \param h  Широта верх.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */
    Json getDevicesByGeo(double x, double y, double w, double h, size_t skip, size_t num);

    /**
     * \brief Метод возвращает N имеющихся устройств по идентификаторам БД.
     * \param ids  Массив строк с идентификаторами из БД.
     */
    Json getDevicesByIds(std::vector<std::string> ids);

    /**
     * \brief Метод доавляет новое устройство или обновляет существующее.
     * \param dev Json с описание нового устройства.
     */
    bool insertDevice(const Json& dev);

    /**
     * \brief Метод удаляет информацию об устройстве.
     * \param dev_id Идентификатор удаляемого устройства.
     */
    bool removeDevice(const std::string& dev_id);

    /**
     * \brief Метод возвращает информацию об устройстве.
     * \param dev_id Идентификатор требуемого устройства.
     */
    Json getDevice(const std::string& dev_id);

    /**
     * \brief Метод возвращает N имеющихся событий.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */
    Json getEvents(size_t num_objs = 10, size_t skip_objs = 0);

    /**
     * \brief Метод возвращает N имеющихся событий по идентификатору устройства.
     * \param dev_id  Идентификатор устройства.
     * \param num_objs  Количество запрашиваемых устройств.
     * \param skip_objs Количество пропускаемых в запросе устройств.
     */
    Json getEventsByDevId(const std::string& dev_id, size_t skip, size_t num);

    /**
     * \brief Метод возвращает N имеющихся событий по точке и радиусу геолокации.
     * \param longitude  Долгота.
     * \param latitude  Широта.
     * \param radius  Радиус в геокоординатах.
     * \param num_objs  Количество запрашиваемых событий.
     * \param skip_objs Количество пропускаемых в запросе событий.
     */
    Json getEventsByGeo(double longitude, double latitude, double radius, size_t skip, size_t num);

    /**
     * \brief Метод возвращает N имеющихся событий по полигону геолокации.
     * \param x  Долгота лево.
     * \param y  Широта низ.
     * \param w  Долгота право.
     * \param h  Широта верх.
     * \param num_objs  Количество запрашиваемых событий.
     * \param skip_objs Количество пропускаемых в запросе событий.
     */
    Json getEventsByGeo(double x, double y, double w, double h, size_t skip, size_t num);

    /**
     * \brief Метод возвращает N имеющихся событий по идентификаторам БД.
     * \param ids  Массив строк с идентификаторами из БД.
     */
    Json getEventsByIds(std::vector<std::string> ids);

    /**
     * \brief Метод доавляет новое событие или обновляет существующее.
     * \param ev Json с описание нового событие.
     */
    bool insertEvent(const Json& ev);

    /**
     * \brief Метод удаляет информация о событие.
     * \param ev_id Идентификатор удаляемого события.
     */
    bool removeEvent(const std::string& ev_id);

    /**
     * \brief Метод возвращает информацию о событии.
     * \param ev_id Идентификатор требуемого события.
     */
    Json getEvent(const std::string& ev_id);
};
} /// server
