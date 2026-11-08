cmake_minimum_required(VERSION 3.20)
project(SettingsKeyTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD
    17
    CACHE STRING "C++ standard"
)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.6.2 REQUIRED COMPONENTS Widgets Test)
add_executable(settings_key_tests tst_settings.cpp)
target_include_directories(settings_key_tests PRIVATE ../../src)
target_link_libraries(settings_key_tests PRIVATE Qt6::Widgets Qt6::Test)
target_compile_definitions(
    settings_key_tests PRIVATE QT_RESTRICTED_CAST_FROM_ASCII
                               QT_NO_CAST_TO_ASCII
)
enable_testing()
add_test(NAME settings_key_tests COMMAND settings_key_tests)
