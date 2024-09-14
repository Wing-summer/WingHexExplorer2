cmake_minimum_required(VERSION 3.5)

project(
    QConsoleWidget
    VERSION 2.14.1
    LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(
    QConsoleWidget STATIC src/QConsoleIODevice.cpp src/QConsoleIODevice.h
                          src/QConsoleWidget.cpp src/QConsoleWidget.h)

target_link_libraries(QConsoleWidget PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
