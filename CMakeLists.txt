cmake_minimum_required(VERSION 3.5)

project(
  WingHexExplorer2
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Concurrent
                                                  PrintSupport LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED
             COMPONENTS Widgets Network Concurrent PrintSupport LinguistTools)

add_definitions(-DWINGHEX_VERSION="${CMAKE_PROJECT_VERSION}")

if(WIN32)
  add_definitions(-DNOMINMAX)
endif()

if(${QT_VERSION_MAJOR} EQUAL 5)
  if(Qt5Widgets_VERSION VERSION_LESS 5.12.0)
    message(FATAL_ERROR "Minimum supported Qt5 version is 5.10!")
  endif()
endif()

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /utf-8")
  string(APPEND CMAKE_C_FLAGS " /utf-8")
endif()

add_subdirectory(QLogger)
add_subdirectory(QHexView)
add_subdirectory(QScintilla)
add_subdirectory(Qt-Advanced-Docking-System)
add_subdirectory(AngelScript/angelscript/projects/cmake)

set(QAPPLICATION_CLASS
    QApplication
    CACHE STRING "Inheritance class for SingleApplication")
add_subdirectory(SingleApplication)

set(RIBBON_SRC
    QWingRibbon/ribbon.cpp
    QWingRibbon/ribbon.h
    QWingRibbon/ribbonbuttongroup.cpp
    QWingRibbon/ribbonbuttongroup.h
    QWingRibbon/ribbonbuttongroup.ui
    QWingRibbon/ribbontabcontent.cpp
    QWingRibbon/ribbontabcontent.h
    QWingRibbon/ribbontabcontent.ui)

set(QPATHEDIT_SRC QPathEdit/qpathedit.cpp QPathEdit/qpathedit.h)

set(DIALOG_SRC
    src/dialog/mainwindow.cpp
    src/dialog/mainwindow.h
    src/dialog/settingdialog.cpp
    src/dialog/settingdialog.h
    src/dialog/settingdialog.ui
    src/dialog/framelesswindowbase.h
    src/dialog/framelesswindowbase.cpp
    src/dialog/framelessmainwindow.h
    src/dialog/framelessmainwindow.cpp
    src/dialog/framelessdialog.h
    src/dialog/framelessdialog.cpp
    src/dialog/sponsordialog.h
    src/dialog/sponsordialog.cpp
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
    src/dialog/checksumdialog.cpp)

set(CONTROL_SRC
    src/control/titlebar.h
    src/control/titlebar.cpp
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
    src/control/scripteditor.cpp)

set(CLASS_SRC
    src/class/logger.cpp
    src/class/logger.h
    src/class/skinmanager.cpp
    src/class/skinmanager.h
    src/class/workspacemanager.cpp
    src/class/workspacemanager.h
    src/class/setting.h
    src/class/eventfilter.h
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
    src/class/wingcolordialog.h
    src/class/wingcolordialog.cpp
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
    src/class/qscilexeras.h
    src/class/qscilexeras.cpp)

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
    src/settings/scriptsettingdialog.ui)

set(PLUGIN_SRC
    src/plugin/iwingplugin.h src/plugin/pluginsystem.cpp
    src/plugin/pluginsystem.h src/plugin/settingpage.h
    src/plugin/settingpage.cpp)

set(TS_FILES lang/ws_zh_CN.ts)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "./lang")
if(${QT_VERSION_MAJOR} EQUAL 5)
  qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} OPTIONS
                         -no-obsolete)
elseif(${QT_VERSION_MAJOR} EQUAL 6)
  qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} OPTIONS
                         -no-obsolete)
else()
  message(FATAL_ERROR "Unsupported QT version")
endif()

set(PROJECT_SOURCES
    main.cpp
    src/define.h
    src/utilities.h
    src/dbghelper.h
    src/framelesswindow.h
    ${QPATHEDIT_SRC}
    ${RIBBON_SRC}
    ${CLASS_SRC}
    ${DIALOG_SRC}
    ${PLUGIN_SRC}
    ${CONTROL_SRC}
    ${SETTING_SRC}
    ${QM_FILES}
    theme/breeze.qrc
    resources.qrc)

add_custom_target(
  build-time-make-directory ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/lang"
          "${CMAKE_BINARY_DIR}/scripts")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  if(WIN32)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
    qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION ${PROJECT_SOURCES}
                      ${app_icon_resource_windows})
  else()
    qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION ${PROJECT_SOURCES})
  endif()
else()
  if(ANDROID)
    message(FATAL_ERROR "Android is not supported!")
  else()
    if(WIN32)
      set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
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
          SingleApplication::SingleApplication
          QHexView
          QLoggerLib
          QScintilla
          angelscript
          qt${QT_VERSION_MAJOR}advanceddocking)

target_include_directories(${CMAKE_PROJECT_NAME}
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

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
