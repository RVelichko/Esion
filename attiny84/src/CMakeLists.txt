#set(AVR "avr")
#set(AVR_SOURCE
#)
#add_library(${AVR} ${AVR_SOURCE})


set(ESION "esion")
set(SOURCE
    #../avr/cores/tinymodern/pins_arduino.c
    #../avr/cores/tinymodern/WInterrupts.c
    #../avr/cores/tinymodern/wiring_analog.c
    #../avr/cores/tinymodern/wiring.c
    #../avr/cores/tinymodern/wiring_digital.c
    #../avr/cores/tinymodern/wiring_pulse.c
    #../avr/cores/tinymodern/wiring_shift.c
    #../avr/cores/tinymodern/abi.cpp
    #../avr/cores/tinymodern/HardwareSerial.cpp
    #../avr/cores/tinymodern/main.cpp
    #../avr/cores/tinymodern/new.cpp
    #../avr/cores/tinymodern/Print.cpp
    #../avr/cores/tinymodern/Stream.cpp
    #../avr/cores/tinymodern/Tone.cpp
    #../avr/cores/tinymodern/WMath.cpp
    #../avr/cores/tinymodern/WString.cpp
    ../avr/cores/tiny/abi.cpp
    ../avr/cores/tiny/HardwareSerial.cpp
    ../avr/cores/tiny/main.cpp
    ../avr/cores/tiny/new.cpp
    ../avr/cores/tiny/Print.cpp
    ../avr/cores/tiny/Stream.cpp
    ../avr/cores/tiny/TinySoftwareSerial.cpp
    ../avr/cores/tiny/Tone.cpp
    ../avr/cores/tiny/WMath.cpp
    ../avr/cores/tiny/WString.cpp
    ../avr/cores/tiny/WInterrupts.c
    ../avr/cores/tiny/wiring_analog.c
    ../avr/cores/tiny/wiring.c
    ../avr/cores/tiny/wiring_digital.c
    ../avr/cores/tiny/wiring_pulse.c
    ../avr/cores/tiny/wiring_shift.c
    main.cpp
    )
add_executable(${ESION} ${SOURCE})
#target_link_libraries(${ESION} ${AVR})

avr_create_hex(${ESION})
avr_programm(${ESION})
