#include <exception>

#include <boost/regex.hpp>

#include "Log.hpp"
#include "DbFacade.hpp"

// /etc/mongod.conf -> bindIp: 0.0.0.0
// sudo service mongod restart
// sudo netstat -tnlp

// use admin
// db.createUser({user:"esion",pwd:"esionpassword",roles:[{ role:"userAdminAnyDatabase", db:"admin"}]})

using namespace server;

typedef mongo::BSONObjBuilder BObjBuilder;
typedef mongo::BSONArrayBuilder BArrBuilder;
typedef mongo::Query DbQuery;
typedef std::vector<BsonObj> BsonObjs;


class FindDevicesQuery {
    BsonObj _query;

    /*!
     * \brief  Метод выделяет адрес из строки фильтра для поля "coll"
     */
    std::string extractCollection(std::string& filter) {
        std::string res;
        boost::smatch what;
        //boost::regex_search(filter, what, regex);
        return res;
    }

    /*!
     * \brief  Метод выделяет геопозицию из строки фильтра для поля "geo"
     */
    std::string extractGeoposition(std::string& filter) {
        std::string res;
        boost::smatch what;
        auto regex = boost::regex(R"([ \t\r]*(\d{2}\.\d{2,4},[ \t]{0,4}\d{2}\.\d{2,4})[^\d\S]+)");
        boost::regex_search(filter, what, regex);
        std::string::size_type pos = filter.find(what[1]);
        if (pos not_eq std::string::npos) {
            res = what[1];
            filter.erase(pos, res.size());
        }
        return res;
    }

    /*!
     * \brief  Метод выделяет время последнего объновления из строки фильтра для поля "update_time"
     */
    std::string extractUpdateTime(std::string& filter) {
        std::string res;
        boost::smatch what;
        auto regex = boost::regex(R"(\d{2}\.\d{2}\.(\d{4}|\d{2})(( {1,4})\d{2}:\d{2}:\d{2})?)");
        boost::regex_search(filter, what, regex);
        std::string::size_type pos = filter.find(what[0]);
        if (pos not_eq std::string::npos) {
            res = what[0];
            filter.erase(pos, res.size());
        }
        return res;
    }

    /*!
     * \brief  Метод выделяет <active | not_active> из строки фильтра для поля "status",
     */
    std::string extractStatus(std::string& filter) {
        std::string res;
        boost::smatch what;
        auto regex = boost::regex("(active)|(not_active)");
        boost::regex_search(filter, what, regex);
        std::string::size_type pos = filter.find(what[0]);
        if (pos not_eq std::string::npos) {
            res = what[0];
            filter.erase(pos, res.size());
        }
        return res;
    }

public:
    FindDevicesQuery(const std::string& filter) {
        std::string f = filter;
        std::string status = extractStatus(f);
        std::string update_time = extractUpdateTime(f);
        std::string geo = extractGeoposition(f);
        std::string coll = extractCollection(f);
        if (not coll.empty() and not geo.empty() and not update_time.empty() and not status.empty()) {
            BObjBuilder q;
            if (not coll.empty()) {
                q.append("coll", coll);
            }
            if (not geo.empty()) {
                q.append("geo", geo);
            }
            if (not update_time.empty()) {
                q.append("update_time", update_time);
            }
            if (not status.empty()) {
                q.append("status", status);
            }
            _query = q.obj();
        }
    }

    operator BsonObj () {
        return _query;
    }
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BsonObj DbFacade::toBson(const Json& json) try {
    BsonObj bo = mongo::fromjson(json.dump());
    return bo;
} catch (std::exception& e) {
    LOG(ERROR) << e.what();
    return BsonObj();
}


Json DbFacade::toJson(const BsonObj& bson) try {
    std::string bstr = bson.jsonString();
    Json j = Json::parse(bstr);
    return j;
} catch (std::exception& e) {
    LOG(ERROR) << e.what();
    return Json();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string DbFacade::getMdbNs(const std::string &coll_name) {
    return _db_name + "." + coll_name;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DbFacade::DbFacade() {
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


size_t DbFacade::getCollectionCount(std::string db_name, std::string coll_name) {
    size_t count = 0;
    if (_dbc) {
        if (db_name.empty()) {
            db_name = _db_name;
        }
        if (not coll_name.empty()) {
            count = _dbc->count(db_name + "." + coll_name);
        }
    }
    return count;
}


void DbFacade::disconnect() try {
    _dbc.reset();
    _db_name.clear();
} catch (const mongo::DBException &e) {
    LOG(ERROR) << "Can`t disconnect from DB: " << e.what();
}


Json DbFacade::findUser(const std::string& user, const std::string& pswd) try {
    BsonObj q = BObjBuilder().append("name", user).append("pswd", pswd).obj();
    BsonObj buser = _dbc->findOne(getMdbNs(AUTH_COLLECTION_NAME), q);
    Json juser = DbFacade::toJson(buser);
    return juser;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t find [" << user << " | " << pswd << "] user in DB: " << e.what();
    return Json();
}


Json DbFacade::findUser(const std::string& token) try {
    BsonObj q = BObjBuilder().append("token", token).obj();
    BsonObj buser = _dbc->findOne(getMdbNs(AUTH_COLLECTION_NAME), q);
    Json juser = DbFacade::toJson(buser);
    return juser;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t find [" << token << "] user in DB: " << e.what();
    return Json();
}


bool DbFacade::insertUser(const Json& jusr) try {
    std::string name = jusr["name"];
    std::string pswd = jusr["pswd"];
    auto q = BObjBuilder().append("name", name).append("pswd", pswd).obj();
    auto found_usr = _dbc->findOne(getMdbNs(AUTH_COLLECTION_NAME), q);
    if (found_usr.isEmpty()) {
        _dbc->insert(getMdbNs(AUTH_COLLECTION_NAME), DbFacade::toBson(jusr));
        LOG(INFO) << "Add new user [" << name << "|" << pswd << "] to DB";
        return true;
    } else {
        _dbc->update(getMdbNs(AUTH_COLLECTION_NAME), q, DbFacade::toBson(jusr));
        LOG(INFO) << "Update user [" << name << "|" << pswd << "] in DB";
        return true;
    }
    return false;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t add new device to DB: " << e.what();
    return false;
}


Json DbFacade::getDevices(uint8_t num_objs, uint8_t skip_objs) try {
    BsonObjs found_devs;
    _dbc->findN(found_devs, getMdbNs(CONTROOLERS_COLLECTION_NAME), BObjBuilder().obj(), num_objs,  skip_objs);
    Json jdevs;
    size_t i = 0; 
    for (auto bdev : found_devs) {
        jdevs[i] = (DbFacade::toJson(bdev));
        ++i;
    }
    return jdevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << num_objs << " `" << skip_objs << "] devices from DB: " << e.what();
    return Json();
}


Json DbFacade::getDevicesByGeo(double longitude, double latitude, double radius, size_t skip, size_t num) try {
    BsonObjs found_devs;
    Json jq = {
        {"geo", {
             {"$geoWithin", {
                  {"$centerSphere", {{longitude, latitude}, radius}}
             }}
        }}
    };
    DbQuery q(jq.dump());
    _dbc->findN(found_devs, getMdbNs(CONTROOLERS_COLLECTION_NAME), q, num, skip);
    size_t i = 0;
    Json jdevs;
    for (auto bdev : found_devs) {
        jdevs[i] = (DbFacade::toJson(bdev));
        ++i;
    }
    return jdevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << longitude << " `" << latitude << "] devices from DB: " << e.what();
    return Json();
}


Json DbFacade::getDevicesByGeo(double x, double y, double w, double h, size_t skip, size_t num) try {
    BsonObjs found_devs;
    Json jq = {
        {"geo", {
             {"$geoWithin", {
                  {"$polygon", {{x, y}, {w, y}, {w, h}, {x, h}} }
             }}
        }}
    };
    DbQuery q(jq.dump());
    _dbc->findN(found_devs, getMdbNs(CONTROOLERS_COLLECTION_NAME), q, num, skip);
    size_t i = 0;
    Json jdevs;
    for (auto bdev : found_devs) {
        jdevs[i] = (DbFacade::toJson(bdev));
        ++i;
    }
    return jdevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << x << "," << y << "," << w << "," << h << "] devices from DB: " << e.what();
    return Json();
}


Json DbFacade::getDevicesByIds(std::vector<std::string> ids) try {
    BsonObjs found_devs;
    BArrBuilder bids;
    for (auto id : ids) {
        bids.append(mongo::OID(id));
    }
    auto bq = BObjBuilder().append("_id", BObjBuilder().append("$in", bids.arr()).obj()).obj();
    DbQuery q(bq);
    _dbc->findN(found_devs, getMdbNs(CONTROOLERS_COLLECTION_NAME), q, ids.size());
    size_t i = 0;
    Json jdevs;
    for (auto bdev : found_devs) {
        jdevs[i] = (DbFacade::toJson(bdev));
        ++i;
    }
    return jdevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get devices from DB by isd: " << e.what();
    return Json();
}


bool DbFacade::insertDevice(const Json& dev) try {
    std::string dev_id = dev["dev_id"];
    auto q = BObjBuilder().append("dev_id", dev_id).obj();
    auto found_dev = _dbc->findOne(getMdbNs(CONTROOLERS_COLLECTION_NAME), q);
    if (found_dev.isEmpty()) {
        _dbc->insert(getMdbNs(CONTROOLERS_COLLECTION_NAME), DbFacade::toBson(dev));
        LOG(INFO) << "Add new device [" << dev_id << "] to DB";
        return true;
    } else {
        _dbc->update(getMdbNs(CONTROOLERS_COLLECTION_NAME), q, DbFacade::toBson(dev));
        LOG(INFO) << "Update device [" << dev_id << "] in DB";
        return true;
    }
    return false;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t add new device to DB: " << e.what();
    return false;
}


bool DbFacade::removeDevice(const std::string& dev_id) try {
    auto q = BObjBuilder().append("dev_id", dev_id).obj();
    auto found_dev = _dbc->findOne(getMdbNs(CONTROOLERS_COLLECTION_NAME), q);
    if (not found_dev.isEmpty()) {
        _dbc->remove(getMdbNs(CONTROOLERS_COLLECTION_NAME), q);
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
    auto q = BObjBuilder().append("dev_id", dev_id).obj();
    auto found_dev = _dbc->findOne(getMdbNs(CONTROOLERS_COLLECTION_NAME), q);
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


Json DbFacade::getEvents(uint8_t num_objs, uint8_t skip_objs) try {
    BsonObjs found_evs;
    _dbc->findN(found_evs, getMdbNs(EVENTS_COLLECTION_NAME), BObjBuilder().obj(), num_objs,  skip_objs);
    Json jevs;
    size_t i = 0;
    for (auto bev : found_evs) {
        jevs[i] = (DbFacade::toJson(bev));
        ++i;
    }
    return jevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << num_objs << " `" << skip_objs << "] events from DB: " << e.what();
    return Json();
}


Json DbFacade::getEventsByDevId(const std::string& dev_id, size_t skip, size_t num) try {
    auto q = BObjBuilder().append("dev_id", dev_id).obj();
    auto found_ev = _dbc->findOne(getMdbNs(EVENTS_COLLECTION_NAME), q);
    if (found_ev.isEmpty()) {
        return toJson(found_ev);
    } else {
        LOG(INFO) << "Can`t find event by [" << dev_id << "] in DB";
    }
    return Json();
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get " << dev_id << "[" << num << " `" << skip << "] events from DB: " << e.what();
    return Json();
}


Json DbFacade::getEventsByGeo(double longitude, double latitude, double radius, size_t skip, size_t num) try {
    BsonObjs found_evs;
    Json jq = {
        {"geo", {
             {"$geoWithin", {
                  {"$centerSphere", {{longitude, latitude}, radius}}
             }}
        }}
    };
    DbQuery q(jq.dump());
    _dbc->findN(found_evs, getMdbNs(EVENTS_COLLECTION_NAME), q, num, skip);
    size_t i = 0;
    Json jevs;
    for (auto bev : found_evs) {
        jevs[i] = (DbFacade::toJson(bev));
        ++i;
    }
    return jevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << longitude << "," << latitude << "," << radius << "," << num << " `" << skip
               << "] events from DB: " << e.what();
    return Json();
}


Json DbFacade::getEventsByGeo(double x, double y, double w, double h, size_t skip, size_t num) try {
    BsonObjs found_devs;
    Json jq = {
        {"geo", {
             {"$geoWithin", {
                  {"$polygon", {{x, y}, {w, y}, {w, h}, {x, h}} }
             }}
        }}
    };
    DbQuery q(jq.dump());
    _dbc->findN(found_devs, getMdbNs(EVENTS_COLLECTION_NAME), q, num, skip);
    size_t i = 0;
    Json jdevs;
    for (auto bdev : found_devs) {
        jdevs[i] = (DbFacade::toJson(bdev));
        ++i;
    }
    return jdevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << x << "," << y << "," << w << "," << h << "] events from DB: " << e.what();
    return Json();
}


Json DbFacade::getEventsByIds(std::vector<std::string> ids) try {
    BsonObjs found_evs;
    BArrBuilder bids;
    for (auto id : ids) {
        bids.append(mongo::OID(id));
    }
    auto bq = BObjBuilder().append("_id", BObjBuilder().append("$in", bids.arr()).obj()).obj();
    DbQuery q(bq);
    _dbc->findN(found_evs, getMdbNs(EVENTS_COLLECTION_NAME), q, ids.size());
    size_t i = 0;
    Json jevs;
    for (auto bev : found_evs) {
        jevs[i] = (DbFacade::toJson(bev));
        ++i;
    }
    return jevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get events from DB: " << e.what();
    return Json();
}


bool DbFacade::insertEvent(const Json& ev) try {
    std::string ev_id = ev["ev_id"];
    auto q = BObjBuilder().append("ev_id", ev_id).obj();
    auto found_ev = _dbc->findOne(getMdbNs(EVENTS_COLLECTION_NAME), q);
    if (found_ev.isEmpty()) {
        _dbc->insert(getMdbNs(EVENTS_COLLECTION_NAME), DbFacade::toBson(ev));
        LOG(INFO) << "Add new event [" << ev_id << "] to DB";
        return true;
    } else {
        _dbc->update(getMdbNs(EVENTS_COLLECTION_NAME), q, DbFacade::toBson(ev));
        LOG(INFO) << "Update event [" << ev_id << "] in DB";
        return true;
    }
    return false;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t insert event to DB: " << e.what();
    return Json();
}


bool DbFacade::removeEvent(const std::string& ev_id) try {
    auto q = BObjBuilder().append("ev_id", ev_id).obj();
    auto found_ev = _dbc->findOne(getMdbNs(EVENTS_COLLECTION_NAME), q);
    if (not found_ev.isEmpty()) {
        _dbc->remove(getMdbNs(EVENTS_COLLECTION_NAME), q);
        return true;
    } else {
        LOG(ERROR) << "Can`t remove event [" << ev_id << "].";
    }
    return false;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t remove [" << ev_id << "] event from DB: " << e.what();
    return Json();
}


Json DbFacade::getEvent(const std::string& ev_id) try {
    auto q = BObjBuilder().append("dev_id", ev_id).obj();
    auto found_ev = _dbc->findOne(getMdbNs(EVENTS_COLLECTION_NAME), q);
    if (not found_ev.isEmpty()) {
        return DbFacade::toJson(found_ev);
    } else {
        LOG(ERROR) << "Can`t find event [" << ev_id << "].";
    }
    return Json();
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << ev_id << "] events from DB: " << e.what();
    return Json();
}
