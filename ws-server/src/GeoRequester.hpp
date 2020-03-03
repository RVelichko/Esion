/**
 * \brief  Класс реализующего запрос геопозиции у Яндекс карт API.
 * \author R.N.Velichko rostislav.vel@gmail.com
 * \date   14.06.2019
 */

#pragma once

#include <string>
#include <utility>

#include "json.hpp"
#include "DbFacade.hpp"


namespace server {

typedef nlohmann::json Json;

typedef std::pair<double, double> GeoLocation;
typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;


class GeoRequester {
    std::string _api_key;

    Json parseResult(const std::string& body_buf, Json& jgeo);

public:
    GeoRequester(const std::string& api_key);
    virtual ~GeoRequester();

    /*!
     * \brief Метод выполняет запрос координат по заданному адресу.
     * \param coll  Строка с искомым адресом.
     * \return Json: {
     *           "addr_line" : "<откорректированная строка адреса>",
     *           "point" : [<latitude>, <longitude>]
     *         }
     */
    Json request(const std::string& coll);
};
} /// server
