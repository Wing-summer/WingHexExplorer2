cmake_minimum_required(VERSION 3.20)
project(AdsStateTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD
    17
    CACHE STRING "C++ standard"
)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
set(QT_VERSION_MAJOR 6)
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE
)
set(BUILD_STATIC
    ON
    CACHE BOOL "" FORCE
)
set(ADS_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/Qt-Advanced-Docking-System"
    CACHE PATH "ADS sources, optionally an older revision for comparison"
)
option(ADS_LEGACY_STATE "Build only the benchmark against the old XML API" OFF)
add_subdirectory("${ADS_SOURCE_DIR}" ads)
get_target_property(ads_target ads::qtadvanceddocking ALIASED_TARGET)
set_property(TARGET ${ads_target} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
add_executable(ads_benchmark benchmark.cpp)
target_link_libraries(ads_benchmark PRIVATE ads::qtadvanceddocking Qt6::Widgets)
if(ADS_LEGACY_STATE)
    target_compile_definitions(ads_benchmark PRIVATE ADS_LEGACY_STATE)
    return()
endif()
add_executable(ads_startup_probe startup_probe.cpp)
target_link_libraries(
    ads_startup_probe PRIVATE ads::qtadvanceddocking Qt6::Widgets
)
add_executable(ads_state_tests tst_dockingstate.cpp)
add_dependencies(ads_state_tests ads_startup_probe)
target_link_libraries(
    ads_state_tests PRIVATE ads::qtadvanceddocking Qt6::Test Qt6::Widgets
)
enable_testing()
add_test(NAME ads_state_tests COMMAND ads_state_tests)
set_tests_properties(
    ads_state_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
