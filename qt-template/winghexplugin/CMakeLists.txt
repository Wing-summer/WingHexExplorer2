cmake_minimum_required(VERSION 3.16)

project(%{ProjectName} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
endif()

option(TEST_MODE TRUE)

set(WINGHEX_SDK "%{WingHexSDKPath}")

set(PLUGIN_INTERFACE_FOUND FALSE)
set(PLUGIN_SETPAGE_FOUND FALSE)
if(EXISTS "${WINGHEX_SDK}/iwingplugin.h")
    set(PLUGIN_INTERFACE_FOUND TRUE)
endif()
if(EXISTS "${WINGHEX_SDK}/settingpage.h")
    set(PLUGIN_SETPAGE_FOUND TRUE)
endif()
if(PLUGIN_INTERFACE_FOUND AND PLUGIN_SETPAGE_FOUND)
    message(STATUS "${WINGHEX_SDK} is valid SDK path")
else()
    message(FATAL_ERROR "Invalid SDK path!")
endif()

set(WINGHEX_SDK_HEADER "${WINGHEX_SDK}/iwingplugin.h"
                       "${WINGHEX_SDK}/settingpage.h")
include_directories(${WINGHEX_SDK})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

@if %{HasTranslation}
set(TS_FILES %{TsFileName})

set(TRANSLATION_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(${QT_VERSION_MAJOR} EQUAL 5)
    qt5_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES})
elseif(${QT_VERSION_MAJOR} EQUAL 6)
    qt6_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES})
else()
    message(FATAL_ERROR "Unsupported QT version")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/TranslationUtils.cmake)
add_plugin_translations_resource(QM_RES %{ProjectName} ${QM_FILES})
@else
## Example for WingHexExplorer2 Plugin Localization

# set(TS_FILES )

# set(TRANSLATION_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# if(${QT_VERSION_MAJOR} EQUAL 5)
#     qt5_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES})
# elseif(${QT_VERSION_MAJOR} EQUAL 6)
#     qt6_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES})
# else()
#     message(FATAL_ERROR "Unsupported QT version")
# endif()

# include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/TranslationUtils.cmake)
# add_plugin_translations_resource(QM_RES %{ProjectName} ${QM_FILES})
@endif

add_library(%{ProjectName} SHARED
    ${WINGHEX_SDK_HEADER}
@if %{HasTranslation}
    ${QM_FILES}
    ${QM_RES}
@else
    # ${QM_FILES}
    # ${QM_RES}
@endif
    %{SrcFileName}
    %{HdrFileName}
    %{PluginJsonFile}
)

set_target_properties(%{ProjectName} PROPERTIES SUFFIX ".wingplg")

if(TEST_MODE)
    # If you want to be able to debug easily every time you compile, please set
    # this variable. Because this test plugin is a subproject of the main
    # project, use CMAKE_BINARY_DIR

    set(WINGHEX_PATH "%{WingHexEXEPath}")

    set(WINGHEX_PLUGIN_PATH "${WINGHEX_PATH}/plugin")
    add_custom_command(
        TARGET %{ProjectName}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WINGHEX_PLUGIN_PATH}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:%{ProjectName}>
                ${WINGHEX_PLUGIN_PATH})
endif()

target_link_libraries(%{ProjectName} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
