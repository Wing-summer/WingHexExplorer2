cmake_minimum_required(VERSION 3.16)

project(WingPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(
    WingPlugin SHARED
    iwingplugin.cpp
    iwingplugin.h
    iwingpluginbase.h
    iwingpluginbase.cpp
    wingplugin_global.h
    wingcore.h
    wingplugincalls.h
    wingplugincalls.cpp
    wingplugincalls_p.h
    wingeditorviewwidget.h
    wingeditorviewwidget.cpp
    settingpage.h
    iwingdevice.h
    iwingdevice.cpp
    settingpage.cpp
    iwingpluginbasecalls.h
    iwingpluginbasecalls.cpp
    iwingplugincalls.h
    iwingplugincalls.cpp
    wingplugincallconvertor.h
    wingplugincallconvertor.cpp
    wingpluginwidget.h
    wingpluginwidget.cpp)

target_link_libraries(WingPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(WingPlugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_compile_definitions(WingPlugin PRIVATE WINGPLUGIN_LIBRARY)
