/**
 * \brief  Генератор отладочных данных для заполнения БД.
 * \author Величко Ростислав
 * \date   07.07.2019
 */


#include <iostream>
#include <memory>
#include <tuple>
#include <ctime>
#include <functional>
#include <exception>
#include <random>

#include <boost/filesystem.hpp>

#include "DbFacade.hpp"
#include "GeoRequester.hpp"
#include "rapidcsv.h"
#include "Log.hpp"


/*!
 * \brief Генерация необходимых индексов:
 *        db.< collection >.createIndex({coll:"text", dev_id:"text", user:"text"}, {default_language:"russian"})
 *        db.< collection >.createIndex({"geo":"2dsphere"})
 */

namespace bfs = boost::filesystem;
//using namespace io;

static char DEFAULT_DB_ADDRESS[]  = "127.0.0.1";
static char DEFAULT_DB_NAME[]     = "devices";
static char DEFAULT_DB_LOGIN[]    = "esion";
static char DEFAULT_DB_PSWD[]     = "esionpassword";

static char DEFAULT_ADDRESSES_FILE[] = "addresses.csv";

static char DEFAULT_YMAP_API_KEY[] = "ad12ff63-587b-42c7-b1e1-e8b8b0913cda";


struct GlobalArgs {
    char* db_addr;        ///< параметр -a
    char* db_name;        ///< параметр -n
    char* db_login;       ///< параметр -l
    char* db_pswd;        ///< параметр -s
    char* addrs_file;     ///< параметр -f
    char* yamap_api_key;  ///< параметр -y
} __global_args;


