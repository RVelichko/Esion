set(APP_ESION_SERVER server-emulator)
add_executable(${APP_ESION_SERVER}
    http_server.cpp
    main.cpp
    )
target_link_libraries(${APP_ESION_SERVER}
    ${LIB_ESION_SERVER}
    log
    sdispathcer
    event
    ${CMAKE_THREAD_LIBS_INIT}
    )
