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
    LOG(INFO) << "Connecting to DB [" << addr << ":" << db_name << "," << login << "," << pswd << "]";
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


void DbFacade::eraseOldTokens(size_t timeout) try {
    auto old = static_cast<size_t>(time(nullptr)) - timeout;
    Json jq = {
        {"token", {
            {"$lt", std::to_string(old)}
        }}
    };
    BsonObj q(DbFacade::toBson(jq));
    size_t found = _dbc->query(getMdbNs(AUTH_COLLECTION_NAME), q)->itcount();
    BsonObjs busers;
    _dbc->findN(busers, getMdbNs(AUTH_COLLECTION_NAME), q, found);
    for (auto buser : busers) {
        auto jusr = DbFacade::toJson(buser);
        auto jtokens = jusr["token"];
        if (jtokens.is_array()) {
            Json jeraseds;
            for (auto jtoken : jtokens) {
                size_t t = static_cast<size_t>(std::atoll(jtoken.get<std::string>().c_str()));
                if (old <= t) {
                    jeraseds.push_back(jtoken);
                }
            }
            if (not jeraseds.empty()) {
                jusr["token"] = jeraseds;
                _dbc->update(getMdbNs(AUTH_COLLECTION_NAME), q, DbFacade::toBson(jusr));
            }
        } else {
            LOG(ERROR) << "Tag token is`t array.";
        }
    }
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t erase old tokens. " << e.what();
}


Json DbFacade::findUser(const std::string& user, const std::string& pswd) try {
    Json jq = {
        {"name", user},
        {"pswd", pswd}
    };
    BsonObj q(DbFacade::toBson(jq));
    BsonObj buser = _dbc->findOne(getMdbNs(AUTH_COLLECTION_NAME), q);
    Json juser = DbFacade::toJson(buser);
    return juser;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t find [" << user << " | " << pswd << "] user in DB: " << e.what();
    return Json();
}


Json DbFacade::findUser(const std::string& token) try {
    Json jq = {
        {"token", token}
    };
    BsonObj q(DbFacade::toBson(jq));
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
    LOG(ERROR) << "Can`t add|update user to DB: " << e.what();
    return false;
}


Json DbFacade::getUniqueAddresses(const std::string& coll_id, const std::string& filter) try {
    Json jpipline;
    if (not filter.empty()) {
        jpipline.push_back({
            {"$match", {
                {"$text", {
                    {"$search", filter}
                }},
                {"coll_id", coll_id}
            }}});
    } else {
        jpipline.push_back({
            {"$match", {
                {"coll_id", coll_id}
            }}});
    }
    jpipline.push_back({
        {"$group", {
            {"_id", "$geo"},
            {"address", {
               {"$addToSet", "$coll"}
            }},
            {"device_count", {
              {"$sum", 1}
            }}
        }}});
    jpipline.push_back({
        {"$project", {
            {"_id", 0},
            {"geo", "$_id"},
            {"address", "$address"},
            {"device_count", "$device_count"}
        }}});
    BsonObj bpipline = DbFacade::toBson(jpipline);
    auto db_cursor = _dbc->aggregate(getMdbNs(CONTROOLERS_COLLECTION_NAME), bpipline);
    Json jaddrs;
    if (db_cursor.get()) {
        while (db_cursor->more()) {
            auto baddr = db_cursor->next();
            jaddrs.push_back(DbFacade::toJson(baddr));
        }
    }
    return jaddrs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << filter << "] addresses from DB: " << e.what();
    return Json();
}


