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
    ${WING_SOURCE_ROOT}/src/luau/lsp/luausymbols.cpp
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

set(WING_GENERATED_BINDINGS_DIR "" CACHE PATH "Existing generated binding project for integration coverage")
if(WING_GENERATED_BINDINGS_DIR)
    add_test(NAME wing_luau_lsp_bindings COMMAND wing_luau_lsp_tests bindings)
    set_tests_properties(wing_luau_lsp_bindings PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;WING_BINDINGS_DIR=${WING_GENERATED_BINDINGS_DIR}")
    add_test(NAME wing_luau_lsp_hexdoc COMMAND wing_luau_lsp_tests hexdoc)
    set_tests_properties(wing_luau_lsp_hexdoc PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;WING_BINDINGS_DIR=${WING_GENERATED_BINDINGS_DIR};WING_SOURCE_DIR=${WING_SOURCE_ROOT}")
endif()

option(WING_LUAU_LSP_EDITOR_TESTS "Build editor completion interaction tests" OFF)
if(WING_LUAU_LSP_EDITOR_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport Test)
    add_subdirectory("${WING_SOURCE_ROOT}/3rdparty/WingCodeEdit" WingCodeEdit EXCLUDE_FROM_ALL)
    get_target_property(editor_lsp_sources wing_luau_lsp_tests SOURCES)
    list(REMOVE_ITEM editor_lsp_sources tst_luaulanguageserver.cpp)
    add_executable(wing_luau_editor_tests
        tst_editorcompletion.cpp
        tst_editortheme.cpp
        ${editor_lsp_sources}
        ${WING_SOURCE_ROOT}/src/control/editor/luaucompletion.cpp
        ${WING_SOURCE_ROOT}/src/control/editor/editorlspevent.cpp
        ${WING_SOURCE_ROOT}/src/control/editor/lspeditorinterface.cpp
        ${WING_SOURCE_ROOT}/src/control/editor/codeinfotip.cpp
        ${WING_SOURCE_ROOT}/src/control/editor/snippetprocessor.cpp
        ${WING_SOURCE_ROOT}/src/model/codecompletionmodel.cpp
        ${WING_SOURCE_ROOT}/src/class/util/resettabletimer.cpp
        ${WING_SOURCE_ROOT}/src/class/settings/thememanager.h)
    target_include_directories(wing_luau_editor_tests PRIVATE
        "${WING_SOURCE_ROOT}" "${WING_SOURCE_ROOT}/src" "${WING_SOURCE_ROOT}/3rdparty")
    target_link_libraries(wing_luau_editor_tests PRIVATE
        WingCodeEdit Qt6::Test Luau.Analysis Luau.Require Luau.Compiler Luau.VM)
    add_test(NAME wing_luau_editor_interaction COMMAND wing_luau_editor_tests)
    set_tests_properties(wing_luau_editor_interaction PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
