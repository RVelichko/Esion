# Пример добавления теста:
# add_unit_tests(<Имя теста> <библиотека 1>  ... <библиотека N>)

include(${CMAKE_DIR}/UTest.cmake)

include_directories(${UTILS_DIR})
include_directories(${SRC_DIR})

#add_unit_test(ut_url_parser log)
add_unit_test(ut_db_facade base_utils esion_server log pthread ${MongoClient_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_LIBRARY})
