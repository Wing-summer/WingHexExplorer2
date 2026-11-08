cmake_minimum_required(VERSION 3.20)
project(WorkspaceTests LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(WINGHEX_BUILD_WORKSPACE_MIGRATOR
       "Build the legacy JSON workspace migration tool" OFF
)
find_package(Qt6 6.6.2 REQUIRED COMPONENTS Test)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/WorkspaceCodec.cmake")
if(WINGHEX_BUILD_WORKSPACE_MIGRATOR)
    add_subdirectory("../../tools/wingpro-migrate" migrator)
endif()
add_executable(
    workspace_tests tst_workspace.cpp
                    "../../tools/wingpro-migrate/legacyworkspace.cpp"
)
target_include_directories(
    workspace_tests PRIVATE "../../tools/wingpro-migrate"
)
target_link_libraries(workspace_tests PRIVATE wing_workspace_codec Qt6::Test)
if(WINGHEX_BUILD_WORKSPACE_MIGRATOR)
    target_compile_definitions(
        workspace_tests
        PRIVATE WORKSPACE_MIGRATOR_PATH="$<TARGET_FILE:wingpro-migrate>"
    )
    add_dependencies(workspace_tests wingpro-migrate)
endif()
enable_testing()
add_test(NAME workspace_tests COMMAND workspace_tests)
