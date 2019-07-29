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


static const char AUTH_COLLECTION_NAME[] = "users";
static const char CONTROOLERS_COLLECTION_NAME[] = "counters";
static const char EVENTS_COLLECTION_NAME[] = "events";
static const size_t DEFAULT_NUMBER_REQUESTED = 100;


namespace server {

typedef nlohmann::json Json;
typedef mongo::BSONObj BsonObj;


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
     * \param timeout  Таймаут для удаления устаревших токенов.
     */
    void eraseOldTokens(size_t timeout);

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
     * \brief Метод возвращает список уникальных адресов из базы.
     * \param coll_id  Идентификатор пользовательской коллекции.
     * \param filter  Строка с фильтром адреса.
     */
    Json getUniqueAddresses(const std::string& coll_id, const std::string& filter);

    /**
     * \brief Метод возвращает N имеющихся устройств.
     * \param [OUT] total_num  Общее количество устройств.
     * \param db_coll  Имя коллекции с записями.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param field  Имя поля по которому необходимо отсортировать результат выдачи.
     * \param direct  Направление сортировки при TRUE - в прямом порядке, при FALSE - в обратном.
     * \param num  Количество запрашиваемых устройств.
     * \param skip Количество пропускаемых в запросе устройств.
     */
    Json getList(size_t& total_num, const std::string& db_coll, const std::string& coll_id,
                 const std::string& field = "", bool direct = true,
                 size_t num = DEFAULT_NUMBER_REQUESTED, size_t skip = 0);

    /**
     * \brief Метод возвращает N имеющихся устройств.
     * \param [OUT] found  Количество найденных устройств.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param date_time  Врямя по которому необходимо осуществить поиск.
     * \param date_type  Тип времени для устройства [время запуска устройства / время последнего обновления].
     * \param field  Имя поля по которому необходимо отсортировать результат выдачи.
     * \param direct  Направление сортировки при TRUE - в прямом порядке, при FALSE - в обратном.
     * \param num  Количество запрашиваемых устройств.
     * \param skip Количество пропускаемых в запросе устройств.
     */
    Json getDevicesByTime(size_t& found, const std::string& coll_id, time_t date_time, std::string& date_type,
                          const std::string& field = "", bool direct = true,
                          size_t num = DEFAULT_NUMBER_REQUESTED, size_t skip = 0);

    /**
     * \brief Метод возвращает N имеющихся устройств.
     * \param [OUT] found  Количество найденных записей, соответствующих фильтру.
     * \param db_coll  Имя коллекции с записями.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param filter  Строка с фильтром.
     * \param field  Имя поля по которому необходимо отсортировать результат выдачи.
     * \param direct  Направление сортировки при TRUE - в прямом порядке, при FALSE - в обратном.
     * \param num  Количество запрашиваемых устройств.
     * \param skip Количество пропускаемых в запросе устройств.
     * \param is_all_fields  Возвращать все поля в запросе.
     */
    Json getByFilter(size_t& found, const std::string& db_coll, const std::string& coll_id, const std::string& filter,
                     const std::string& field = "", bool direct = true,
                     size_t num = DEFAULT_NUMBER_REQUESTED, size_t skip = 0, bool is_all_fields = false);

    /**
     * \brief Метод возвращает N имеющихся устройств по точке и радиусу геолокации.
     * \param [OUT] found  Количество найденных записей, соответствующих фильтру.
     * \param db_coll  Имя коллекции с записями.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param longitude  Долгота.
     * \param latitude  Широта.
     * \param radius  Радиус в геокоординатах.
     * \param num  Количество запрашиваемых устройств.
     * \param skip  Количество пропускаемых в запросе устройств.
     */
    Json getByGeo(size_t& found, const std::string& db_coll, const std::string& coll_id,
                  double longitude, double latitude, double radius, size_t num, size_t skip);

