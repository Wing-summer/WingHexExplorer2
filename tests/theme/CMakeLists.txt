cmake_minimum_required(VERSION 3.20)
project(WingThemeTests LANGUAGES CXX)
option(WINGHEX_FORCE_CPP17 "Use the C++17 fallback" OFF)
if(WINGHEX_FORCE_CPP17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(
    Qt6 6.6.2 REQUIRED
    COMPONENTS Widgets
               Svg
               Xml
               Test
)
get_filename_component(WING_SOURCE_ROOT "../.." ABSOLUTE)
add_subdirectory("${WING_SOURCE_ROOT}/WingPlugin" WingPlugin)
add_executable(
    wing_theme_tests
    tst_thememanager.cpp
    "${WING_SOURCE_ROOT}/TestPlugin/testform.cpp"
    "${WING_SOURCE_ROOT}/TestPlugin/testform.h"
    "${WING_SOURCE_ROOT}/TestPlugin/testform.ui"
    "${WING_SOURCE_ROOT}/TestPlugin/readertestform.cpp"
    "${WING_SOURCE_ROOT}/TestPlugin/readertestform.h"
    "${WING_SOURCE_ROOT}/TestPlugin/readertestform.ui"
    "${WING_SOURCE_ROOT}/TestPlugin/ctltestform.cpp"
    "${WING_SOURCE_ROOT}/TestPlugin/ctltestform.h"
    "${WING_SOURCE_ROOT}/TestPlugin/ctltestform.ui"
    "${WING_SOURCE_ROOT}/TestPlugin/qchecklist.cpp"
    "${WING_SOURCE_ROOT}/TestPlugin/qchecklist.h"
    "${WING_SOURCE_ROOT}/src/dialog/splashdialog.cpp"
    "${WING_SOURCE_ROOT}/src/dialog/splashdialog.h"
    "${WING_SOURCE_ROOT}/src/dialog/splashdialog.ui"
    "${WING_SOURCE_ROOT}/src/class/settings/thememanager.cpp"
    "${WING_SOURCE_ROOT}/src/class/settings/thememanager.h"
    "${WING_SOURCE_ROOT}/resources.qrc"
    "${WING_SOURCE_ROOT}/theme/breeze.qrc"
)
set_target_properties(wing_theme_tests PROPERTIES AUTOUIC ON)
target_include_directories(
    wing_theme_tests PRIVATE "${WING_SOURCE_ROOT}/src" "${WING_SOURCE_ROOT}"
                             "${WING_SOURCE_ROOT}/TestPlugin"
)
target_link_libraries(
    wing_theme_tests
    PRIVATE WingPlugin
            Qt6::Widgets
            Qt6::Svg
            Qt6::Xml
            Qt6::Test
)
target_compile_definitions(
    wing_theme_tests PRIVATE QT_NO_EMIT QT_NO_CONTEXTLESS_CONNECT
)
enable_testing()
add_test(NAME wing_theme_tests COMMAND wing_theme_tests)
set_tests_properties(
    wing_theme_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    wing_theme_benchmark
    benchmark_icons.cpp
    "${WING_SOURCE_ROOT}/src/class/settings/thememanager.cpp"
    "${WING_SOURCE_ROOT}/src/class/settings/thememanager.h"
    "${WING_SOURCE_ROOT}/resources.qrc"
    "${WING_SOURCE_ROOT}/theme/breeze.qrc"
)
target_include_directories(
    wing_theme_benchmark PRIVATE "${WING_SOURCE_ROOT}/src"
                                 "${WING_SOURCE_ROOT}"
)
target_link_libraries(
    wing_theme_benchmark
    PRIVATE WingPlugin
            Qt6::Widgets
            Qt6::Svg
            Qt6::Xml
)
