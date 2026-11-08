cmake_minimum_required(VERSION 3.20)
project(WingToastTests LANGUAGES CXX)
option(WINGHEX_FORCE_CPP17 "Use the C++17 fallback" OFF)
if(WINGHEX_FORCE_CPP17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.6.2 REQUIRED COMPONENTS Widgets Test)
get_filename_component(WING_SOURCE_ROOT "../.." ABSOLUTE)
add_executable(
    wing_toast_tests tst_toast.cpp "${WING_SOURCE_ROOT}/src/control/toast.cpp"
                     "${WING_SOURCE_ROOT}/src/control/toast.h"
)
target_include_directories(wing_toast_tests PRIVATE "${WING_SOURCE_ROOT}/src")
target_link_libraries(wing_toast_tests PRIVATE Qt6::Widgets Qt6::Test)
enable_testing()
add_test(NAME wing_toast_tests COMMAND wing_toast_tests)
set_tests_properties(
    wing_toast_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
