/**
 * \brief  Класс обёртка для доступа в индексную БД Xapian.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   24.06.2019
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include <xapian.h>

#include "json.hpp"
#include "DbFacade.hpp"


namespace sindex {

typedef nlohmann::json Json;
typedef std::vector<Json> Jsons;
typedef std::unique_ptr<Xapian::WritableDatabase> PXapianDatabase;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;


static const char DEVICES_INDEX_DB_NAME[] = "devices";
static const char EVENTS_INDEX_DB_NAME[] = "events";
static const uint16_t INDEXATE_COUNT = 100;
static const uint16_t ENQUERY_COUNT = 100;


class IndexDbFacade {
    std::string _xdb_path;
    PXapianDatabase _xdb_devs;
    PXapianDatabase _xdb_evs;
    PDbFacade _db;

    /**
     * \brief Метод добавляет индекс для устройства.
     * \param db_id  Идентификатор строки в БД.
     * \param str  Строка для индексации.
     */
    bool addIndex(Xapian::WritableDatabase *xdb, const std::string& db_id, const std::string& str);

    /**
     * \brief Метод выполняет поиск по индексам.
     * \param db_id  Идентификатор строки в БД.
     * \param str  Строка для поиска.
     * \param offset  Смещение по списку найденных символов.
     * \param max_count  Максимальное количество результатов в ответе.
     * \return  Возвращает массив строк с индексами из основной БД, соответствующей запросу.
     */
    std::vector<std::string> findIndexes(Xapian::WritableDatabase *xdb, const std::string& qstr,
                                         size_t offset = 0, size_t max_count = ENQUERY_COUNT);

    /**
     * \brief Метод выполняет инициалзизацию поисковых индексов.
     * \param index_name  Подимя индексной базы.
     * \param coll_name  Имя коллекции в основной базе, для индексирования.
     */
    PXapianDatabase initIndexes(const std::string &index_name, const std::string &coll_name);

public:
    IndexDbFacade(const PDbFacade& db);
    virtual ~IndexDbFacade();

    /**
     * \brief Метод выполняет индексацию и обновление существующего индеса.
     * \param xdb_path  Путь к индексным БД.
     */
    void init(const std::string& xdb_path);

    /**
     * \brief Метод добавляет индекс для устройства.
     * \param db_id  Идентификатор строки в БД.
     * \param str  Строка для индексации.
     */
    bool addDeviceIndex(const std::string& db_id, const std::string& str);

    /**
     * \brief Метод добавляет индекс для события.
     * \param db_id  Идентификатор строки в БД.
     * \param str  Строка для индексации.
     */
    bool addEventIndex(const std::string& db_id, const std::string& str);

    /**
     * \brief Метод выполняет поиск индексов для устройств.
     * \param str  Поисковая строка.
     * \param offset  Смещение по списку найденных символов.
     * \param max_count  Максимальное количество результатов в ответе.
     */
    std::vector<std::string> getDevicesIndexes(const std::string& str, size_t offset = 0, size_t max_count = ENQUERY_COUNT);

    /**
     * \brief Метод выполняет поиск индексов для событий.
     * \param str  Поисковая строка.
     * \param offset  Смещение по списку найденных символов.
     * \param max_count  Максимальное количество результатов в ответе.
     */
    std::vector<std::string> getEventsIndexes(const std::string& str, size_t offset = 0, size_t max_count = ENQUERY_COUNT);
};
} /// sindex