Json DbFacade::getList(size_t& total_num, const std::string& db_coll, const std::string& coll_id,
                       const std::string& field, bool direct, size_t num, size_t skip) try {
    BsonObjs founds;
    Json jq = {
        {"coll_id", coll_id}
    };
    DbQuery q(jq.dump());
    total_num = _dbc->query(getMdbNs(db_coll), q)->itcount();
    if (not field.empty()) {
        size_t d = (direct ? 1 : -1);
        auto sq = q.sort(field, d);
        q = sq;
    }
    if (total_num < num) {
        num = total_num;
    }
    Json jexlude_field = {
        {"counters", 0}
    };
    BsonObj bexlude_field = DbFacade::toBson(jexlude_field);
    _dbc->findN(founds, getMdbNs(db_coll), q, num,  skip, &bexlude_field);
    Json jvals;
    size_t i = 0;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception& e) {
    LOG(ERROR) << "Can`t get [" << num << " | " << skip << "] " << field << ": "  << BTOS(direct) << ", "
               << db_coll << " from DB: " << e.what();
    return Json();
}


Json DbFacade::getDevicesByTime(size_t& found, const std::string& coll_id, time_t date_time, std::string& date_type,
                                const std::string& field, bool direct, size_t num, size_t skip) try {
    BsonObjs founds;
    Json jq = {
        {"coll_id", coll_id},
        {date_type, {
             {"$gte", date_time},
             {"$lt", (date_time + 86400)}
        }}
    };
    DbQuery q(jq.dump());
    found = _dbc->query(getMdbNs(CONTROOLERS_COLLECTION_NAME), q)->itcount();
    size_t d = (direct ? 1 : -1);
    auto sq = q.sort(field, d);
    if (found < num) {
        num = found;
    }
    Json jexlude_field = {
        {"counters", 0}
    };
    BsonObj bexlude_field = DbFacade::toBson(jexlude_field);
    _dbc->findN(founds, getMdbNs(CONTROOLERS_COLLECTION_NAME), q, num,  skip, &bexlude_field);
    Json jvals;
    size_t i = 0;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception& e) {
    LOG(ERROR) << "Can`t get [" << num << " | " << skip << "] " << date_type << ": "  << date_time << ", "
               << field << ": "  << BTOS(direct) << ", "
               << "devices from DB: " << e.what();
    return Json();
}


Json DbFacade::getByFilter(size_t& found, const std::string& db_coll, const std::string& coll_id, const std::string& filter,
                           const std::string& field, bool direct, size_t num, size_t skip, bool is_all_fields) try {
    BsonObjs founds;
    Json jq = {
        {"$text", {
             {"$search", filter}
        }},
        {"coll_id", coll_id}
    };
    DbQuery q(jq.dump());
    found = _dbc->query(getMdbNs(db_coll), q)->itcount();
    if (found < num) {
        num = found;
    }
    if (not field.empty()) {
        size_t d = (direct ? 1 : -1);
        auto sq = q.sort(field, d);
        q = sq;
    }
    Json jexlude_field;
    if (is_all_fields) {
        jexlude_field = {
            {"counters.cubic_meter", 0}
        };
    } else {
        jexlude_field = {
            {"counters", 0}
        };
    }
    BsonObj bexlude_field = DbFacade::toBson(jexlude_field);
    _dbc->findN(founds, getMdbNs(db_coll), q, num, skip, &bexlude_field);
    size_t i = 0;
    Json jvals;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception& e) {
    LOG(ERROR) << "Can`t get [" << filter << "] " << db_coll << " from DB with sort ["
               << field << "|" << BTOS(direct) << "]: " << e.what();
    return Json();
}


Json DbFacade::getByGeo(size_t& found, const std::string& db_coll, const std::string& coll_id,
                        double longitude, double latitude, double radius, size_t num, size_t skip) try {
    BsonObjs founds;
    Json jq = {
        {"geo", {
             {"$geoWithin", {
                  {"$centerSphere", {{longitude, latitude}, radius}}
             }}
        }},
        {"coll_id", coll_id}
    };
    DbQuery q(jq.dump());
    found = _dbc->query(getMdbNs(db_coll), q)->itcount();
    if (found < num) {
        num = found;
    }
    Json jexlude_field = {
        {"counters", 0}
    };
    BsonObj bexlude_field = DbFacade::toBson(jexlude_field);
    _dbc->findN(founds, getMdbNs(db_coll), q, num, skip, &bexlude_field);
    size_t i = 0;
    Json jvals;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << longitude << " `" << latitude << "] " << db_coll << " from DB: " << e.what();
    return Json();
}


