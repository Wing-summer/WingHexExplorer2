cmake_minimum_required(VERSION 3.20)

project(WingHexExplorer2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_VERSION "2.3.0")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Network Concurrent
                                              PrintSupport Xml LinguistTools)
find_package(
    Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Concurrent
                                              PrintSupport Xml LinguistTools)

message("Build ${PROJECT_NAME} with ${CMAKE_BUILD_TYPE}.")

install(CODE "set(CMAKE_INSTALL_LOCAL_ONLY TRUE)" ALL_COMPONENTS)

option(WINGHEX_USE_FRAMELESS ON)
option(BUILD_TEST_PLUGIN OFF)
option(BUILD_SHARED_MEM_EXT OFF)
option(ANGEL_LSP ON)

add_definitions(-DAS_NO_THREADS)
add_definitions(-DWING_SYSTEM_NAME="${CMAKE_SYSTEM_NAME}")

if(DEFINED CMAKE_CXX_BYTE_ORDER AND CMAKE_CXX_BYTE_ORDER)
    if(CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
        add_definitions(-DWING_LITTLE_ENDIAN=0)
    elseif(CMAKE_CXX_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
        add_definitions(-DWING_LITTLE_ENDIAN=1)
    else()
        message(
            FATAL_ERROR
                "CMAKE_CXX_BYTE_ORDER defined but unknown value: ${CMAKE_CXX_BYTE_ORDER}"
        )
    endif()
else()
    message(FATAL_ERROR "CMAKE_CXX_BYTE_ORDER not set or empty")
endif()

if(BUILD_TEST_PLUGIN)
    add_subdirectory(TestPlugin)
    add_subdirectory(TestBadPlugin)
    add_subdirectory(TestHexExt)
    add_subdirectory(TestManager)
endif()

if(BUILD_SHARED_MEM_EXT)
    add_subdirectory(ShareMemoryDrv)
endif()

add_definitions(-DWINGHEX_VERSION="${PROJECT_VERSION}"
                -DAPP_ORG="WingCloudStudio" -DAPP_NAME="${PROJECT_NAME}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/WINGHEX_VERSION" ${PROJECT_VERSION})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/QT_VERSION" ${QT_VERSION_MAJOR})

if(WIN32)
    find_package(QT NAMES Qt6 Qt5 REQUIRED AxContainer)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED AxContainer)
    add_definitions(-DNOMINMAX)
endif()

if(${QT_VERSION_MAJOR} EQUAL 6)
    if(Qt6Widgets_VERSION VERSION_LESS 6.6.2)
        message(FATAL_ERROR "Minimum supported Qt6 version is 6.6.2!")
    endif()
else()
    message(
        FATAL_ERROR "Qt${QT_VERSION_MAJOR} has been dropped and not supported!")
endif()

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /utf-8")
    string(APPEND CMAKE_C_FLAGS " /utf-8")
endif()

if(WINGHEX_USE_FRAMELESS)
    option(QWINDOWKIT_BUILD_STATIC "Build static libraries" TRUE)
    option(QWINDOWKIT_INSTALL "Install library" OFF)
    add_subdirectory(3rdparty/qwindowkit)
    add_compile_definitions(WINGHEX_USE_FRAMELESS)
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(ADS_VERSION 4.3.1)
option(BUILD_EXAMPLES "Build the examples" FALSE)
option(BUILD_STATIC "Build the static library" TRUE)

add_subdirectory(3rdparty/cpptrace)
add_subdirectory(3rdparty/QHexView)
add_subdirectory(3rdparty/WingCodeEdit)
add_subdirectory(3rdparty/Qt-Advanced-Docking-System)
add_subdirectory(3rdparty/AngelScript/sdk/angelscript/projects/cmake)

set(DISABLE_WARNINGS ON)
set(ANTLR4_INSTALL OFF)
set(ANTLR_BUILD_CPP_TESTS OFF)
set(ANTLR_BUILD_SHARED OFF)
set(ANTLR_BUILD_STATIC ON)
set(WITH_STATIC_CRT OFF)
add_subdirectory(3rdparty/antlr4-cpp)

set(NUMCAL_GRAMMAR
    src/grammar/NumCal/NumCalBaseVisitor.cpp
    src/grammar/NumCal/NumCalBaseVisitor.h
    src/grammar/NumCal/NumCalLexer.cpp
    src/grammar/NumCal/NumCalLexer.h
    src/grammar/NumCal/NumCalParser.cpp
    src/grammar/NumCal/NumCalParser.h
    src/grammar/NumCal/NumCalVisitor.cpp
    src/grammar/NumCal/NumCalVisitor.h)

set(CSTRUCT_GRAMMAR
    src/grammar/c/CStructVisitor.h
    src/grammar/c/CStructBaseVisitor.cpp
    src/grammar/c/CStructLexer.cpp
    src/grammar/c/CStructParser.cpp
    src/grammar/c/CStructVisitor.cpp
    src/grammar/c/CStructBaseVisitor.h
    src/grammar/c/CStructLexer.h
    src/grammar/c/CStructParser.h)

set(ASCONSOLE_GRAMMAR
    src/grammar/ASConsole/AngelscriptConsoleLexer.cpp
    src/grammar/ASConsole/AngelscriptConsoleParser.cpp
    src/grammar/ASConsole/AngelscriptConsoleParserBaseVisitor.cpp
    src/grammar/ASConsole/AngelscriptConsoleParserVisitor.cpp
    src/grammar/ASConsole/AngelscriptConsoleLexer.h
    src/grammar/ASConsole/AngelscriptConsoleParser.h
    src/grammar/ASConsole/AngelscriptConsoleParserBaseVisitor.h
    src/grammar/ASConsole/AngelscriptConsoleParserVisitor.h)

set(SNIPPET_GRAMMAR
    src/grammar/Snippet/SnippetVisitor.h
    src/grammar/Snippet/SnippetVisitor.cpp
    src/grammar/Snippet/SnippetParser.h
    src/grammar/Snippet/SnippetParser.cpp
    src/grammar/Snippet/SnippetLexer.h
    src/grammar/Snippet/SnippetLexer.cpp
    src/grammar/Snippet/SnippetBaseVisitor.h
    src/grammar/Snippet/SnippetBaseVisitor.cpp)

set(ANTLR4_GRAMMAR ${NUMCAL_GRAMMAR} ${CSTRUCT_GRAMMAR} ${ASCONSOLE_GRAMMAR}
                   ${SNIPPET_GRAMMAR})

set(ANGEL_SCRIPT_ADDON_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/AngelScript/sdk/add_on")
set(ANGEL_SCRIPT_ADDON
    ${ANGEL_SCRIPT_ADDON_ROOT}/autowrapper/aswrappedcall.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/contextmgr/contextmgr.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/contextmgr/contextmgr.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptarray/scriptarray.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptarray/scriptarray.h
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
    ${ANGEL_SCRIPT_ADDON_ROOT}/weakref/weakref.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/datetime/datetime.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/datetime/datetime.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfile.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfile.h
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfilesystem.cpp
    ${ANGEL_SCRIPT_ADDON_ROOT}/scriptfile/scriptfilesystem.h)

add_subdirectory(3rdparty/qtsingleapplication)
add_subdirectory(3rdparty/fmt)

set(AS_DEBUGGER_SRC
    3rdparty/as-debugger/as_debugger.cpp 3rdparty/as-debugger/as_debugger.h
    3rdparty/as-debugger/as_helpers.cpp 3rdparty/as-debugger/as_helpers.h)

set(RIBBON_SRC
    3rdparty/QWingRibbon/ribbon.cpp
    3rdparty/QWingRibbon/ribbon.h
    3rdparty/QWingRibbon/ribbonbuttongroup.cpp
    3rdparty/QWingRibbon/ribbonbuttongroup.h
    3rdparty/QWingRibbon/ribbonbuttongroup.ui
    3rdparty/QWingRibbon/ribbontabcontent.cpp
    3rdparty/QWingRibbon/ribbontabcontent.h
    3rdparty/QWingRibbon/ribbontabcontent.ui)

set(QCONSOLEWIDGET_SRC
    3rdparty/QConsoleWidget/QConsoleIODevice.cpp
    3rdparty/QConsoleWidget/QConsoleIODevice.h
    3rdparty/QConsoleWidget/QConsoleWidget.cpp
    3rdparty/QConsoleWidget/QConsoleWidget.h
    3rdparty/QConsoleWidget/commandhistorymanager.h
    3rdparty/QConsoleWidget/commandhistorymanager.cpp)

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
    src/dialog/metadialog.cpp
    src/dialog/metadialog.h
    src/dialog/scriptingdialog.h
    src/dialog/scriptingdialog.cpp
    src/dialog/finddialog.cpp
    src/dialog/finddialog.h
    src/dialog/checksumdialog.h
    src/dialog/checksumdialog.cpp
    src/dialog/colorpickerdialog.h
    src/dialog/colorpickerdialog.cpp
    src/dialog/colorpickerdialog.ui
    src/dialog/splashdialog.ui
    src/dialog/splashdialog.cpp
    src/dialog/splashdialog.h
    src/dialog/historydeldialog.h
    src/dialog/historydeldialog.cpp
    src/dialog/historydeldialog.ui
    src/dialog/crashreport.h
    src/dialog/crashreport.cpp
    src/dialog/crashreport.ui
    src/dialog/showtextdialog.cpp
    src/dialog/showtextdialog.h
    src/dialog/definitiondownload.cpp
    src/dialog/definitiondownload.h)

set(STRUCT_LIB_SRC
    src/structlib/cstructerrorlistener.cpp
    src/structlib/cstructerrorlistener.h
    src/structlib/cstructerrorstrategy.cpp
    src/structlib/cstructerrorstrategy.h
    src/structlib/cstructvisitorparser.cpp
    src/structlib/cstructvisitorparser.h
    src/structlib/ctypeparser.cpp
    src/structlib/ctypeparser.h
    src/structlib/structdefine.h)

set(CONTROL_SRC
    src/control/codeedit.h
    src/control/codeedit.cpp
    src/control/editorview.h
    src/control/editorview.cpp
    src/control/toast.h
    src/control/toast.cpp
    src/control/scriptingconsole.cpp
    src/control/scriptingconsole.h
    src/control/scriptingconsolebase.h
    src/control/scriptingconsolebase.cpp
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
    src/control/asobjtreewidget.h
    src/control/asobjtreewidget.cpp
    src/control/dockwidgettab.h
    src/control/dockwidgettab.cpp
    src/control/hexlineedit.h
    src/control/hexlineedit.cpp
    src/control/popupactionwidget.h
    src/control/popupactionwidget.cpp
    src/control/settingspopup.cpp
    src/control/settingspopup.h
    src/control/searchreplacewidget.cpp
    src/control/searchreplacewidget.h
    src/control/gotolinewidget.h
    src/control/gotolinewidget.cpp
    src/control/codeeditcontrolwidget.h
    src/control/codeeditcontrolwidget.cpp
    src/control/consolecodeedit.h
    src/control/consolecodeedit.cpp
    src/control/asidbtreeview.h
    src/control/asidbtreeview.cpp)

set(CLASS_SRC
    src/class/logger.cpp
    src/class/logger.h
    src/class/skinmanager.cpp
    src/class/skinmanager.h
    src/class/workspacemanager.cpp
    src/class/workspacemanager.h
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
    src/class/scriptmachine.h
    src/class/scriptmachine.cpp
    src/class/scriptmanager.h
    src/class/scriptmanager.cpp
    src/class/languagemanager.h
    src/class/languagemanager.cpp
    src/class/settingmanager.h
    src/class/settingmanager.cpp
    src/class/asdebugger.h
    src/class/asdebugger.cpp
    src/class/appmanager.h
    src/class/appmanager.cpp
    src/class/angelscripthelper.h
    src/class/ascompletion.cpp
    src/class/ascompletion.h
    src/class/asbuilder.h
    src/class/asbuilder.cpp
    src/class/aspreprocesser.h
    src/class/aspreprocesser.cpp
    src/class/layoutmanager.h
    src/class/layoutmanager.cpp
    src/class/wingupdater.h
    src/class/wingupdater.cpp
    src/class/richtextitemdelegate.h
    src/class/richtextitemdelegate.cpp
    src/class/showinshell.h
    src/class/showinshell.cpp
    src/class/dockcomponentsfactory.h
    src/class/dockcomponentsfactory.cpp
    src/class/diffutil.h
    src/class/diffutil.cpp
    src/class/crashhandler.h
    src/class/crashhandler.cpp
    src/class/pluginsystem.h
    src/class/pluginsystem.cpp
    src/class/inspectqtloghelper.h
    src/class/inspectqtloghelper.cpp
    src/class/codeinfotip.h
    src/class/codeinfotip.cpp
    src/class/wingconsolehighligher.h
    src/class/wingconsolehighligher.cpp
    src/class/asconsolecompletion.h
    src/class/asconsolecompletion.cpp
    src/class/scriptsettings.h
    src/class/scriptsettings.cpp
    src/class/wingangel.h
    src/class/wingangel.cpp
    src/class/winggeneric.h
    src/class/winggeneric.cpp
    src/class/changedstringlist.h
    src/class/changedstringlist.cpp
    src/class/editorviewcontext.h
    src/class/editorviewcontext.cpp
    src/class/consolehighlighanim.h
    src/class/consolehighlighanim.cpp
    src/class/angellsp.h
    src/class/angellsp.cpp
    src/class/lsp.h
    src/class/resettabletimer.h
    src/class/resettabletimer.cpp
    src/class/astype_evaluator.h
    src/class/angelscriptconsolevisitor.h
    src/class/angelscriptconsolevisitor.cpp
    src/class/aswingcache.h
    src/class/aswingcache.cpp
    src/class/lspeditorinterface.h
    src/class/snippetprocessor.h
    src/class/snippetprocessor.cpp)

set(INTERNAL_PLG_SRC src/class/wingangelapi.h src/class/wingangelapi.cpp
                     src/class/wingcstruct.h src/class/wingcstruct.cpp)

if(WINGHEX_USE_FRAMELESS)
    set(WIDGET_FRAME_SRC
        src/widgetframe/windowbar.cpp src/widgetframe/windowbar.h
        src/widgetframe/windowbar_p.h src/widgetframe/windowbutton.cpp
        src/widgetframe/windowbutton.h src/widgetframe/windowbutton_p.h)
    list(APPEND CLASS_SRC src/class/framelesshelper.h
         src/class/framelesshelper.cpp)
else()
    set(WIDGET_FRAME_SRC)
endif()

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
    src/model/dbgcallstackmodel.h
    src/model/dbgcallstackmodel.cpp
    src/model/codecompletionmodel.h
    src/model/codecompletionmodel.cpp
    src/model/asidbtreemodel.h
    src/model/asidbtreemodel.cpp
    src/model/asidbwatchmodel.h
    src/model/asidbwatchmodel.cpp)

