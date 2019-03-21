# Сборка приложения.
set(APP_ESION_SERVER esion-server)
add_executable(${APP_ESION_SERVER}
    ws_server.cpp
    main.cpp
    )
target_link_libraries(${APP_ESION_SERVER}
    log
    sdispathcer
    websocket_server
    ssl
    crypto
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )

