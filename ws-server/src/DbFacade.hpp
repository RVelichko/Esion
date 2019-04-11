#pragma once

#include <string>
#include <vector> 
#include <memory>

#include "mongo/client/dbclient.h"

#include "json.hpp"

namespace server {

typedef nlohmann::json Json;
typedef std::vector<std::string> DevIds; 
typedef mongo::BSONObj BsonObj;


static const char CONTROOLERS_COLLECTION_NAME[] = "controllers";
    
class DbFacade {
    typedef mongo::DBClientConnection DbConnection;
    typedef std::unique_ptr<DbConnection> PDbConnection;
    
    std::string _db_name;
    PDbConnection _dbc;
    
    std::string getMdbNs();

public:
    static BsonObj toBson(const Json& json);
    static Json toJson(const BsonObj& json);
    
    DbFacade();
    virtual ~DbFacade();
    
    bool connect(const std::string& add, const std::string db_name, const std::string& login, const std::string& pswd);
    void disconnect();
    
    DevIds getIds();
    bool addDevice(const Json& dev);
    bool updateDevice(const Json& dev);  
    bool deleteDevice(const std::string& dev_id);  
    Json getDevice(const std::string& dev_id);
};
} /// server
