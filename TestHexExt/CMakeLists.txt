cmake_minimum_required(VERSION 3.16)

project(TestHexExt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

# Test mode, please configure the main program directory to facilitate debugging

# 测试模式，启用请配置主程序目录，方便调试

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
option(TEST_MODE TRUE)

# For Qt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Widgets
    REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(TestHexExt SHARED TestHexExt.json testhexext.h testhexext.cpp)

set_target_properties(TestHexExt PROPERTIES SUFFIX ".winghexe")

if(TEST_MODE)
    # If you want to be able to debug easily every time you compile, please set
    # this variable. Because this test plugin is a subproject of the main
    # project, use CMAKE_BINARY_DIR

    # 如果你想每次编译都能够方便调试的话，请设置这个变量。因为这个测试插件是和主项目子项目，所以用 CMAKE_BINARY_DIR

    # vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    set(WINGHEX_PATH "${CMAKE_BINARY_DIR}")

    set(WINGHEX_PLUGIN_PATH "${WINGHEX_PATH}")
    add_custom_command(
        TARGET TestHexExt
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WINGHEX_PLUGIN_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:TestHexExt>
                ${WINGHEX_PLUGIN_PATH})
endif()

target_link_libraries(TestHexExt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                         WingPlugin)
target_link_libraries(TestHexExt PRIVATE Qt6::Core)