Json DbFacade::getByPoly(size_t& found, const std::string& db_coll, const std::string& coll_id,
                         double x, double y, double w, double h, size_t num, size_t skip) try {
    BsonObjs founds;
    Json jq = {
        {"geo", {
             {"$geoWithin", {
                  {"$polygon", {{x, y}, {w, y}, {w, h}, {x, h}} }
             }}
        }},
        {"coll_id", coll_id}
    };
    DbQuery q(jq.dump());
    found = _dbc->query(getMdbNs(db_coll), q)->itcount();
    if (found < num) {
        num = found;
    }
    Json jexlude_field = {
        {"counters", 0}
    };
    BsonObj bexlude_field = DbFacade::toBson(jexlude_field);
    _dbc->findN(founds, getMdbNs(db_coll), q, num, skip, &bexlude_field);
    size_t i = 0;
    Json jvals;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << x << "," << y << "," << w << "," << h << "] " << db_coll << " from DB: " << e.what();
    return Json();
}


Json DbFacade::getByIds(const std::string& db_coll, std::vector<std::string> ids) try {
    BsonObjs founds;
    BArrBuilder bids;
    for (auto id : ids) {
        bids.append(mongo::OID(id));
    }
    auto bq = BObjBuilder().append("_id", BObjBuilder().append("$in", bids.arr()).obj()).obj();
    DbQuery q(bq);
    _dbc->findN(founds, getMdbNs(db_coll), q, ids.size());
    size_t i = 0;
    Json jvals;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get devices from DB by isd: " << e.what();
    return Json();
}


