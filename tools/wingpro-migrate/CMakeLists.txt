cmake_minimum_required(VERSION 3.20)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(WingWorkspaceMigrator LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

option(WINGHEX_BUILD_WORKSPACE_MIGRATOR
       "Build the legacy JSON workspace migration tool" OFF)
if(NOT WINGHEX_BUILD_WORKSPACE_MIGRATOR)
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/WorkspaceCodec.cmake")

add_library(wing_workspace_legacy STATIC legacyworkspace.cpp)
target_link_libraries(wing_workspace_legacy PUBLIC wing_workspace_codec)
target_include_directories(wing_workspace_legacy
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(wingpro-migrate main.cpp)
target_link_libraries(wingpro-migrate PRIVATE wing_workspace_legacy)
include(GNUInstallDirs)
install(TARGETS wingpro-migrate RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
