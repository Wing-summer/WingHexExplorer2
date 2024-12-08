cmake_minimum_required(VERSION 3.16)

project(TestPlugin)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Test mode, please configure the main program directory to facilitate debugging

# 测试模式，启用请配置主程序目录，方便调试

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
option(TEST_MODE TRUE)

# Set the location where the SDK is stored. Only iwingplugin.h and settingpage.h
# are required.

# 设置存放 SDK 的位置，只须 iwingplugin.h 和 settingpage.h 存在

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
set(WINGHEX_SDK "${CMAKE_CURRENT_SOURCE_DIR}/../src/plugin")

set(PLUGIN_INTERFACE_FOUND FALSE)
set(PLUGIN_SETPAGE_FOUND FALSE)
if(EXISTS "${WINGHEX_SDK}/iwingplugin.h")
    set(PLUGIN_INTERFACE_FOUND TRUE)
endif()
if(EXISTS "${WINGHEX_SDK}/settingpage.h")
    set(PLUGIN_SETPAGE_FOUND TRUE)
endif()
if(PLUGIN_INTERFACE_FOUND AND PLUGIN_SETPAGE_FOUND)
    message(STATUS "${WINGHEX_SDK} is valid SDK path")
else()
    message(FATAL_ERROR "Invalid SDK path!")
endif()

set(WINGHEX_SDK_HEADER "${WINGHEX_SDK}/iwingplugin.h"
                       "${WINGHEX_SDK}/settingpage.h")
include_directories(${WINGHEX_SDK})

# For Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Widgets LinguistTools
    REQUIRED)

set(TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/lang/TestPlugin_zh_CN.ts")

set(TRANSLATION_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(${QT_VERSION_MAJOR} EQUAL 5)
    qt5_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES} OPTIONS
                           -no-obsolete)
elseif(${QT_VERSION_MAJOR} EQUAL 6)
    qt6_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES} OPTIONS
                           -no-obsolete)
else()
    message(FATAL_ERROR "Unsupported QT version")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/TranslationUtils.cmake)
add_plugin_translations_resource(QM_RES TestPlugin ${QM_FILES})

add_library(
    TestPlugin SHARED
    ${WINGHEX_SDK_HEADER}
    testplugin.h
    testplugin.cpp
    TestPlugin.json
    testform.h
    testform.cpp
    testform.ui
    ${QM_FILES}
    ${QM_RES}
    resources.qrc)

set_target_properties(TestPlugin PROPERTIES SUFFIX ".wingplg")

if(TEST_MODE)
    # If you want to be able to debug easily every time you compile, please set
    # this variable. Because this test plugin is a subproject of the main
    # project, use CMAKE_BINARY_DIR

    # 如果你想每次编译都能够方便调试的话，请设置这个变量。因为这个测试插件是和主项目子项目，所以用 CMAKE_BINARY_DIR

    # vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    set(WINGHEX_PATH "${CMAKE_BINARY_DIR}")

    set(WINGHEX_PLUGIN_PATH "${WINGHEX_PATH}/plugin")
    add_custom_command(
        TARGET TestPlugin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WINGHEX_PLUGIN_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:TestPlugin>
                ${WINGHEX_PLUGIN_PATH})
endif()

target_link_libraries(TestPlugin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
