cmake_minimum_required(VERSION 3.18)

project(WingHexExplorer2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_VERSION 1.0.0~beta)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Concurrent
                                                  PrintSupport LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED
             COMPONENTS Widgets Network Concurrent PrintSupport LinguistTools)

add_definitions(-DWINGHEX_VERSION="${PROJECT_VERSION}"
                -DAPP_ORG="WingCloudStudio" -DAPP_NAME="${PROJECT_NAME}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/WINGHEX_VERSION" ${PROJECT_VERSION})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/QT_VERSION" ${QT_VERSION_MAJOR})

if(WIN32)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

if(${QT_VERSION_MAJOR} EQUAL 5)
    if(Qt5Widgets_VERSION VERSION_LESS 5.14.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.14!")
    endif()
elseif(${QT_VERSION_MAJOR} EQUAL 6)
    if(Qt6Widgets_VERSION VERSION_LESS 6.6.2)
        message(FATAL_ERROR "Minimum supported Qt6 version is 6.6.2!")
    endif()
else()
    message(FATAL_ERROR "Unsupported Qt version!")
endif()

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
option(QWINDOWKIT_BUILD_STATIC "Build static libraries" TRUE)
option(QWINDOWKIT_INSTALL "Install library" OFF)
set(ADS_VERSION 4.3.1)

add_subdirectory(3rdparty/qwindowkit)
add_subdirectory(3rdparty/QHexView)
add_subdirectory(3rdparty/qcodeedit2)
add_subdirectory(3rdparty/qcodemodel2)
add_subdirectory(3rdparty/Qt-Advanced-Docking-System)
add_subdirectory(3rdparty/AngelScript/sdk/angelscript/projects/cmake)
add_subdirectory(3rdparty/QConsoleWidget)
add_subdirectory(3rdparty/QJsonModel)

set(ANGEL_SCRIPT_ADDON_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/AngelScript/sdk/add_on")
set(ANGEL_SCRIPT_ADDON
    ${ANGEL_SCRIPT_ADDON_ROOT}/autowrapper/aswrappedcall.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/contextmgr/contextmgr.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/contextmgr/contextmgr.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/datetime/datetime.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/datetime/datetime.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptany/scriptany.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptany/scriptany.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptarray/scriptarray.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptarray/scriptarray.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfile.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfile.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfilesystem.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfilesystem.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptgrid/scriptgrid.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptgrid/scriptgrid.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthandle/scripthandle.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthandle/scripthandle.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthelper/scripthelper.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scripthelper/scripthelper.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmath.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmath.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmathcomplex.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptmath/scriptmathcomplex.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/serializer/serializer.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/serializer/serializer.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/weakref/weakref.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/weakref/weakref.h)

set(QAPPLICATION_CLASS
    QApplication
    CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(3rdparty/SingleApplication)

set(RIBBON_SRC
    3rdparty/QWingRibbon/ribbon.cpp
    3rdparty/QWingRibbon/ribbon.h
    3rdparty/QWingRibbon/ribbonbuttongroup.cpp
    3rdparty/QWingRibbon/ribbonbuttongroup.h
    3rdparty/QWingRibbon/ribbonbuttongroup.ui
    3rdparty/QWingRibbon/ribbontabcontent.cpp
    3rdparty/QWingRibbon/ribbontabcontent.h
    3rdparty/QWingRibbon/ribbontabcontent.ui)

set(QPATHEDIT_SRC 3rdparty/QPathEdit/qpathedit.cpp
                  3rdparty/QPathEdit/qpathedit.h)

set(DIALOG_SRC
    src/dialog/framelessmainwindow.h
    src/dialog/framelessmainwindow.cpp
    src/dialog/mainwindow.cpp
    src/dialog/mainwindow.h
    src/dialog/settingdialog.cpp
    src/dialog/settingdialog.h
    src/dialog/settingdialog.ui
    src/dialog/framelessdialogbase.h
    src/dialog/framelessdialogbase.cpp
    src/dialog/aboutsoftwaredialog.ui
    src/dialog/aboutsoftwaredialog.cpp
    src/dialog/aboutsoftwaredialog.h
    src/dialog/fileinfodialog.cpp
    src/dialog/fileinfodialog.h
    src/dialog/encodingdialog.cpp
    src/dialog/encodingdialog.h
    src/dialog/pluginwindow.cpp
    src/dialog/pluginwindow.h
    src/dialog/metadialog.cpp
    src/dialog/metadialog.h
    src/dialog/driverselectordialog.cpp
    src/dialog/driverselectordialog.h
    src/dialog/openregiondialog.cpp
    src/dialog/openregiondialog.h
    src/dialog/scriptingdialog.h
    src/dialog/scriptingdialog.cpp
    src/dialog/finddialog.cpp
    src/dialog/finddialog.h
    src/dialog/checksumdialog.h
    src/dialog/checksumdialog.cpp
    src/dialog/colorpickerdialog.h
    src/dialog/colorpickerdialog.cpp
    src/dialog/colorpickerdialog.ui)

set(CONTROL_SRC
    src/control/editorview.h
    src/control/editorview.cpp
    src/control/toast.h
    src/control/toast.cpp
    src/control/scriptingconsole.cpp
    src/control/scriptingconsole.h
    src/control/gotowidget.h
    src/control/gotowidget.cpp
    src/control/gotowidget.ui
    src/control/scripteditor.h
    src/control/scripteditor.cpp
    src/control/qcolorpicker_slider.hpp
    src/control/qcolorpicker_slider.cpp
    src/control/huecolorpickerslider.h
    src/control/huecolorpickerslider.cpp
    src/control/qtableviewext.h
    src/control/qtableviewext.cpp
    src/control/qlistviewext.h
    src/control/qlistviewext.cpp
    src/control/qcodecompletionwidget.h
    src/control/qcodecompletionwidget_p.h
    src/control/qcodecompletionwidget.cpp)

set(CLASS_SRC
    src/class/logger.cpp
    src/class/logger.h
    src/class/skinmanager.cpp
    src/class/skinmanager.h
    src/class/workspacemanager.cpp
    src/class/workspacemanager.h
    src/class/setting.h
    src/class/eventfilter.h
    src/class/framelesshelper.h
    src/class/framelesshelper.cpp
    src/class/qkeysequences.h
    src/class/qkeysequences.cpp
    src/class/wingmessagebox.h
    src/class/wingmessagebox.cpp
    src/class/recentfilemanager.cpp
    src/class/recentfilemanager.h
    src/class/winginputdialog.h
    src/class/winginputdialog.cpp
    src/class/wingfiledialog.h
    src/class/wingfiledialog.cpp
    src/class/scriptmachine.h
    src/class/scriptmachine.cpp
    src/class/scriptmanager.h
    src/class/scriptmanager.cpp
    src/class/languagemanager.h
    src/class/languagemanager.cpp
    src/class/settingmanager.h
    src/class/settingmanager.cpp
    src/class/wingprogressdialog.h
    src/class/wingprogressdialog.cpp
    src/class/asdebugger.h
    src/class/asdebugger.cpp
    src/class/scriptconsolemachine.h
    src/class/scriptconsolemachine.cpp
    src/class/wingangelapi.h
    src/class/wingangelapi.cpp
    src/class/angelobjstring.h
    src/class/angelobjstring.cpp
    src/class/scripteditortheme.h
    src/class/scripteditortheme.cpp
    src/class/appmanager.h
    src/class/appmanager.cpp
    src/class/angelscripthelper.h
    src/class/ascompletion.cpp
    src/class/ascompletion.h
    src/class/asbuilder.h
    src/class/asbuilder.cpp
    src/class/qaslexer.cpp
    src/class/qaslexer.h
    src/class/qasparser.cpp
    src/class/qasparser.h
    src/class/qdocumentsearch.cpp
    src/class/qdocumentsearch.h
    src/class/ascontextmgr.h
    src/class/ascontextmgr.cpp)

set(WIDGET_FRAME_SRC
    src/widgetframe/windowbar.cpp src/widgetframe/windowbar.h
    src/widgetframe/windowbar_p.h src/widgetframe/windowbutton.cpp
    src/widgetframe/windowbutton.h src/widgetframe/windowbutton_p.h)

set(MODEL_SRC
    src/model/findresultmodel.h
    src/model/findresultmodel.cpp
    src/model/numshowmodel.h
    src/model/numshowmodel.cpp
    src/model/bookmarksmodel.h
    src/model/bookmarksmodel.cpp
    src/model/metadatamodel.h
    src/model/metadatamodel.cpp
    src/model/checksummodel.h
    src/model/checksummodel.cpp
    src/model/qjsontablemodel.h
    src/model/qjsontablemodel.cpp
    src/model/scriptobjmodel.h
    src/model/scriptobjmodel.cpp
    src/model/dbgcallstackmodel.h
    src/model/dbgcallstackmodel.cpp
    src/model/dbgvarshowmodel.h
    src/model/dbgvarshowmodel.cpp
    src/model/dbgbreakpointmodel.h
    src/model/dbgbreakpointmodel.cpp)

set(SETTING_SRC
    src/settings/generalsettingdialog.h
    src/settings/generalsettingdialog.cpp
    src/settings/generalsettingdialog.ui
    src/settings/pluginsettingdialog.h
    src/settings/pluginsettingdialog.cpp
    src/settings/pluginsettingdialog.ui
    src/settings/editorsettingdialog.h
    src/settings/editorsettingdialog.cpp
    src/settings/editorsettingdialog.ui
    src/settings/scriptsettingdialog.h
    src/settings/scriptsettingdialog.cpp
    src/settings/scriptsettingdialog.ui
    src/settings/scriptbehaviorsettingdialog.h
    src/settings/scriptbehaviorsettingdialog.cpp
    src/settings/scriptbehaviorsettingdialog.ui
    src/settings/othersettingsdialog.h
    src/settings/othersettingsdialog.cpp
    src/settings/othersettingsdialog.ui)

set(SCRIPT_ADDON_SRC
    src/scriptaddon/scriptqstring.h
    src/scriptaddon/scriptqstring.cpp
    src/scriptaddon/scriptqdictionary.h
    src/scriptaddon/scriptqdictionary.cpp
    src/scriptaddon/scriptregex.h
    src/scriptaddon/scriptregex.cpp
    src/scriptaddon/scriptcolor.h
    src/scriptaddon/scriptcolor.cpp)

set(CODEEDIT_WIDGET
    src/qcodeeditwidget/qgotolinepanel.h
    src/qcodeeditwidget/qgotolinepanel.cpp
    src/qcodeeditwidget/qgotolinepanel.ui
    src/qcodeeditwidget/qsearchreplacepanel.h
    src/qcodeeditwidget/qsearchreplacepanel.cpp
    src/qcodeeditwidget/qsearchreplacepanel.ui
    src/qcodeeditwidget/qeditconfig.h
    src/qcodeeditwidget/qeditconfig.cpp
    src/qcodeeditwidget/qeditconfig.ui
    src/qcodeeditwidget/qstatuspanel.cpp
    src/qcodeeditwidget/qstatuspanel.h
    src/qcodeeditwidget/qformatconfig.h
    src/qcodeeditwidget/qformatconfig.cpp
    src/qcodeeditwidget/qformatconfig.ui)

set(PLUGIN_SRC
    src/plugin/iwingplugin.h src/plugin/pluginsystem.cpp
    src/plugin/pluginsystem.h src/plugin/settingpage.h
    src/plugin/settingpage.cpp)

# localization support
file(
    GLOB_RECURSE TS_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/lang/*/winghex.ts")

foreach(TS_FILE IN LISTS TS_FILES)
    get_filename_component(TS_DIR ${TS_FILE} DIRECTORY)
    set(QM_DIR "${CMAKE_CURRENT_BINARY_DIR}/.tmp/${TS_DIR}")
    set(QM_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TS_DIR}")
    file(MAKE_DIRECTORY "${QM_DIR}")
    file(MAKE_DIRECTORY "${QM_OUT_DIR}")
    set_source_files_properties(${TS_FILE} PROPERTIES OUTPUT_LOCATION ${QM_DIR})
endforeach()

set(TRANSLATION_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QConsoleWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QHexView
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QJsonModel
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QPathEdit
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qcodeedit2
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Qt-Advanced-Docking-System/src
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QWingRibbon
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SingleApplication
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(${QT_VERSION_MAJOR} EQUAL 5)
    qt5_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES} OPTIONS
                           -no-obsolete)
elseif(${QT_VERSION_MAJOR} EQUAL 6)
    qt6_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES} OPTIONS
                           -no-obsolete)
else()
    message(FATAL_ERROR "Unsupported QT version")
endif()

set(LANG_SRC "${CMAKE_CURRENT_SOURCE_DIR}/lang")
set(LANG_DEST "${CMAKE_CURRENT_BINARY_DIR}/.tmp/lang")
set(LANG_TARGETS "")

set(LANG_PAK_NAME "winghex_tr.pak")
add_definitions(-DLANG_PAK_NAME="${LANG_PAK_NAME}")

foreach(TS_FILE IN LISTS TS_FILES)
    get_filename_component(TS_DIR ${TS_FILE} DIRECTORY)
    get_filename_component(LANG_NAME "${TS_DIR}" NAME)
    set(QM_DIR "${CMAKE_CURRENT_BINARY_DIR}/.tmp/${TS_DIR}")
    set(QM_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TS_DIR}")
    set(LANG_NEED_COPY_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/${TS_DIR}")
    set(LANG_NEED_COPY
        "${LANG_NEED_COPY_PREFIX}/about.md"
        "${LANG_NEED_COPY_PREFIX}/components.md"
        "${LANG_NEED_COPY_PREFIX}/credits.md"
        "${LANG_NEED_COPY_PREFIX}/devs.md"
        "${LANG_NEED_COPY_PREFIX}/trans.md")
    add_custom_target(
        copy_lang_files
        COMMAND ${CMAKE_COMMAND} -E copy ${LANG_NEED_COPY} ${QM_DIR}
        COMMENT "Copying files to ${LANG_DEST}")

    set(LANG_PAK "${QM_DIR}/${LANG_PAK_NAME}")

    set(LANG_COMPRESS_CONTENT "winghex.qm" "about.md" "components.md"
                              "credits.md" "devs.md" "trans.md")

    add_custom_target(
        pak_lang_files
        COMMAND ${CMAKE_COMMAND} -E tar cf ${LANG_PAK_NAME} --format=zip
                ${LANG_COMPRESS_CONTENT}
        WORKING_DIRECTORY "${QM_DIR}"
        COMMENT "Compressing ${TS_FILE} into ${LANG_PAK_NAME}"
        DEPENDS ${QM_FILES} copy_lang_files)

    set(TARGET_NAME "langgen_${LANG_NAME}")
    add_custom_target(
        ${TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${LANG_PAK_NAME} ${QM_OUT_DIR}
        WORKING_DIRECTORY "${QM_DIR}"
        COMMENT "Copy ${TS_FILE} into ${LANG_PAK_NAME}"
        DEPENDS pak_lang_files)

    list(APPEND LANG_TARGETS ${TARGET_NAME})
endforeach()

add_custom_target(post_build_tasks ALL DEPENDS ${LANG_TARGETS})

set(PROJECT_SOURCES
    main.cpp
    src/utilities.h
    src/dbghelper.h
    ${QPATHEDIT_SRC}
    ${WIDGET_FRAME_SRC}
    ${RIBBON_SRC}
    ${CLASS_SRC}
    ${MODEL_SRC}
    ${DIALOG_SRC}
    ${PLUGIN_SRC}
    ${CONTROL_SRC}
    ${SETTING_SRC}
    ${SCRIPT_ADDON_SRC}
    ${ANGEL_SCRIPT_ADDON}
    ${CODEEDIT_WIDGET}
    ${QM_FILES}
    theme/breeze.qrc
    resources.qrc)

add_custom_target(
    build-time-make-directory ALL
    COMMAND
        ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/lang"
        "${CMAKE_CURRENT_BINARY_DIR}/scripts"
        "${CMAKE_CURRENT_BINARY_DIR}/.tmp")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(WIN32)
        set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
        qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION
                          ${PROJECT_SOURCES} ${app_icon_resource_windows})
    else()
        qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION
                          ${PROJECT_SOURCES})
    endif()
else()
    if(ANDROID)
        message(FATAL_ERROR "Android is not supported!")
    else()
        if(WIN32)
            set(app_icon_resource_windows
                "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
            add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES}
                                                 ${app_icon_resource_windows})
        else()
            add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})
        endif()
    endif()
endif()

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Concurrent
            Qt${QT_VERSION_MAJOR}::PrintSupport
            Qt${QT_VERSION_MAJOR}::GuiPrivate
            Qt${QT_VERSION_MAJOR}::CorePrivate
            SingleApplication::SingleApplication
            QWKCore
            QWKWidgets
            QHexView
            QCodeEditor2
            QCodeModel2
            QJsonModel
            angelscript
            QConsoleWidget
            qt${QT_VERSION_MAJOR}advanceddocking)

target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
                                  "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1. If
# you are developing for iOS or macOS you should consider setting an explicit,
# fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER
                         com.wingsummer.WingHexExplorer2)
endif()
set_target_properties(
    WingHexExplorer2
    PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
               ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} MACOSX_BUNDLE
               TRUE WIN32_EXECUTABLE
               TRUE)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WingHexExplorer2)
endif()
