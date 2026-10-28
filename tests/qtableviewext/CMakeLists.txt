cmake_minimum_required(VERSION 3.20)
project(WingTableViewTests LANGUAGES CXX)
option(WINGHEX_FORCE_CPP17 "Use the C++17 fallback" OFF)
if(WINGHEX_FORCE_CPP17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.6.2 REQUIRED COMPONENTS Widgets Concurrent Test)
get_filename_component(WING_SOURCE_ROOT "../.." ABSOLUTE)
add_executable(
    wing_table_tests
    tst_qtableviewext.cpp
    "${WING_SOURCE_ROOT}/src/control/searchabletextbrowser.cpp"
    "${WING_SOURCE_ROOT}/src/control/searchabletextbrowser.h"
    "${WING_SOURCE_ROOT}/src/control/searchbar.cpp"
    "${WING_SOURCE_ROOT}/src/control/searchbar.h"
    "${WING_SOURCE_ROOT}/src/control/searchabletreewidget.cpp"
    "${WING_SOURCE_ROOT}/src/control/searchabletreewidget.h"
    "${WING_SOURCE_ROOT}/src/control/delegate/wrappingitemdelegate.cpp"
    "${WING_SOURCE_ROOT}/src/control/delegate/wrappingitemdelegate.h"
    icons.qrc
    "${WING_SOURCE_ROOT}/src/control/delegate/richtextitemdelegate.cpp"
    "${WING_SOURCE_ROOT}/src/control/delegate/richtextitemdelegate.h"
    "${WING_SOURCE_ROOT}/src/control/tablecolumnwidthcontroller.cpp"
    "${WING_SOURCE_ROOT}/src/control/tablecolumnwidthcontroller.h"
    "${WING_SOURCE_ROOT}/src/control/qtableviewext.cpp"
    "${WING_SOURCE_ROOT}/src/control/qtableviewext.h"
    "${WING_SOURCE_ROOT}/src/model/asynctableproxymodel.cpp"
    "${WING_SOURCE_ROOT}/src/model/asynctableproxymodel.h"
)
target_include_directories(wing_table_tests PRIVATE "${WING_SOURCE_ROOT}/src")
target_link_libraries(
    wing_table_tests PRIVATE Qt6::Widgets Qt6::Concurrent Qt6::Test
)
target_compile_definitions(
    wing_table_tests PRIVATE QT_NO_EMIT QT_NO_CONTEXTLESS_CONNECT
)
enable_testing()
add_test(NAME wing_table_tests COMMAND wing_table_tests)
set_tests_properties(
    wing_table_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
