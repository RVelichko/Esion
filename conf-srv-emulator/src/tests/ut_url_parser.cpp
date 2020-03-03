#ifndef BOOST_STATIC_LINK
#   define BOOST_TEST_DYN_LINK
#endif // BOOST_STATIC_LINK

#define BOOST_TEST_MODULE ServerComponents
#define BOOST_AUTO_TEST_MAIN

#include <string>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "Log.hpp"

namespace test {


typedef std::string String;

//
//struct Url {
//    Url(const String& url) {
//        String s(url);
//        int pr = s.indexOf("://");
//        if (pr not_eq -1) {
//            protocol = s.substring(0, pr);
//            s = s.substring(pr + 3);
//        }
//        int pp = s.indexOf(":");
//        int pt = s.indexOf("/");
//        int pq = s.indexOf("?");
//        if (pp not_eq -1) {
//            String sp = s.substring(0, pp);
//            port = static_cast<uint16_t>(sp.toInt());
//            if (pt not_eq -1) {
//                host = s.substring(pp + 1, pt);
//                if (pq not_eq -1) {
//                    path = s.substring(pt, pq);
//                    query = s.substring(pq);
//                } else {
//                    path = s.substring(pt);
//                }
//            } else {
//                host = s.substring(pp + 1);
//            }
//        }
//        if (pt not_eq -1) {
//            host = s.substring(0, pt);
//            if (pq not_eq -1) {
//                path = s.substring(pt, pq);
//                query = s.substring(pq);
//            } else {
//                path = s.substring(pt);
//            }
//        } else {
//            host = s;
//        }
//    }
//
//    String protocol;
//    String host;
//    uint16_t port;
//    String path;
//    String query;
//};
//

BOOST_AUTO_TEST_CASE(TestUrlParser) {
    LOG_TO_STDOUT;

    //BOOST_CHECK_NO_THROW((i == old_num));
    //BOOST_REQUIRE(TEST_MAX_NUMBER_1 == swe.getNumber());
}
} // test
