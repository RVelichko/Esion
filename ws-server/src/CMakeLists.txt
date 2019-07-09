# Сборка приложения.
set(LIB_BASE_UTILS base_utils)
add_library(${LIB_BASE_UTILS} STATIC
    DbFacade.cpp
    IndexDbFacade.cpp
    BaseWorker.cpp
    )

set(LIB_ESION_SERVER esion_server)
add_library(${LIB_ESION_SERVER} STATIC
    GeoRequester.cpp
    DevicePeerWorker.cpp
    ReportGenerator.cpp
    OperatorCommands.cpp
    OperatorPeerWorker.cpp
    )
target_link_libraries(${LIB_ESION_SERVER}
    ${LIB_BASE_UTILS}
    ${Boost_REGEX_LIBRARY})

set(APP_ESION_SERVER esion-server)
add_executable(${APP_ESION_SERVER} ws_main.cpp)
target_link_libraries(${APP_ESION_SERVER}
    ${LIB_BASE_UTILS}
    ${LIB_ESION_SERVER}
    log
    sdispathcer
    patterns
    websocket_server
    ssl
    crypto
    ${CURL_LIBRARIES}
    ${XAPIAN_LIBRARIES}
    ${MongoClient_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )

set(LIB_INDEX_SERVER index_server)
add_library(${LIB_INDEX_SERVER} STATIC
    SearchIndexCommands.cpp
    SearchIndexPeerWorker.cpp
    )
target_link_libraries(${LIB_INDEX_SERVER} ${LIB_BASE_UTILS})

set(APP_INDEX_SERVER esion-index-server)
add_executable(${APP_INDEX_SERVER} si_main.cpp)
target_link_libraries(${APP_INDEX_SERVER}
    ${LIB_BASE_UTILS}
    ${LIB_INDEX_SERVER}
    log
    sdispathcer
    patterns
    websocket_server
    ssl
    crypto
    ${XAPIAN_LIBRARIES}
    ${MongoClient_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )


set(APP_DB_DATA_GEN db-data-generator)
add_executable(${APP_DB_DATA_GEN}
    GeoRequester.cpp
    gen_main.cpp
    )
target_link_libraries(${APP_DB_DATA_GEN}
    ${LIB_BASE_UTILS}
    log
    ${MongoClient_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CURL_LIBRARIES}
    )
