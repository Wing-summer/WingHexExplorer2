cmake_minimum_required(VERSION 3.16)

project(TestDepPlugin LANGUAGES CXX)

option(WINGHEX_TEST_MODE TRUE)

add_wing_plugin(
    NAME
    TestDepPlugin

    TYPE
    PLUGIN

    SOURCES
    testdepplugin.cpp
    testdepplugin.h
    TestDepPlugin.json

    TEST_MODE
    ${WINGHEX_TEST_MODE}
)
