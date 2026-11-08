include_guard(GLOBAL)
find_package(Qt6 6.6.2 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
get_filename_component(WING_WORKSPACE_ROOT ".." ABSOLUTE BASE_DIR
                       "${CMAKE_CURRENT_LIST_DIR}")
if(NOT TARGET QHexView)
    add_subdirectory("${WING_WORKSPACE_ROOT}/3rdparty/QHexView"
                     "${CMAKE_CURRENT_BINARY_DIR}/QHexView")
endif()
add_library(wing_workspace_codec STATIC
            "${WING_WORKSPACE_ROOT}/src/class/file/workspacemanager.cpp")
target_include_directories(
    wing_workspace_codec PUBLIC "${WING_WORKSPACE_ROOT}/src"
                                "${WING_WORKSPACE_ROOT}/3rdparty")
target_link_libraries(wing_workspace_codec PUBLIC QHexView Qt6::Core Qt6::Gui)
