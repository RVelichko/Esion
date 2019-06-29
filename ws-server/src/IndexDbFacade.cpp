//#include <filesystem>
#include <boost/filesystem.hpp>

#include "Log.hpp"
#include "IndexDbFacade.hpp"


namespace bfs = boost::filesystem;
//namespace fs = std::filesystem;

using namespace sindex;
using namespace server;


bool IndexDbFacade::addIndex(Xapian::WritableDatabase *xdb, const std::string& db_id, const std::string& str) {
    bool is_ok = false;
    if (xdb) {
        Xapian::Document doc;
        Xapian::TermGenerator term_gen;
        term_gen.set_stemmer(Xapian::Stem("ru"));
        term_gen.set_document(doc);
        term_gen.index_text(str, 1, "S");
        doc.set_data(db_id);
        std::string id_term = "Q" + db_id;
        doc.add_boolean_term(id_term);
        xdb->replace_document(id_term, doc);
        is_ok = true;
    } else {
        LOG(FATAL) << "Xapian DB is`t inited!";
    }
    return is_ok;
}


std::vector<std::string> IndexDbFacade::findIndexes(Xapian::WritableDatabase *xdb, const std::string& qstr,
                                                    size_t offset, size_t max_count) {
    std::vector<std::string> ids;
    Xapian::QueryParser qparsr;
    qparsr.set_stemmer(Xapian::Stem("ru"));
    qparsr.set_stemming_strategy(qparsr.STEM_SOME);
    qparsr.add_prefix("title", "S");
    Xapian::Query query = qparsr.parse_query(qstr);
    Xapian::Enquire enquire(*xdb);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(offset, max_count);
    LOG(DEBUG) << "'" << qstr << "'[" << offset << ":" << offset + max_count << "] =";
    for (Xapian::MSetIterator m = mset.begin(); m not_eq mset.end(); ++m) {
        Xapian::docid did = *m;
        LOG(DEBUG) << m.get_rank() + 1 << ": #" << did;
        ids.push_back(m.get_document().get_data());
    }
    return ids;
}


PXapianDatabase IndexDbFacade::initIndexes(const std::string &index_name, const std::string &coll_name) {
    auto xdb = PXapianDatabase(new Xapian::WritableDatabase(_xdb_path + "_" + index_name, Xapian::DB_CREATE_OR_OPEN));
    /// Сверить количество индексных записей с внешней БД.
    if (_db) {
        size_t db_length = _db->getCollectionCount("", coll_name);
        size_t xdb_length = xdb->get_doccount();
        if (xdb_length < db_length) {
            for (size_t i = xdb_length; i < db_length; i += INDEXATE_COUNT) {
                auto add_index_fn = [&](const std::string &coll_name, const Json &jarr) {
                    if (not jarr.empty() and jarr.is_array()) {
                        for (auto jval : jarr) {
                            auto db_id = jval.value("_id", "");
                            auto dev_id = jval.value("dev_id", "");
                            auto coll  = jval.value("coll", "");
                            auto user  = jval.value("user", "");
                            if (not db_id.empty() and not coll.empty()) {
                                addIndex(xdb.get(), db_id, dev_id + " " + coll + " " + user);
                            }
                        }
                    } else {
                        LOG(WARNING) << coll_name << " collection is empty!";
                    }
                };
                if (coll_name == CONTROOLERS_COLLECTION_NAME) {
                    add_index_fn(CONTROOLERS_COLLECTION_NAME, _db->getDevices(INDEXATE_COUNT, i));
                }
                if (coll_name == EVENTS_COLLECTION_NAME) {
                    add_index_fn(EVENTS_COLLECTION_NAME, _db->getEvents(INDEXATE_COUNT, i));
                }
            }
            xdb->commit();
        } else if (db_length < xdb_length) {
            LOG(WARNING) << "DB`s is not relevant!";
        }
    } else {
        LOG(FATAL) << "DB is NULL!";
    }
    return xdb;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


IndexDbFacade::IndexDbFacade(const PDbFacade& db)
    : _db(db) {
    LOG(DEBUG);
}


IndexDbFacade::~IndexDbFacade() {
    LOG(DEBUG);
}


void IndexDbFacade::init(const std::string& xdb_path) {
    _xdb_path = xdb_path;
    /// Создать или прочитать индексную базу.
    _xdb_devs = initIndexes(DEVICES_INDEX_DB_NAME, CONTROOLERS_COLLECTION_NAME);
    _xdb_evs = initIndexes(EVENTS_INDEX_DB_NAME, EVENTS_COLLECTION_NAME);
}


bool IndexDbFacade::addDeviceIndex(const std::string& db_id, const std::string& str) {
    return addIndex(_xdb_devs.get(), db_id, str);
}


bool IndexDbFacade::addEventIndex(const std::string& db_id, const std::string& str) {
    return addIndex(_xdb_evs.get(), db_id, str);
}


std::vector<std::string> IndexDbFacade::getDevicesIndexes(const std::string& str, size_t offset, size_t max_count) {
    return findIndexes(_xdb_devs.get(), str, offset, max_count);
}


std::vector<std::string> IndexDbFacade::getEventsIndexes(const std::string& str, size_t offset, size_t max_count) {
    return findIndexes(_xdb_evs.get(), str, offset, max_count);
}