static const char *__opt_string = "a:n:l:s:y:h?";
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void HelpMessage() {
    std::cout << "  Use:\n\t#db-data-genrator\n"
              << "  Args:\n"
              << "\t[-a]\t DB address.\t[" << DEFAULT_DB_ADDRESS << "]\n"
              << "\t[-n]\t DB name.\t[" << DEFAULT_DB_NAME << "]\n"
              << "\t[-l]\t DB login.\t[" << DEFAULT_DB_LOGIN << "]\n"
              << "\t[-s]\t DB password.\t[" << DEFAULT_DB_PSWD << "]\n"
              << "\t[-f]\t Addresses file path.\t[" << DEFAULT_ADDRESSES_FILE << "]\n"
              << "\t[-y]\t Yandex map API key.\t[" << DEFAULT_YMAP_API_KEY << "]\n"
              << "__________________________________________________________________\n\n";
    exit(EXIT_FAILURE);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef server::DbFacade DbFacade;
typedef std::shared_ptr<DbFacade> PDbFacade;
typedef server::Json Json;
typedef server::GeoLocation GeoLocation;
typedef server::GeoRequester GeoRequester;


static const time_t MAX_OLD_TIMEOUT = 5184000;

class DbDataTestGenerator {
    PDbFacade _db;
    std::string _addr_file;
    GeoRequester _geo_req;

    time_t getUniqTime(time_t t, size_t count) {
        static std::set<time_t> uniq_times;
        time_t uniqt;
        while (uniqt = t + rand() % (MAX_OLD_TIMEOUT / 100) + count, not uniq_times.insert(uniqt).second);
        return uniqt;
    }

    double randPercent() {
        return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
    }

    /*!
    * \brief Json с описание устройства: {
    *          "dev_id":"<идентификатор устройства, (уникален) >",
    *          "coll":"<адрес расположения устройства — более подробный чем для объекта>",
    *          "user":"<владелец устройства>",
    *          "geo":[ <longitude>, <latitude> ],
    *          "update_time":"<время последнего подключения к серверу>",
    *          "power_type":"<тип питания устройства>",
    *          "voltage":"<текущее напряжение батареи автономного питания>",
    *          "desc":"<некоторое описание устройтсва>",
    *          "status":"<active | not_active>",
    *          "counters":[
    *            {
    *              "type":"<none | любая не пустая строка (при none остальных полей в этой части json может не быть) >",
    *              "count":"<количество обработанных импульсов>",
    *              "unit":"<единица измерения>",
    *              "unit_type":"<количество единиц измерения на импульс>",
    *              "unit_count":"<количество кубометров>",
    *              "serial":"<серийный монер счётчика>",
    *              "verify_date":"<дата последней поверки счётчика>",
    *              "desc":"<некоторое описание, например какая вода измерятеся>",
    *            },
    *            <таких записей всего 4, первая запись для счётчика устройства №1, вторая для №2 и т.д.  >
    *          ]
    *        }
    */
    Json createDevices() {
        static std::vector<std::string> names = {
            "Rod", "Veles", "Svarog", "Ruevit", "Vishen", "Semargl", "Lada", "Lelya"
        };
        Json jdevs;
        rapidcsv::Document doc(_addr_file, rapidcsv::LabelParams(-1, -1), rapidcsv::SeparatorParams(','));
        size_t count = 0;
        for (size_t i = 0; i < doc.GetRowCount(); ++i) {
            auto row = doc.GetRow<std::string>(i);
            std::string addr;
            if (not row[1].empty() or not row[2].empty()) {
                addr = "Санкт-Петербург " + row[1] + ' ' + row[2];
            }
            std::string desc = row[0];
            if (not addr.empty()) {
                auto jgeo = _geo_req.request(addr);
                size_t num_devs = rand() % 200;
                /// Сформировать JSON устройства.
                time_t now = time(nullptr);
                time_t old = now - rand() % MAX_OLD_TIMEOUT;
                time_t old_halb = std::floor((now - old) * 0.5);
                size_t apmt = rand() % 10 + 1;
                for (size_t n = 0; n < num_devs; ++n) {
                    ++count;
                    Json counters;
                    size_t max_cms = rand() % 100 + 1;
                    time_t timeout = now - old;
                    for (size_t i = 0; i < 4; ++i) {
                        if (randPercent() < 0.5) {
                            Json count = {
                                {"type", "none"}
                            };
                            counters.push_back(count);
                        } else {
                            size_t sum_cnt = 0;
                            size_t unit_count = (randPercent() < 0.7 ? 1 : 10);
                            Json jcubic_meters;
                            double sum_cm = 0.0;
                            for(size_t cm = 0; cm < max_cms; ++cm) {
                                size_t cnt = rand() % 50;
                                sum_cnt += cnt;
                                double dcm = static_cast<double>(cnt * unit_count) / 1000.0;
                                sum_cm += dcm;
                                Json jcm = {
                                    {"t", old + ((timeout / max_cms) * cm)},
                                    {"cm", sum_cm}
                                };
                                jcubic_meters.push_back(jcm);
                            }
                            time_t verify_date = old + (rand() % old_halb);
                            Json count = {
                                {"type", "Debug type"},
                                {"cubic_meter", jcubic_meters},
                                {"count", sum_cnt},
                                {"unit", "Литр"},
                                {"unit_type", "Импульс"},
                                {"unit_count", unit_count},
                                {"serial", rand() % 500},
                                {"verify_date", verify_date},
                                {"desc", "Отладочный канал N " + std::to_string(i + 1)}
                            };
                            counters.push_back(count);
                        }
                    }
                    bool is_lion = randPercent() < 0.5;
                    double voltage = 0.0;
                    size_t volt_perc = 0;
                    if (is_lion) {
                        voltage = 3.2 + 0.4 * randPercent();
                        volt_perc = floor(voltage * 100.0 / 3.8 + 0.05);
                    } else {
                        voltage = 5.3 + 0.6 * randPercent();
                        volt_perc = floor(voltage * 100.0 / 6 + 0.05);
                    }
                    Json jdev = {
                        {"coll_id", "Debug Debug"},
                        {"dev_id", std::to_string(getUniqTime(old, count))},
                        {"coll", addr},
                        {"apmt", apmt + n},
                        {"user", (names[rand() % names.size()] + " " + names[rand() % names.size()])},
                        {"geo", jgeo["point"]},
                        {"start_time", old},
                        {"update_time", now},
                        {"power_type", is_lion ? "LiOn 3.8V" : "4AA 6V"},
                        {"voltage", voltage},
                        {"voltage_perc", volt_perc},
                        {"status", (randPercent() < 0.9 ? "active" : "not_active" )},
                        {"counters", counters},
                        {"desc", desc + ": N " + std::to_string(count)}
                    };
                    LOG(TRACE) << jdev.dump() << " | " << n << " | " << count;
                    jdevs.push_back(jdev);
                }
            }
        }
        return jdevs;
    }

    Json createEvents(const Json& jdevs) {
        Json jevs;
        size_t count = 0;
        for (auto jdev : jdevs) {
            if (randPercent() < 0.2) {
                size_t num = rand() % 2 + 1;
                for (size_t i = 0; i < num; ++i) {
                    ++count;
                    static std::vector<std::string> prirs = {"Низкий", "Средний", "Высокий", "Критический"};
                    size_t prirs_id = (rand() % prirs.size());
                    Json jev = {
                        {"ev_id", std::to_string(time(nullptr) + count)},
                        {"dev_id", jdev["dev_id"]},
                        {"coll", jdev["coll"]},
                        {"apmt", jdev["apmt"]},
                        {"user", jdev["user"]},
                        {"geo", jdev["geo"]},
                        {"time", (jdev["start_time"].get<size_t>() + rand() % 1000)},
                        {"priority_id", prirs_id + 1},
                        {"priority", prirs[prirs_id]},
                        {"desc", "Сгенерированное отладочное событие N " + std::to_string(count)},
                        {"coll_id", "Debug Debug"}
                    };
                    LOG(TRACE) << jev.dump() << " | " << i << " | " << num << " | " << count;
                    jevs.push_back(jev);
                }
            }
        }
        return jevs;
    }

public:
    DbDataTestGenerator(const PDbFacade& db, const std::string& addr_file, const std::string& ymap_api_key)
        : _db(db)
        , _addr_file(addr_file)
        , _geo_req(ymap_api_key) {
        srand(time(nullptr));
        try {
            if (bfs::exists(bfs::path(_addr_file))) {
                LOG(DEBUG) << "_addr_file";
                auto jdevs = createDevices();
                for (auto jdev : jdevs) {
                    //LOG(TRACE) << jdev.dump();
                    _db->insertDevice(jdev);
                }
                auto jevs = createEvents(jdevs);
                for (auto jev : jevs) {
                    //LOG(TRACE) << jev.dump();
                    _db->insertEvent(jev);
                }
            } else {
                LOG(ERROR) << "Can`t find file \"" << _addr_file << "\"";
            }
        } catch (const std::exception& e) {
            LOG(FATAL) << e.what();
        }
    }

    ~DbDataTestGenerator() {
        LOG(DEBUG);
    }
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc_, char **argv_) {
    /// Инициализация globalArgs до начала работы с ней.
    __global_args.db_addr        = DEFAULT_DB_ADDRESS;
    __global_args.db_name        = DEFAULT_DB_NAME;
    __global_args.db_login       = DEFAULT_DB_LOGIN;
    __global_args.db_pswd        = DEFAULT_DB_PSWD;
    __global_args.addrs_file     = DEFAULT_ADDRESSES_FILE;
    __global_args.yamap_api_key  = DEFAULT_YMAP_API_KEY;

    /// Обработка входных опций.
    int opt = getopt(argc_, argv_, __opt_string);
    while(opt not_eq -1) {
        switch(opt) {
            case 'a':
                __global_args.db_addr = optarg;
                break;
            case 'n':
                __global_args.db_name = optarg;
                break;
            case 'l':
                __global_args.db_login = optarg;
                break;
            case 's':
                __global_args.db_pswd = optarg;
                break;
            case 'f':
                __global_args.addrs_file = optarg;
                break;
            case 'y':
                __global_args.yamap_api_key = optarg;
                break;

            case 'h':
            case '?':
                HelpMessage();
                break;

            default: break;
        }
        opt = getopt(argc_, argv_, __opt_string);
    }
    LOG_TO_STDOUT;
    /// Доступ к БД.
    PDbFacade db = std::make_shared<DbFacade>();
    if (db->connect(__global_args.db_addr, __global_args.db_name, __global_args.db_login, __global_args.db_pswd)) {
        DbDataTestGenerator gen(db, __global_args.addrs_file, __global_args.yamap_api_key);
        return EXIT_SUCCESS;
    }
    LOG(ERROR) << "Check Database parameters.";
    return EXIT_FAILURE;
}
