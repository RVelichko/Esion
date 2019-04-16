#ifndef BOOST_STATIC_LINK
#   define BOOST_TEST_DYN_LINK
#endif // BOOST_STATIC_LINK

#define BOOST_TEST_MODULE ServerComponents
#define BOOST_AUTO_TEST_MAIN

#include <string>
#include <iostream>
#include <ctime>

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "Log.hpp"
#include "json.hpp"
#include "DbFacade.hpp"

namespace test {

typedef server::DbFacade DbFacade; 
typedef nlohmann::json Json;
typedef mongo::BSONObj BsonObj;


static char DEFAULT_DB_ADDRESS[]  = "94.127.68.132";
static char DEFAULT_DB_NAME[]     = "devices";
static char DEFAULT_DB_LOGIN[]    = "esion";
static char DEFAULT_DB_PASSWORD[] = "esionpassword";

//static char TEST_DEVICE_ID[] = "12345678";
static char TEST_USER[] = "TEST_RECORDS";


Json GetTestDevice(const std::string& dev_id, time_t time, const std::string& desc = "test description") {
    Json dev = {
        {"id", dev_id},
        {"power_type", "4AA, [6V]"},
        {"power", "5.9999"},
        {"user", TEST_USER},
        {"time", *((int64_t*)&time)},
        {"counters", {
            {
                {"count", 100},
                {"type", "test type1"},
                {"unit", "liter"},
                {"units_count", 1},
                {"max_value", 1000000000},
                {"serial_num", 001},
                {"desc", "test description 1"}
            },
            {
                {"count", 50},
                {"type", "test type2"},
                {"unit", "liter"},
                {"units_count", 1},
                {"max_value", 1000000000},
                {"serial_num", 002},
                {"desc", "test description 2"}
            },
            {{"type", "none"}},
            {{"type", "none"}}
        }},
        {"desc", desc}
    };
    return dev;
}


BOOST_AUTO_TEST_CASE(TestDbFacade) {
    LOG_TO_STDOUT;

    auto t = time(nullptr);
    auto dev_id = std::to_string(t);
    auto jdev = GetTestDevice(dev_id, t);
    //auto jdev = GetTestDevice("1554910784");
    LOG(DEBUG) << jdev.dump();
    auto bdev = DbFacade::toBson(jdev);
    jdev = DbFacade::toJson(bdev);
    LOG(DEBUG) << jdev.dump();
    
    DbFacade dbf;
    dbf.connect(DEFAULT_DB_ADDRESS, DEFAULT_DB_NAME, DEFAULT_DB_LOGIN, DEFAULT_DB_PASSWORD);
    dbf.insertDevice(jdev);
    LOG(DEBUG) << "######################################";
    size_t num = 0;
    for (auto jdev : dbf.getDevices(10, 2)) {
        LOG(DEBUG) << "[" << ++num << "] " << jdev;
    } 
    dbf.insertDevice(GetTestDevice(dev_id, time(nullptr), "UPDATED DEVICE"));
    LOG(DEBUG) << "[GET] " << dbf.getDevice("1554910784");
    dbf.removeDevice("1554910784");
    LOG(DEBUG) << "######################################";
    num = 0;
    for (auto jdev : dbf.getDevices()) {
        LOG(DEBUG) << "[" << ++num << "] " << jdev;
    } 
    dbf.disconnect();
    //BOOST_CHECK_NO_THROW((i == old_num));
    //BOOST_REQUIRE(TEST_MAX_NUMBER_1 == swe.getNumber());
}
} // test
