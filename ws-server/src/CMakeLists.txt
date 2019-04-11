# Сборка приложения.
include_directories(${MongoClient_INCLUDE_DIR})

set(LIB_ESION_SERVER esion_server)
add_library(${LIB_ESION_SERVER} STATIC
    DbFacade.cpp
    BaseWorker.cpp
    DevicePeerWorker.cpp
    OperatorPeerWorker.cpp
    )
set(APP_ESION_SERVER esion-server)
add_executable(${APP_ESION_SERVER} main.cpp)
target_link_libraries(${APP_ESION_SERVER}
    ${LIB_ESION_SERVER}
    log
    sdispathcer
    websocket_server
    ssl
    crypto
    ${MongoClient_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )

