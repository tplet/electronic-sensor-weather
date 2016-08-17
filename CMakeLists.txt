cmake_minimum_required(VERSION 3.3)
# Some cutoms functions
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions.txt)
# Parameters
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/parameters.txt)


# Project name
set(NAME weather_sensor)
set(EXECUTABLE ${NAME})


# Project
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
project(${NAME})

# Files
file(GLOB_RECURSE FILES_CPP "src/*.cpp")
file(GLOB_RECURSE FILES_H "src/*.h")


## Dependencies:
# common-arduino (library)
add_subdirectory(./vendors/common-arduino/)
# DHT (library)
add_subdirectory(./vendors/DHT)

# Hardware libraries
#include_directories(${ARDUINO_PLATFORM_PATH}/ARDUINO_PLATFORM/libraries)

generate_arduino_firmware(
    ${CMAKE_PROJECT_NAME}
    SKETCH main.ino
    SRCS ${FILES_CPP}
    HDRS ${FILES_H}
    BOARD ${BOARD}
    PORT ${PORT}
    ARDLIBS Wire SPI LiquidCrystal
    LIBS common-arduino DHT
#   NO_AUTOLIBS
)

# Add src directory to "include path"
target_include_directories(${EXECUTABLE} PUBLIC ./src)
