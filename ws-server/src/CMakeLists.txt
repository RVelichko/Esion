# Сборка приложения.
set(LIB_ESION_SERVER esion_server)
add_library(${LIB_ESION_SERVER} STATIC
    DbFacade.cpp
    BaseWorker.cpp
    GeoRequester.cpp
    DevicePeerWorker.cpp
    ReportGenerator.cpp
    OperatorCommands.cpp
    OperatorPeerWorker.cpp
    )

set(APP_ESION_SERVER esion-server)
add_executable(${APP_ESION_SERVER} ws_main.cpp)
target_link_libraries(${APP_ESION_SERVER}
    ${LIB_ESION_SERVER}
    log
    sdispathcer
    patterns
    websocket_server
    ssl
    crypto
    ${CURL_LIBRARIES}
    ${MongoClient_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )


set(APP_DB_DATA_GEN db-data-generator)
add_executable(${APP_DB_DATA_GEN}
    DbFacade.cpp
    GeoRequester.cpp
    gen_main.cpp
    )
target_link_libraries(${APP_DB_DATA_GEN}
    log
    ${MongoClient_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${CURL_LIBRARIES}
    )
