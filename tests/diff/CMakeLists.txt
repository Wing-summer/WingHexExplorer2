cmake_minimum_required(VERSION 3.20)
project(WingDiffTests LANGUAGES CXX)
option(WINGHEX_FORCE_CPP17 "Use C++17" OFF)
set(CMAKE_CXX_STANDARD 20)
if(WINGHEX_FORCE_CPP17)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent Test)
get_filename_component(WING_ROOT "../.." ABSOLUTE)
add_subdirectory("${WING_ROOT}/3rdparty/QHexView" QHexView)
add_executable(
    diff_tests
    tst_diff.cpp
    "${WING_ROOT}/src/control/diffrenderer.cpp"
    "${WING_ROOT}/src/class/diff/binarydiff.cpp"
    "${WING_ROOT}/src/class/diff/diffsource.cpp"
    "${WING_ROOT}/src/model/diffresultmodel.cpp"
)
target_include_directories(
    diff_tests PRIVATE "${WING_ROOT}/src" "${WING_ROOT}/3rdparty"
)
target_link_libraries(
    diff_tests
    PRIVATE QHexView
            Qt6::Widgets
            Qt6::Concurrent
            Qt6::Test
)
enable_testing()
add_test(NAME diff_tests COMMAND diff_tests)
set_tests_properties(
    diff_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(
    diff_benchmark benchmark.cpp "${WING_ROOT}/src/class/diff/binarydiff.cpp"
)
target_include_directories(diff_benchmark PRIVATE "${WING_ROOT}/src")
target_link_libraries(diff_benchmark PRIVATE Qt6::Core)