bool DbFacade::insertDevice(const Json& dev) try {
    std::string dev_id = dev["dev_id"];
    Json jq = {
        {"dev_id", dev_id}
    };
    DbQuery q(jq.dump());
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
    Json jq = {
        {"dev_id", dev_id}
    };
    DbQuery q(jq.dump());
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
    Json jq = {
        {"dev_id", dev_id}
    };
    DbQuery q(jq.dump());
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


Json DbFacade::getEventsByDevId(size_t& found, const std::string& dev_id, size_t num, size_t skip) try {
    BsonObjs found_evs;
    auto q = BObjBuilder().append("dev_id", dev_id).obj();
    found = _dbc->query(getMdbNs(EVENTS_COLLECTION_NAME), q)->itcount();
    if (found < num) {
        num = found;
    }
    _dbc->findN(found_evs, getMdbNs(EVENTS_COLLECTION_NAME), q, num, skip);
    Json jevs;
    size_t i = 0;
    for (auto bev : found_evs) {
        jevs[i] = (DbFacade::toJson(bev));
        ++i;
    }
    return jevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get " << dev_id << " events from DB: " << e.what();
    return Json();
}


Json DbFacade::getEventsByDevId(size_t& found, const std::string& dev_id, const std::string& field, bool direct,
                                size_t num, size_t skip) try {
    BsonObjs found_evs;
    DbQuery q(BObjBuilder().append("dev_id", dev_id).obj());
    found = _dbc->query(getMdbNs(EVENTS_COLLECTION_NAME), q)->itcount();
    if (found < num) {
        num = found;
    }
    size_t d = direct ? 1 : -1;
    auto sq = q.sort(field, d);
    _dbc->findN(found_evs, getMdbNs(EVENTS_COLLECTION_NAME), sq, num, skip);
    Json jevs;
    size_t i = 0;
    for (auto bev : found_evs) {
        jevs[i] = (DbFacade::toJson(bev));
        ++i;
    }
    return jevs;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get " << dev_id << " events from DB: " << e.what();
    return Json();
}


Json DbFacade::getEventsByTime(size_t& found, const std::string& coll_id, time_t date_time, std::string& date_type,
                               const std::string& field, bool direct, size_t num, size_t skip)  try {
    BsonObjs founds;
    Json jq = {
        {"coll_id", coll_id},
        {date_type, {
             {"$gte", date_time},
             {"$lt", (date_time + 86400)}
        }}
    };
    DbQuery q(jq.dump());
    found = _dbc->query(getMdbNs(EVENTS_COLLECTION_NAME), q)->itcount();
    if (not field.empty()) {
        size_t d = (direct ? 1 : -1);
        auto sq = q.sort(field, d);
        q = sq;
    }
    if (found < num) {
        num = found;
    }
    Json jexlude_field = {
        {"counters", 0}
    };
    BsonObj bexlude_field = DbFacade::toBson(jexlude_field);
    _dbc->findN(founds, getMdbNs(EVENTS_COLLECTION_NAME), q, num,  skip, &bexlude_field);
    Json jvals;
    size_t i = 0;
    for (auto bval : founds) {
        jvals[i] = (DbFacade::toJson(bval));
        ++i;
    }
    return jvals;
} catch (const std::exception& e) {
    LOG(ERROR) << "Can`t get [" << num << " | " << skip << "] " << date_type << ": "  << date_time << ", "
               << field << ": "  << BTOS(direct) << ", "
               << "devices from DB: " << e.what();
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


size_t DbFacade::getCriticalNum(const std::string& coll_id, const std::string& filter) try {
    size_t num = 0;
    Json jpipline;
    if (not filter.empty()) {
        jpipline.push_back({
            {"$match", {
                {"$text", {
                    {"$search", filter}
                }},
                {"priority", "Критический"},
                {"coll_id", coll_id}
            }}
        });
     } else {
        jpipline.push_back({
            {"$match", {
                {"priority", "Критический"},
                {"coll_id", coll_id}
            }}
        });
    }
    jpipline.push_back({
        {"$group", {
            {"_id", "$dev_id"}
        }}
    });
    jpipline.push_back({
        {"$count", "critical_devices"}
    });
    BsonObj bpipline = DbFacade::toBson(jpipline);
    auto db_cursor = _dbc->aggregate(getMdbNs(EVENTS_COLLECTION_NAME), bpipline);
    if (db_cursor.get()) {
        while (db_cursor->more()) {
            auto bs = db_cursor->next();
            auto js = DbFacade::toJson(bs);
            auto jcritical_devices = js.find("critical_devices");
            if (jcritical_devices not_eq js.end() and jcritical_devices->is_number()) {
                num = jcritical_devices->get<size_t>();
            }
        }
    }
    return num;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get [" << filter << "] criticals number from DB: " << e.what();
    return 0;
}


size_t DbFacade::getCriticalNumByDevId(const std::string& coll_id, const std::string& dev_id) try {
    size_t num = 0;
    Json jpipline;
    jpipline.push_back({
        {"$match", {
            {"dev_id", dev_id},
            {"priority", "Критический"},
            {"coll_id", coll_id}
        }}
    });
    jpipline.push_back({
        {"$group", {
            {"_id", "$dev_id"}
        }}
    });
    jpipline.push_back({
        {"$count", "critical_devices"}
    });
    BsonObj bpipline = DbFacade::toBson(jpipline);
    auto db_cursor = _dbc->aggregate(getMdbNs(EVENTS_COLLECTION_NAME), bpipline);
    if (db_cursor.get()) {
        while (db_cursor->more()) {
            auto bs = db_cursor->next();
            auto js = DbFacade::toJson(bs);
            auto jcritical_devices = js.find("critical_devices");
            if (jcritical_devices not_eq js.end() and jcritical_devices->is_number()) {
                num = jcritical_devices->get<size_t>();
            }
        }
    }
    return num;
} catch (const std::exception &e) {
    LOG(ERROR) << "Can`t get for [" << dev_id << "] criticals number from DB: " << e.what();
    return 0;
}

