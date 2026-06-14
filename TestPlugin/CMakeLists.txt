cmake_minimum_required(VERSION 3.16)

project(TestPlugin)

# Test mode, please configure the main program directory to facilitate debugging

# 测试模式，启用请配置主程序目录，方便调试

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
option(WINGHEX_TEST_MODE TRUE)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/TranslationUtils.cmake)

add_wing_plugin(
    NAME
    TestPlugin

    TYPE
    PLUGIN

    SOURCES
    testplugin.h
    testplugin.cpp
    TestPlugin.json
    testform.h
    testform.cpp
    testform.ui
    readertestform.h
    readertestform.cpp
    readertestform.ui
    qchecklist.h
    qchecklist.cpp
    ctltestform.h
    ctltestform.cpp
    ctltestform.ui
    testsettingpage.h
    testsettingpage.cpp
    testwingeditorviewwidget.h
    testwingeditorviewwidget.cpp

    RESOURCES
    resources.qrc

    TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/lang/TestPlugin_zh_CN.ts"

    TEST_MODE
    ${WINGHEX_TEST_MODE}

    TRANSLATION_UTILS_ALREADY_INCLUDED
    TRUE

    TRANSLATION_OPTIONS
    -no-obsolete
)
