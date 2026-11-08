cmake_minimum_required(VERSION 3.20)
project(StartupProfileTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.6.2 REQUIRED COMPONENTS Widgets)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
get_filename_component(WING_SOURCE_ROOT "../.." ABSOLUTE)
foreach(mode enabled disabled)
    add_executable(startup_profile_${mode} profile_probe.cpp)
    target_include_directories(
        startup_profile_${mode} PRIVATE "${WING_SOURCE_ROOT}/src"
    )
    target_link_libraries(startup_profile_${mode} PRIVATE Qt6::Widgets)
endforeach()
target_sources(
    startup_profile_enabled
    PRIVATE "${WING_SOURCE_ROOT}/src/class/application/startuptrace.cpp"
)
target_compile_definitions(
    startup_profile_enabled PRIVATE WINGHEX_ENABLE_STARTUP_PROFILE
)
enable_testing()
add_test(
    NAME startup_profile_tests
    COMMAND
        "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_profile.py"
        "$<TARGET_FILE:startup_profile_enabled>"
        "$<TARGET_FILE:startup_profile_disabled>"
)
set_tests_properties(
    startup_profile_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