set(SETTING_SRC
    src/settings/settings.h
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
    src/settings/othersettingsdialog.h
    src/settings/othersettingsdialog.cpp
    src/settings/othersettingsdialog.ui
    src/settings/lspsettingdialog.h
    src/settings/lspsettingdialog.cpp
    src/settings/lspsettingdialog.ui
    src/settings/qeditconfig.h
    src/settings/qeditconfig.cpp
    src/settings/qeditconfig.ui)

set(SCRIPT_ADDON_SRC
    src/scriptaddon/scriptqstring.h
    src/scriptaddon/scriptqstring.cpp
    src/scriptaddon/scriptqdictionary.h
    src/scriptaddon/scriptqdictionary.cpp
    src/scriptaddon/scriptregex.h
    src/scriptaddon/scriptregex.cpp
    src/scriptaddon/scriptcolor.h
    src/scriptaddon/scriptcolor.cpp
    src/scriptaddon/scriptjson.h
    src/scriptaddon/scriptjson.cpp
    src/scriptaddon/scriptenv.h
    src/scriptaddon/scriptenv.cpp
    src/scriptaddon/scriptany.cpp
    src/scriptaddon/scriptany.h)

# localization support
file(
    GLOB_RECURSE TS_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/lang/*/winghex_*.ts")

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
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Qt-Advanced-Docking-System/src
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/QWingRibbon
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

qt_create_translation(QM_FILES ${TRANSLATION_PATH} ${TS_FILES} OPTIONS
                      -no-obsolete)

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
    set(LANG_NEED_COPY "${LANG_NEED_COPY_PREFIX}/about.md"
                       "${LANG_NEED_COPY_PREFIX}/devs.md")

    add_custom_target(
        copy_lang_files_${LANG_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${LANG_NEED_COPY} ${QM_DIR}
        COMMENT "Copying files to ${LANG_DEST}")

    set(LANG_PAK "${QM_DIR}/${LANG_PAK_NAME}")

    set(LANG_COMPRESS_CONTENT "winghex_${LANG_NAME}.qm" "about.md" "devs.md")

    add_custom_target(
        pak_lang_files_${LANG_NAME}
        COMMAND ${CMAKE_COMMAND} -E tar cf ${LANG_PAK_NAME} --format=zip
                ${LANG_COMPRESS_CONTENT}
        WORKING_DIRECTORY "${QM_DIR}"
        COMMENT "Compressing ${TS_FILE} into ${LANG_PAK_NAME}"
        DEPENDS ${QM_FILES} copy_lang_files_${LANG_NAME})

    set(TARGET_NAME "langgen_${LANG_NAME}")
    add_custom_target(
        ${TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${LANG_PAK_NAME} ${QM_OUT_DIR}
        WORKING_DIRECTORY "${QM_DIR}"
        COMMENT "Copy ${TS_FILE} into ${LANG_PAK_NAME}"
        DEPENDS pak_lang_files_${LANG_NAME})

    list(APPEND LANG_TARGETS ${TARGET_NAME})
endforeach()

add_custom_target(post_build_tasks ALL DEPENDS ${LANG_TARGETS})

set(PROJECT_SOURCES
    main.cpp
    src/utilities.h
    src/dbghelper.h
    src/define.h
    src/predefgen.h
    ${QCONSOLEWIDGET_SRC}
    ${WIDGET_FRAME_SRC}
    ${RIBBON_SRC}
    ${STRUCT_LIB_SRC}
    ${CLASS_SRC}
    ${AS_DEBUGGER_SRC}
    ${INTERNAL_PLG_SRC}
    ${MODEL_SRC}
    ${DIALOG_SRC}
    ${CONTROL_SRC}
    ${SETTING_SRC}
    ${SCRIPT_ADDON_SRC}
    ${ANGEL_SCRIPT_ADDON}
    ${QM_FILES}
    ${ANTLR4_GRAMMAR}
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
        find_package(Qt6 REQUIRED COMPONENTS Core)

        qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION
                          ${PROJECT_SOURCES} ${app_icon_resource_windows})

        target_link_libraries(WingHexExplorer2 PRIVATE Qt6::Core)
    else()
        qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION
                          ${PROJECT_SOURCES})
    endif()
endif()

target_compile_options(
    WingHexExplorer2
    PRIVATE $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<CONFIG:Release>>:-g>
            $<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:/Zi>>)

target_link_options(WingHexExplorer2 PRIVATE
                    $<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:/DEBUG>>)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Concurrent
            Qt${QT_VERSION_MAJOR}::PrintSupport
            Qt${QT_VERSION_MAJOR}::GuiPrivate
            Qt${QT_VERSION_MAJOR}::CorePrivate
            Qt${QT_VERSION_MAJOR}::Xml
            cpptrace::cpptrace
            QtSingleApplication::QtSingleApplication
            WingPlugin
            QHexView
            WingCodeEdit
            angelscript
            fmt::fmt
            antlr4_static
            qtadvanceddocking-qt${QT_VERSION_MAJOR})

if(WINGHEX_USE_FRAMELESS)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE QWKCore QWKWidgets)
endif()

if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME}
                          PRIVATE Qt${QT_VERSION_MAJOR}::AxContainer)
endif()

# emit marco in Qt conflicts with antlr4-cpp
target_compile_definitions(WingHexExplorer2 PRIVATE QT_NO_EMIT)

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

add_dependencies(WingHexExplorer2 post_build_tasks build-time-make-directory)

if(${QT_VERSION_MAJOR} EQUAL 6)
    if(Qt6Widgets_VERSION VERSION_GREATER_EQUAL 6.5.0)
        install(
            TARGETS WingHexExplorer2
            BUNDLE DESTINATION .
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

        # Generate the deployment script.
        qt6_generate_deploy_app_script(
            TARGET WingHexExplorer2 OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR)

        # Call the deployment script on "cmake --install".
        install(SCRIPT ${deploy_script})
    endif()
endif()
add_subdirectory(WingPlugin)

include(CheckIPOSupported)
check_ipo_supported(RESULT _ipo_supported OUTPUT _ipo_error)
if(_ipo_supported)
    set_property(TARGET WingHexExplorer2
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
    set_property(TARGET WingHexExplorer2
                 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

target_compile_options(
    WingHexExplorer2
    PRIVATE
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-ffunction-sections
        -fdata-sections
        -fvisibility=hidden
        -fvisibility-inlines-hidden>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-ffunction-sections
        -fdata-sections
        -fvisibility=hidden
        -fvisibility-inlines-hidden>)

target_link_options(
    WingHexExplorer2
    PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-Wl,--gc-sections>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-Wl,--gc-sections>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/OPT:REF
    /OPT:ICF>)

if(ANGEL_LSP)
    set(NODE_SUBDIR "${CMAKE_SOURCE_DIR}/3rdparty/angel-lsp/server")
    set(BUILD_BIN_DIR "${CMAKE_BINARY_DIR}/lsp")
    set(BUILD_BIN_BUILD_DIR "${CMAKE_BINARY_DIR}/lsp-build")

    if(WIN32)
        set(OUT_NAME "angelscript-ls.exe")
    else()
        set(OUT_NAME "angelscript-ls")
    endif()

    set(SRC_JS "${NODE_SUBDIR}/src/server.ts")
    set(DIST_JS "${BUILD_BIN_BUILD_DIR}/dist/angelscript-language-server.js")
    set(OUT_BIN "${BUILD_BIN_DIR}/${OUT_NAME}")

    find_program(NODE_EXE NAMES node)
    find_program(NPM_EXE NAMES npm)
    find_program(NPX_EXE NAMES npx)

    if(NOT NODE_EXE)
        message(FATAL_ERROR "Node.js not found. Install Node >= 18.")
    endif()
    if(NOT NPM_EXE)
        message(FATAL_ERROR "npm not found. Install npm.")
    endif()
    if(NOT NPX_EXE)
        message(FATAL_ERROR "npx not found. Install npx.")
    endif()

    message(STATUS "Found node: ${NODE_EXE}")
    message(STATUS "Found npm:  ${NPM_EXE}")

    if(WIN32)
        set(PKG_TARGET "node18-win-x64")
    elseif(APPLE)
        set(PKG_TARGET "node18-macos-x64")
    else()
        set(PKG_TARGET "node18-linux-x64")
    endif()

    set(ESBUILD_CMD
        ${NPX_EXE}
        esbuild
        "${SRC_JS}"
        --bundle
        --platform=node
        --target=node18
        "--outfile=${DIST_JS}"
        --minify)

    set(PKG_CMD
        ${NPX_EXE}
        pkg
        "${DIST_JS}"
        --targets
        ${PKG_TARGET}
        --output
        "${OUT_BIN}")

    add_custom_command(
        OUTPUT ${DIST_JS}
        WORKING_DIRECTORY ${NODE_SUBDIR}
        COMMAND ${CMAKE_COMMAND} -E echo "[1] npm ci (local deps)"
        COMMAND ${NPM_EXE} ci --silent
        COMMAND ${CMAKE_COMMAND} -E echo "[2] building with esbuild"
        COMMAND ${NPX_EXE} tsc
        COMMAND ${ESBUILD_CMD}
        COMMENT "Building JS bundle with esbuild -> ${DIST_JS}"
        VERBATIM)

    add_custom_command(
        OUTPUT ${OUT_BIN}
        DEPENDS ${DIST_JS}
        WORKING_DIRECTORY ${NODE_SUBDIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_BIN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_BIN_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "[3] packaging with pkg/npm exec/npx"
        COMMAND ${PKG_CMD}
        COMMENT "Packaging with pkg"
        VERBATIM)
    add_custom_target(build_angel_lsp ALL DEPENDS ${OUT_BIN})
    add_dependencies(WingHexExplorer2 build_angel_lsp)
endif()
