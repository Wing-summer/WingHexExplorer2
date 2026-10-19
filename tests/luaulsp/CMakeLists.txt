cmake_minimum_required(VERSION 3.20)
project(WingLuauLanguageServerTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

get_filename_component(WING_SOURCE_ROOT "../.." ABSOLUTE)
set(LUAU_BUILD_CLI
    OFF
    CACHE BOOL "" FORCE
)
set(LUAU_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE
)
set(LUAU_BUILD_WEB
    OFF
    CACHE BOOL "" FORCE
)
set(LUAU_WERROR
    OFF
    CACHE BOOL "" FORCE
)
add_subdirectory("${WING_SOURCE_ROOT}/3rdparty/luau" luau EXCLUDE_FROM_ALL)

add_executable(
    wing_luau_lsp_tests
    tst_luaulanguageserver.cpp
    ${WING_SOURCE_ROOT}/src/luau/lsp/luaulanguageserver.cpp
    ${WING_SOURCE_ROOT}/src/luau/lsp/luaulanguageserver.h
    ${WING_SOURCE_ROOT}/src/luau/lsp/lsp.h
    ${WING_SOURCE_ROOT}/src/luau/lsp/attachcommentsvisitor.cpp
    ${WING_SOURCE_ROOT}/src/luau/lsp/findnodetype.cpp
    ${WING_SOURCE_ROOT}/src/luau/lsp/semantictokensvisitor.cpp
    ${WING_SOURCE_ROOT}/src/luau/lsp/typehelpers.cpp
    ${WING_SOURCE_ROOT}/src/luau/wingluaurequire.cpp
)
target_include_directories(
    wing_luau_lsp_tests PRIVATE "${WING_SOURCE_ROOT}/src"
)
target_link_libraries(
    wing_luau_lsp_tests
    PRIVATE Qt6::Core
            Qt6::Gui
            Luau.Analysis
            Luau.Require
            Luau.Compiler
            Luau.VM
)

enable_testing()
foreach(
    scenario IN
    ITEMS features
          types
          overloads
          modules
)
    add_test(NAME wing_luau_lsp_${scenario} COMMAND wing_luau_lsp_tests
                                                    ${scenario}
    )
    set_tests_properties(
        wing_luau_lsp_${scenario} PROPERTIES ENVIRONMENT
                                             "QT_QPA_PLATFORM=offscreen"
    )
endforeach()