    /**
     * \brief Метод возвращает N имеющихся устройств по полигону геолокации.
     * \param [OUT] found  Количество найденных записей, соответствующих фильтру.
     * \param db_coll  Имя коллекции с записями.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param x  Долгота лево.
     * \param y  Широта низ.
     * \param w  Долгота право.
     * \param h  Широта верх.
     * \param num  Количество запрашиваемых устройств.
     * \param skip  Количество пропускаемых в запросе устройств.
     */
    Json getByPoly(size_t& found, const std::string& db_coll, const std::string& coll_id,
                   double x, double y, double w, double h, size_t num, size_t skip);

    /**
     * \brief Метод возвращает N имеющихся устройств по идентификаторам БД.
     * \param db_coll  Имя коллекции с записями.
     * \param ids  Массив строк с идентификаторами из БД.
     */
    Json getByIds(const std::string& db_coll, std::vector<std::string> ids);

    /**
     * \brief Метод доавляет новое устройство или обновляет существующее.
     * \param dev  Json с описание нового устройства.
     */
    bool insertDevice(const Json& dev);

    /**
     * \brief Метод удаляет информацию об устройстве.
     * \param dev_id  Идентификатор удаляемого устройства.
     */
    bool removeDevice(const std::string& dev_id);

    /**
     * \brief Метод возвращает информацию об устройстве.
     * \param dev_id  Идентификатор требуемого устройства.
     */
    Json getDevice(const std::string& dev_id);

    /**
     * \brief Метод возвращает N имеющихся событий по идентификатору устройства.
     * \param found  Количество найденных записей, соответствующих фильтру.
     * \param dev_id  Идентификатор устройства.
     * \param num  Количество запрашиваемых устройств.
     * \param skip Количество пропускаемых в запросе устройств.
     */
    Json getEventsByDevId(size_t& found, const std::string& dev_id, size_t num = DEFAULT_NUMBER_REQUESTED, size_t skip = 0);

    /**
     * \brief Метод возвращает N имеющихся событий по идентификатору устройства.
     * \param found  Количество найденных записей, соответствующих фильтру.
     * \param dev_id  Идентификатор устройства.
     * \param field  Имя поля по которому необходимо отсортировать результат выдачи.
     * \param direct  Направление сортировки при TRUE - в прямом порядке, при FALSE - в обратном.
     * \param num  Количество запрашиваемых устройств.
     * \param skip Количество пропускаемых в запросе устройств.
     */
    Json getEventsByDevId(size_t& found, const std::string& dev_id, const std::string& field, bool direct,
                          size_t num = DEFAULT_NUMBER_REQUESTED, size_t skip = 0);

    /**
     * \brief Метод возвращает N имеющихся устройств.
     * \param [OUT] found  Количество найденных устройств.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param date_time  Врямя по которому необходимо осуществить поиск.
     * \param field  Имя поля по которому необходимо отсортировать результат выдачи.
     * \param direct  Направление сортировки при TRUE - в прямом порядке, при FALSE - в обратном.
     * \param num  Количество запрашиваемых устройств.
     * \param skip Количество пропускаемых в запросе устройств.
     */
    Json getEventsByTime(size_t& found, const std::string& coll_id, time_t date_time, std::string& date_type,
                         const std::string& field = "", bool direct = true,
                         size_t num = DEFAULT_NUMBER_REQUESTED, size_t skip = 0);

    /**
     * \brief Метод доавляет новое событие или обновляет существующее.
     * \param dev  Json с описание нового события.
     */
    bool insertEvent(const Json& ev);

    /**
     * \brief Метод удаляет информацию о событии.
     * \param ev_id  Идентификатор удаляемого события.
     */
    bool removeEvent(const std::string& ev_id);

    /**
     * \brief Метод возвращает информацию о событии.
     * \param ev_id  Идентификатор требуемого устройства.
     */
    Json getEvent(const std::string& ev_id);

    /**
     * \brief Метод возвращает количество устройств, соотвествующих фильтру, у которых емеется критическое событие.
     * \param coll_id  Идентификатор пользователя коллекции.
     * \param filter  Строка с фильтром.
     */
    size_t getCriticalNum(const std::string& coll_id, const std::string& filter);
};
} /// server
