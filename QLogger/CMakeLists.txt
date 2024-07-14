cmake_minimum_required(VERSION 3.10)

project(QLoggerLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/consoleoutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonoutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/output.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qlogger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qloggerlib_global.h
    ${CMAKE_CURRENT_SOURCE_DIR}/signaloutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/textoutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/xmloutput.h)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/consoleoutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonoutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qlogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signaloutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textoutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xmloutput.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(QLoggerLib STATIC ${SOURCES} ${HEADERS})

target_link_libraries(QLoggerLib PRIVATE Qt${QT_VERSION_MAJOR}::Core)
