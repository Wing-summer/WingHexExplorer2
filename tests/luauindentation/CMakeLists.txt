cmake_minimum_required(VERSION 3.20)
project(WingLuauIndentationTests LANGUAGES CXX)

option(WINGHEX_FORCE_CPP17 "Build the tests with the C++17 fallback" OFF)
if(WINGHEX_FORCE_CPP17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.6.2 REQUIRED COMPONENTS Widgets PrintSupport Test)
get_filename_component(
    WING_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE
)
set(BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE
)
add_subdirectory(
    "${WING_SOURCE_ROOT}/3rdparty/WingCodeEdit" WingCodeEdit EXCLUDE_FROM_ALL
)

foreach(
    option IN
    ITEMS LUAU_BUILD_CLI
          LUAU_BUILD_TESTS
          LUAU_BUILD_WEB
          LUAU_WERROR
)
    set(${option}
        OFF
        CACHE BOOL "" FORCE
    )
endforeach()
set(CMAKE_AUTOMOC OFF)
add_subdirectory("${WING_SOURCE_ROOT}/3rdparty/luau" luau EXCLUDE_FROM_ALL)
set(CMAKE_AUTOMOC ON)

add_executable(
    wing_luau_indentation_tests
    tst_luauindentation.cpp
    "${WING_SOURCE_ROOT}/src/control/editor/luauindentation.cpp"
    "${WING_SOURCE_ROOT}/3rdparty/QConsoleWidget/QConsoleWidget.cpp"
    "${WING_SOURCE_ROOT}/3rdparty/QConsoleWidget/QConsoleWidget.h"
    "${WING_SOURCE_ROOT}/3rdparty/QConsoleWidget/QConsoleIODevice.cpp"
    "${WING_SOURCE_ROOT}/3rdparty/QConsoleWidget/QConsoleIODevice.h"
)
target_include_directories(
    wing_luau_indentation_tests PRIVATE "${WING_SOURCE_ROOT}/src"
                                        "${WING_SOURCE_ROOT}/3rdparty"
)
target_link_libraries(
    wing_luau_indentation_tests PRIVATE WingCodeEdit Luau.Ast Qt6::Test
)

enable_testing()
add_test(NAME wing_luau_indent_newline COMMAND wing_luau_indentation_tests
                                               newLine indentationModes
)
add_test(NAME wing_luau_indent_closing COMMAND wing_luau_indentation_tests
                                               currentLine
)
add_test(NAME wing_luau_indent_editor
         COMMAND wing_luau_indentation_tests typedClosing newlineSelection
                 identifierSuffix completion controls policyFallback
)
add_test(NAME wing_luau_indent_console COMMAND wing_luau_indentation_tests
                                               console
)
set_tests_properties(
    wing_luau_indent_newline
    wing_luau_indent_closing
    wing_luau_indent_editor
    wing_luau_indent_console
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
