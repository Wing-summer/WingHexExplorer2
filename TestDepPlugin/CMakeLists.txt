cmake_minimum_required(VERSION 3.16)

project(TestDepPlugin LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

option(WINGHEX_TEST_MODE TRUE)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(TestDepPlugin SHARED testdepplugin.cpp testdepplugin.h
                                 TestDepPlugin.json)

target_link_libraries(TestDepPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                            WingPlugin)

target_compile_definitions(TestDepPlugin PRIVATE TESTDEPPLUGIN_LIBRARY)

set_target_properties(TestDepPlugin PROPERTIES SUFFIX ".wingplg")

if(WINGHEX_TEST_MODE)
    set(WINGHEX_PATH "${CMAKE_BINARY_DIR}")

    set(WINGHEX_PLUGIN_PATH "${WINGHEX_PATH}/plugin")
    add_custom_command(
        TARGET TestDepPlugin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WINGHEX_PLUGIN_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:TestDepPlugin> ${WINGHEX_PLUGIN_PATH})
endif()
