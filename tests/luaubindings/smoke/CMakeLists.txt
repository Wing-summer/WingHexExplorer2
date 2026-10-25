# Copyright (c) 2026, wingsummer
# SPDX-License-Identifier: BSD-3-Clause
# See 3rdparty/WingLuauQt/LICENSE for license information.

cmake_minimum_required(VERSION 3.24)
project(WingHexRegistrationSmoke LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
get_filename_component(HOST_SOURCE_DIR "../../.." ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(HOST_BUILD_DIR "${HOST_SOURCE_DIR}/build/Debug" CACHE PATH "Existing host build; no dependencies are downloaded")
set(BINDINGS_DIR "${HOST_BUILD_DIR}/generated/WingLuauBind")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets UiTools OpenGLWidgets)
find_package(Threads REQUIRED)
find_library(WING_BINDINGS WingLuauBind PATHS "${HOST_BUILD_DIR}/luau-bindings" REQUIRED NO_DEFAULT_PATH)
find_library(WING_PLUGIN WingPlugin PATHS "${HOST_BUILD_DIR}/WingPlugin" REQUIRED NO_DEFAULT_PATH)
foreach(component IN ITEMS Require Config Compiler Ast Bytecode VM Common)
  find_library(LUAU_${component} NAMES Luau.${component} PATHS "${HOST_BUILD_DIR}/luau" REQUIRED NO_DEFAULT_PATH)
  list(APPEND _luau_libraries "${LUAU_${component}}")
endforeach()
add_executable(WingHexRegistrationSmoke ../registration.cpp
  "${HOST_SOURCE_DIR}/3rdparty/WingLuauQt/support/execute_luau.cpp"
  "${HOST_SOURCE_DIR}/src/luau/wingluaurequire.cpp"
  "${HOST_SOURCE_DIR}/src/dialog/framelessdialogbase.cpp"
  "${HOST_SOURCE_DIR}/src/dialog/framelessmainwindow.cpp")
target_compile_features(WingHexRegistrationSmoke PRIVATE cxx_std_17)
target_compile_definitions(WingHexRegistrationSmoke PRIVATE WING_CFFI_REPORT="${BINDINGS_DIR}/lua_c_api_report.json")
target_include_directories(WingHexRegistrationSmoke PRIVATE
  "${BINDINGS_DIR}/include" "${BINDINGS_DIR}/generated"
  "${HOST_SOURCE_DIR}/3rdparty/WingLuauQt/include"
  "${HOST_SOURCE_DIR}/src" "${HOST_SOURCE_DIR}"
  "${HOST_SOURCE_DIR}/3rdparty/LuaBridge3/Source")
foreach(component IN ITEMS Require Config Compiler Ast Bytecode VM Common)
  target_include_directories(WingHexRegistrationSmoke PRIVATE
    "${HOST_SOURCE_DIR}/3rdparty/luau/${component}/include")
endforeach()
target_link_libraries(WingHexRegistrationSmoke PRIVATE "${WING_BINDINGS}"
  ${_luau_libraries} "${WING_PLUGIN}" Qt6::Core Qt6::Gui Qt6::Widgets
  Qt6::UiTools Qt6::OpenGLWidgets Threads::Threads ${CMAKE_DL_LIBS})
find_library(CFFI_LIBRARY cffi PATHS "${HOST_BUILD_DIR}/3rdparty/cffi-luau" REQUIRED NO_DEFAULT_PATH)
find_library(FFI_LIBRARY ffi PATHS "${HOST_BUILD_DIR}/_deps/libffi-build" REQUIRED NO_DEFAULT_PATH)
target_link_libraries(WingHexRegistrationSmoke PRIVATE "${CFFI_LIBRARY}" "${FFI_LIBRARY}")
add_library(WingCffiProbeLibrary SHARED "${HOST_SOURCE_DIR}/3rdparty/cffi-luau/tests/testlib.cc")
set_target_properties(WingCffiProbeLibrary PROPERTIES AUTOMOC OFF)
target_compile_definitions(WingHexRegistrationSmoke PRIVATE WING_CFFI_TESTLIB="$<TARGET_FILE:WingCffiProbeLibrary>")
add_dependencies(WingHexRegistrationSmoke WingCffiProbeLibrary)
enable_testing()
add_test(NAME registration COMMAND WingHexRegistrationSmoke)
set_tests_properties(registration PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Override one archive registration function with freshly generated code while
# keeping the remaining SDK/library objects. No application/dependency rebuild.
set(SDK_PROBE_CLASS "" CACHE STRING "Optional single SDK class generated from current framework sources")
if(SDK_PROBE_CLASS)
  if(NOT SDK_PROBE_CLASS MATCHES "^Q[A-Za-z0-9_]+$")
    message(FATAL_ERROR "SDK_PROBE_CLASS must name one Qt class")
  endif()
  if(NOT Python3_EXECUTABLE AND EXISTS "${HOST_BUILD_DIR}/.wingluauqt-venv/bin/python")
    set(Python3_EXECUTABLE "${HOST_BUILD_DIR}/.wingluauqt-venv/bin/python")
  endif()
  find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter)
  set(_framework "${HOST_SOURCE_DIR}/3rdparty/WingLuauQt")
  file(GLOB _generator_inputs CONFIGURE_DEPENDS "${_framework}/python/qtluau_codegen/*.py")
  set(_probe "${CMAKE_CURRENT_BINARY_DIR}/generated/sdk_${SDK_PROBE_CLASS}.cpp")
  add_custom_command(OUTPUT "${_probe}"
    COMMAND "${CMAKE_COMMAND}" -E env "PYTHONPATH=${_framework}/python"
      "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_sdk_probe.py"
      --class-name "${SDK_PROBE_CLASS}" --output "${CMAKE_CURRENT_BINARY_DIR}/generated"
    DEPENDS ${_generator_inputs} "${_framework}/config/framework-bindings.json"
      "${CMAKE_CURRENT_SOURCE_DIR}/generate_sdk_probe.py"
    VERBATIM)
  target_sources(WingHexRegistrationSmoke PRIVATE "${_probe}")
  target_include_directories(WingHexRegistrationSmoke BEFORE PRIVATE "${_framework}/include")
endif()
