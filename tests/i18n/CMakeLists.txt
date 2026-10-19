cmake_minimum_required(VERSION 3.20)
project(WingHexI18nTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    find_package(Qt6 REQUIRED COMPONENTS CorePrivate)
endif()
add_executable(language_probe
    language_probe.cpp
    ../../src/class/settings/languagemanager.cpp
    builtin.qrc)
target_include_directories(language_probe PRIVATE stubs ../../src/class/settings)
target_compile_definitions(language_probe PRIVATE LANG_PAK_NAME="winghex_tr.pak")
target_link_libraries(language_probe PRIVATE Qt6::Core Qt6::CorePrivate Qt6::Gui Qt6::Widgets)
find_package(Qt6 REQUIRED COMPONENTS UiTools LinguistTools)
add_executable(ui_probe ui_probe.cpp)
target_link_libraries(ui_probe PRIVATE Qt6::Widgets Qt6::UiTools)

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qt-tools.json"
    CONTENT "{\"lrelease\":\"$<TARGET_FILE:Qt6::lrelease>\",\"rcc\":\"$<TARGET_FILE:Qt6::rcc>\"}")
