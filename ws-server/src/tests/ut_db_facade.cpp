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
static char TEST_USER[] = "test user information";


Json GetTestDevice(const std::string& dev_id, const std::string& desc = "test description") {
    Json dev = {
        {"id", dev_id},
        {"power_type", "6V"},
        {"power", "5.9999"},
        {"user", TEST_USER},
        {"counters", {
            {
                {"count", 100},
                {"type", "test type1"},
                {"unit", "liter"},
                {"units_count", 1},
                {"max_value", 1000000000},
                {"description", "test description 1"}
            },
            {
                {"count", 50},
                {"type", "test type2"},
                {"unit", "liter"},
                {"units_count", 1},
                {"max_value", 1000000000},
                {"description", "test description 2"}
            },
            {{"type", "none"}},
            {{"type", "none"}},
        }},
        {"description", desc}
    };
    return dev;
}


BOOST_AUTO_TEST_CASE(TestDbFacade) {
    LOG_TO_STDOUT;
    
    //auto jdev = GetTestDevice(std::to_string(time(nullptr)));
    auto jdev = GetTestDevice("1554910784");
    LOG(DEBUG) << jdev.dump();
    auto bdev = DbFacade::toBson(jdev);
    jdev = DbFacade::toJson(bdev);
    LOG(DEBUG) << jdev.dump();
    
    DbFacade dbf;
    dbf.connect(DEFAULT_DB_ADDRESS, DEFAULT_DB_NAME, DEFAULT_DB_LOGIN, DEFAULT_DB_PASSWORD);
    dbf.addDevice(jdev);
    dbf.updateDevice(GetTestDevice("1554910784", "UPDATED DEVICE"));
    LOG(DEBUG) << dbf.getDevice("1554910784");
    dbf.deleteDevice(jdev);
    dbf.disconnect();
    //BOOST_CHECK_NO_THROW((i == old_num));
    //BOOST_REQUIRE(TEST_MAX_NUMBER_1 == swe.getNumber());
}
} // test
