cmake_minimum_required(VERSION 2.6.0)

message("\nConfigure project ESION...")

project(ESION)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_SYSTEM_NAME "Generik")
set(CMAKE_C_COMPILER "avr-gcc")
set(CMAKE_CXX_COMPILER "avr-g++")

set(CSTANDARD "-std=gnu11")
set(CDEBUG "-gstabs")
set(CWARN "-Wall -Wstrict-prototypes")
set(CTUNING "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")
set(COPT "-Os")
set(CMCU "-mmcu=attiny84")
set(CDEFS "-DF_CPU=8000000UL")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CFLAGS "${CMCU} ${CDEBUG} ${CDEFS} ${CINCS} ${COPT} ${CWARN} ${CSTANDARD} ${CEXTRA}")
set(CXXFLAGS "${CMCU} ${CDEFS} ${CINCS} ${COPT}")

set(CMAKE_C_FLAGS  ${CFLAGS})
set(CMAKE_CXX_FLAGS ${CXXFLAGS})

set(ROOT_DIR   ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SRC_DIR    ${ROOT_DIR}/src)
set(DATA_DIR   ${ROOT_DIR}/data)
set(LIB_DIR    ${ROOT_DIR}/avr/libraries)

#set(TINY_DIR    ${ROOT_DIR}/avr/cores/tinymodern)
set(TINY_DIR    ${ROOT_DIR}/avr/cores/tiny)
set(VARIANT_DIR ${ROOT_DIR}/avr/variants/tinyX4)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_DIR}/lib)

set(FLASHER UM232H)
set(CHIP attiny84)

macro(avr_create_hex NAME_TARGET)
    add_custom_command(
        OUTPUT "${NAME_TARGET}.hex" POST_BUILD
        COMMAND avr-objcopy ARGS -O ihex -R.eeprom ${NAME_TARGET} ${NAME_TARGET}.hex
        DEPENDS ${NAME_TARGET}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    add_custom_target("${NAME_TARGET}_hex" DEPENDS ${NAME_TARGET}.hex)
endmacro(avr_create_hex)

macro(avr_programm NAME_TARGET)
    add_custom_target(
        "flash"
        COMMAND avrdude ARGS -c ${FLASHER} -p ${CHIP} -b 57600 -U flash:w:${NAME_TARGET}.hex:a
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    add_dependencies("flash" "${NAME_TARGET}_hex")
endmacro(avr_programm)


add_custom_target(
    "erase"
    COMMAND avrdude ARGS -c ${FLASHER} -p ${CHIP} -e
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

add_custom_target(
    "read_fuses"
    COMMAND avrdude ARGS -c ${FLASHER} -p ${CHIP} -U hfuse:r:hfuse.txt:b -U lfuse:r:lfuse.txt:b
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

add_custom_target(
    "read"
    COMMAND avrdude ARGS -c ${FLASHER} -p ${CHIP} -U flash:r:"reading".hex:h
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

include_directories(${TINY_DIR})
include_directories(${VARIANT_DIR})
#add_subdirectory(${LIB_DIR})

include_directories(${SRC_DIR})
add_subdirectory(${SRC_DIR})
