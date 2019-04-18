#include <exception>

#include "Log.hpp"
#include "DbFacade.hpp"

// /etc/mongod.conf -> bindIp: 0.0.0.0
// sudo service mongod restart
// sudo netstat -tnlp

// use admin
// db.createUser({user:"esion",pwd:"esionpassword",roles:[{ role:"userAdminAnyDatabase", db:"admin"}]})

using namespace server;

typedef mongo::BSONObjBuilder BsonObjBuilder;
typedef std::vector<BsonObj> BsonObjs;



BsonObj DbFacade::toBson(const Json& json) try {
    BsonObj bo = mongo::fromjson(json.dump());
    return bo;
} catch (std::exception& e) {
    LOG(ERROR) << e.what();
    return BsonObj();
}


Json DbFacade::toJson(const BsonObj& bson) try {
    Json j = Json::parse(bson.jsonString());
    return j;
} catch (std::exception& e) {
    LOG(ERROR) << e.what();
    return Json();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string DbFacade::getMdbNs() {
    return _db_name + "." + _coll_name;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DbFacade::DbFacade()
    : _coll_name(CONTROOLERS_COLLECTION_NAME) {
    LOG(DEBUG);
    mongo::client::initialize();
}


DbFacade::~DbFacade() {
    LOG(DEBUG);
}


bool DbFacade::connect(const std::string& addr, 
                       const std::string db_name, 
                       const std::string& login, 
                       const std::string& pswd) try {
    _db_name = db_name;
    bool is_ok = true;
    _dbc.reset(new DbConnection());
    LOG(ERROR) << "Connecting to DB [" << addr << ":" << db_name << "," << login << "," << pswd << "]";
    std::string err_msg;
    if (not _dbc->connect(addr, err_msg)) {
        LOG(ERROR) << "Can`t connect to DB: " << err_msg;
        is_ok = false;
    } else {
        if (_dbc->auth(db_name, login, pswd, err_msg, false)) {
            LOG(ERROR) << "Can`t authorized to DB [" << db_name << "," << login << "," << pswd << "]: " << err_msg;
            is_ok = false;
        }
    }
    return is_ok;
} catch (const mongo::DBException &e) {
    LOG(ERROR) << "Can`t connect to DB: " << e.what();
    return false;
}


void DbFacade::setCollection(const std::string &coll_name) {
    if (coll_name.empty()) {
        _coll_name = CONTROOLERS_COLLECTION_NAME;
    } else {
        _coll_name = coll_name;
    }
}


void DbFacade::disconnect() try {
    _dbc.reset();
    _db_name.clear();
} catch (const mongo::DBException &e) {
    LOG(ERROR) << "Can`t disconnect from DB: " << e.what();
}


Json DbFacade::getDevices(uint8_t num_objs, uint8_t skip_objs) try {
    BsonObjs found_devs;
    _dbc->findN(found_devs, getMdbNs(), BsonObjBuilder().obj(), num_objs,  skip_objs);
    Json jdevs;
    size_t i = 0; 
    for (auto bdev : found_devs) {
        jdevs[i] = (DbFacade::toJson(bdev));
        ++i;
    }
    return jdevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << num_objs << " `" << skip_objs << "] devices from DB: " << e.what();
    return Jsons();
}


bool DbFacade::insertDevice(const Json& dev) try {
    std::string dev_id = dev["id"];
    auto q = BsonObjBuilder().append("id", dev_id).obj();
    auto found_dev = _dbc->findOne(getMdbNs(), q);
    if (found_dev.isEmpty()) {
        _dbc->insert(getMdbNs(), DbFacade::toBson(dev));
        LOG(INFO) << "Add new device [" << dev_id << "] to DB";
        return true;
    } else {
        _dbc->update(getMdbNs(), q, DbFacade::toBson(dev));
        LOG(INFO) << "Update device [" << dev_id << "] in DB";
        return true;
    } 
    return false;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t add new device to DB: " << e.what();
    return false;
}


bool DbFacade::removeDevice(const std::string& dev_id) try {
    auto q = BsonObjBuilder().append("id", dev_id).obj();
    auto found_dev = _dbc->findOne(getMdbNs(), q);
    if (not found_dev.isEmpty()) {
        _dbc->remove(getMdbNs(), q);
        return true;
    } else {
        LOG(ERROR) << "Can`t remove device [" << dev_id << "].";
    }
    return false;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t delete device [" << dev_id << "] in DB: " << e.what();
    return false;
}


Json DbFacade::getDevice(const std::string& dev_id) try {
    auto q = BsonObjBuilder().append("id", dev_id).obj();
    auto found_dev = _dbc->findOne(getMdbNs(), q);
    if (not found_dev.isEmpty()) {
        return DbFacade::toJson(found_dev);
    } else {
        LOG(ERROR) << "Can`t find device [" << dev_id << "].";
    }
    return Json();
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get device [" << dev_id << "] in DB: " << e.what();
    return false;
}