cmake_minimum_required(VERSION 3.20)
project(WingLuauPluginsTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Test Gui CorePrivate)
get_filename_component(HOST "../.." ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LUAU_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${HOST}/3rdparty/luau" "${CMAKE_CURRENT_BINARY_DIR}/luau")
add_executable(tst_luauplugins tst_luauplugins.cpp
  "${HOST}/src/luau/plugin/scriptpluginpackage.cpp"
  "${HOST}/src/class/file/memoryfilesystem.cpp"
  "${HOST}/src/luau/plugin/pluginiodevice.cpp"
  "${HOST}/src/luau/plugin/pluginpaintcontext.cpp"
  "${HOST}/src/luau/runtime/scriptchunk.cpp"
  "${HOST}/src/luau/wingluaurequire.cpp")
target_include_directories(tst_luauplugins PRIVATE "${HOST}/src" "${HOST}/3rdparty/luau/Analysis/include")
target_link_libraries(tst_luauplugins PRIVATE Qt6::Core Qt6::CorePrivate Qt6::Gui Qt6::Test Luau.Compiler Luau.VM Luau.Require Luau.Config)
enable_testing()
add_test(NAME luauplugins COMMAND tst_luauplugins)

add_executable(WingLuauCompile "${HOST}/tools/luaucompile.cpp")
target_link_libraries(WingLuauCompile PRIVATE Luau.Compiler Qt6::Core)
set_target_properties(WingLuauCompile PROPERTIES AUTOMOC OFF OUTPUT_NAME wing-luau-compile)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME luau_plugin_packaging COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_packaging.py")
