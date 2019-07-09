//#include <filesystem>
#include <boost/filesystem.hpp>

#include "Log.hpp"
#include "IndexDbFacade.hpp"


namespace bfs = boost::filesystem;

using namespace sindex;
using namespace server;


bool IndexDbFacade::addIndex(Xapian::WritableDatabase *xdb, const std::string& db_id, const std::string& str) try {
    bool is_ok = false;
    if (xdb) {
        Xapian::Document doc;
        Xapian::TermGenerator term_gen;
        term_gen.set_stemmer(Xapian::Stem("ru"));
        term_gen.set_document(doc);
        term_gen.index_text("addr", 1, "S");
        term_gen.index_text(str);
        doc.set_data(db_id);
        std::string id_term = "Q" + db_id;
        doc.add_boolean_term(id_term);
        //xdb->add_document(doc);
        xdb->replace_document(id_term, doc);
        is_ok = true;
        LOG(TRACE) << "[" << db_id << "] " << str;
    } else {
        LOG(FATAL) << "Xapian DB is`t inited!";
    }
    return is_ok;
} catch (std::exception& e) {
    LOG(ERROR) << "Can`t add index: \"" << e.what() << "\"";
    return false;
}


IndexIds IndexDbFacade::findIndexes(Xapian::WritableDatabase *xdb, const std::string& qstr,
                                                    size_t offset, size_t max_count) try {
    IndexIds ids;
    Xapian::QueryParser qparsr;
    qparsr.set_stemmer(Xapian::Stem("ru"));
    qparsr.set_stemming_strategy(qparsr.STEM_SOME);
    qparsr.add_prefix("addr", "S");
    Xapian::Query query = qparsr.parse_query(qstr);
    Xapian::Enquire enquire(*xdb);
    enquire.set_query(query);
    Xapian::MSet mset = enquire.get_mset(offset, max_count);
    LOG(DEBUG) << "'" << qstr << "'[" << offset << ":" << offset + max_count << "] =";
    for (Xapian::MSetIterator m = mset.begin(); m not_eq mset.end(); ++m) {
        Xapian::docid did = *m;
        LOG(DEBUG) << m.get_rank() + 1 << ": #" << m.get_document().get_data();
        ids.push_back(m.get_document().get_data());
    }
    return ids;
} catch (std::exception& e) {
    LOG(ERROR) << "Can`t find indexes: \"" << e.what() << "\"";
    return std::vector<std::string>();
}


PXapianDatabase IndexDbFacade::initIndexes(const std::string &index_name, const std::string &coll_name) try {
    PXapianDatabase xdb;
    auto path = bfs::path(_xdb_path + "_" + index_name);
    xdb = PXapianDatabase(new Xapian::WritableDatabase(path.string(), Xapian::DB_CREATE_OR_OPEN));
    /// Сверить количество индексных записей с внешней БД.
    if (_db and xdb) {
        size_t db_length = _db->getCollectionCount("", coll_name);
        size_t xdb_length = xdb->get_doccount();
        if (xdb_length < db_length) {
            auto add_index_fn = [&](const std::string &coll_name, const Json &jarr) {
                if (not jarr.empty() and jarr.is_array()) {
                    for (auto jval : jarr) {
                        auto jdb_id = jval.find("_id");
                        auto jcoll  = jval.find("coll");
                        std::string db_id;
                        if (jdb_id not_eq jval.end() and jdb_id->is_object() and
                            jcoll not_eq jval.end() and jcoll->is_string()) {
                            db_id = jdb_id->value("$oid", "");
                            if (not db_id.empty()) {
                                std::string str =
                                        jval.value("dev_id", "") + " " +
                                        jval.value("coll", "") + " " +
                                        jval.value("user", "");
                                addIndex(xdb.get(), db_id, str);
                            } else {
                                LOG(ERROR) << "id is empty!";
                            }
                        } else {
                            LOG(ERROR) << "Can`t indexate unit " << db_id << " !";
                        }
                    }
                } else {
                    LOG(WARNING) << coll_name << " collection is empty!";
                }
            };
            for (size_t i = xdb_length; i <= db_length; ) {
                if (coll_name == CONTROOLERS_COLLECTION_NAME) {
                    auto jdevs = _db->getDevices(INDEXATE_COUNT, i);
                    add_index_fn(CONTROOLERS_COLLECTION_NAME, jdevs);
                }
                if (coll_name == EVENTS_COLLECTION_NAME) {
                    auto jevs = _db->getEvents(INDEXATE_COUNT, i);
                    add_index_fn(EVENTS_COLLECTION_NAME, jevs);
                }
                if (i + INDEXATE_COUNT < db_length) {
                    i += INDEXATE_COUNT;
                } else if (i not_eq db_length) {
                    i += db_length - i;
                } else {
                    break;
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
} catch (std::exception& e) {
    LOG(FATAL) << "Can`t init index DB: \"" << e.what() << "\"";
    return PXapianDatabase();
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
    if (_xdb_devs) {
        return addIndex(_xdb_devs.get(), db_id, str);
    }
    return false;
}


bool IndexDbFacade::addEventIndex(const std::string& db_id, const std::string& str) {
    if (_xdb_devs) {
        return addIndex(_xdb_evs.get(), db_id, str);
    }
    return false;
}


IndexIds IndexDbFacade::getDevicesIndexes(const std::string& str, size_t offset, size_t max_count) {
    if (_xdb_devs) {
        return findIndexes(_xdb_devs.get(), str, offset, max_count);
    }
    return IndexIds();
}


IndexIds IndexDbFacade::getEventsIndexes(const std::string& str, size_t offset, size_t max_count) {
    if (_xdb_devs) {
        return findIndexes(_xdb_evs.get(), str, offset, max_count);
    }
    return IndexIds();
}
