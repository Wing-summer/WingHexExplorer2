cmake_minimum_required(VERSION 3.20)

project(WingHexExplorer2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(WINGHEX_FORCE_CPP17 "Force using C++17" OFF)

if(NOT WINGHEX_FORCE_CPP17 AND "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "Compile with C++20")
else()
    set(CMAKE_CXX_STANDARD 17)
    message(STATUS "Compile with C++17")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_VERSION "2.3.8")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Concurrent PrintSupport
                                     Xml LinguistTools)

if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    set(QT_CREATOR_SKIP_MAINTENANCE_TOOL_PROVIDER ON)
    set(QT_NO_PRIVATE_MODULE_WARNING ON)
    find_package(Qt6 REQUIRED GuiPrivate CorePrivate WidgetsPrivate)
endif()

find_package(OpenSSL REQUIRED)

message("Build ${PROJECT_NAME} with ${CMAKE_BUILD_TYPE}.")

install(CODE "set(CMAKE_INSTALL_LOCAL_ONLY TRUE)" ALL_COMPONENTS)

option(WINGHEX_USE_FRAMELESS "Enable custom titlebar support" ON)
option(WINGHEX_BUILD_TEST_PLUGIN "Build test plugins" OFF)
option(WINGHEX_BUILD_SHARED_MEM_EXT "Build ShareMemoryDrv plugin" OFF)
option(WINGHEX_OPTIMIZE_FOR_NATIVE
       "Build with native optimization(less portable)" OFF)

add_definitions(-DAS_NO_THREADS)
add_definitions(-DWING_SYSTEM_NAME="${CMAKE_SYSTEM_NAME}")

include(src/WingPlugin.cmake)

if(WINGHEX_BUILD_TEST_PLUGIN)
    add_subdirectory(TestPlugin)
    add_subdirectory(TestDepPlugin)
endif()

if(WINGHEX_BUILD_SHARED_MEM_EXT)
    add_subdirectory(ShareMemoryDrv)
endif()

if(WINGHEX_OPTIMIZE_FOR_NATIVE)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
    if(COMPILER_SUPPORTS_MARCH_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    else()
        check_cxx_compiler_flag("-mcpu=native" COMPILER_SUPPORTS_MCPU)
        if(COMPILER_SUPPORTS_MCPU)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=native")
        endif()
    endif()
endif()

add_definitions(-DWINGHEX_VERSION="${PROJECT_VERSION}"
                -DAPP_ORG="WingCloudStudio" -DAPP_NAME="${PROJECT_NAME}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/WINGHEX_VERSION" ${PROJECT_VERSION})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/QT_VERSION" "6")

if(WIN32)
    find_package(Qt6 REQUIRED AxContainer)
    add_definitions(-DNOMINMAX)
endif()

if(LINUX)
    find_package(Qt6 REQUIRED DBus)
endif()

if(Qt6_VERSION VERSION_LESS 6.6.2)
    message(FATAL_ERROR "Minimum supported Qt6 version is 6.6.2!")
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
set(ADS_VERSION 4.5.0)
option(BUILD_EXAMPLES "Build the examples" FALSE)
option(BUILD_STATIC "Build the static library" TRUE)

add_subdirectory(3rdparty/cpptrace)
add_subdirectory(3rdparty/QHexView)
add_subdirectory(3rdparty/WingCodeEdit)

set(LUABRIDGE_TESTING OFF)
set(LUABRIDGE_COVERAGE OFF)
set(LUABRIDGE_BENCHMARKS OFF)
add_subdirectory(3rdparty/LuaBridge3)
target_compile_definitions(
    LuaBridge INTERFACE LUABRIDGE_STRICT_STACK_CONVERSIONS=1
                        LUABRIDGE_RAISE_UNREGISTERED_CLASS_USAGE=1)

add_subdirectory(3rdparty/Qt-Advanced-Docking-System)

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

set(SNIPPET_GRAMMAR
    src/grammar/Snippet/SnippetBaseVisitor.cpp
    src/grammar/Snippet/SnippetBaseVisitor.h
    src/grammar/Snippet/SnippetLexer.cpp
    src/grammar/Snippet/SnippetLexer.h
    src/grammar/Snippet/SnippetParser.cpp
    src/grammar/Snippet/SnippetParser.h
    src/grammar/Snippet/SnippetVisitor.cpp
    src/grammar/Snippet/SnippetVisitor.h)

set(ANTLR4_GRAMMAR ${NUMCAL_GRAMMAR} ${SNIPPET_GRAMMAR})

# these settings commted out because cffi-luau will add luau subdirectory automatically
# set(LUAU_BUILD_CLI OFF)
# set(LUAU_BUILD_TESTS OFF)
# set(LUAU_BUILD_WEB OFF)
set(LUAU_WERROR OFF)
set(LUAU_STATIC_CRT OFF)
# set(LUAU_EXTERN_C OFF)
set(LUAU_BUILD_SHARED OFF)
# add_subdirectory(3rdparty/luau) # will added by cffi-luau
add_subdirectory(3rdparty/qtsingleapplication)

set(CFFI_BUILD_TESTS OFF)
set(CFFI_USE_SYSTEM_LIBFFI OFF)
set(CFFI_LIBFFI_GIT_REPOSITORY
    "https://github.com/Wing-summer/ffi"
    CACHE INTERNAL "Git URL of the CMake-buildable libffi fork to fetch" FORCE)
add_subdirectory(3rdparty/cffi-luau)
target_compile_definitions(libffi PRIVATE NO_JAVA_RAW_API)

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
    3rdparty/QConsoleWidget/commandhistorymanager.cpp
    3rdparty/QConsoleWidget/commandhistorymanager.h
    3rdparty/QConsoleWidget/QConsoleIODevice.cpp
    3rdparty/QConsoleWidget/QConsoleIODevice.h
    3rdparty/QConsoleWidget/QConsoleWidget.cpp
    3rdparty/QConsoleWidget/QConsoleWidget.h)

set(DIALOG_SRC
    src/dialog/aboutsoftwaredialog.cpp
    src/dialog/aboutsoftwaredialog.h
    src/dialog/aboutsoftwaredialog.ui
    src/dialog/checksumdialog.cpp
    src/dialog/checksumdialog.h
    src/dialog/colorpickerdialog.cpp
    src/dialog/colorpickerdialog.h
    src/dialog/colorpickerdialog.ui
    src/dialog/crashreport.cpp
    src/dialog/crashreport.h
    src/dialog/crashreport.ui
    src/dialog/encodingdialog.cpp
    src/dialog/encodingdialog.h
    src/dialog/fileinfodialog.cpp
    src/dialog/fileinfodialog.h
    src/dialog/finddialog.cpp
    src/dialog/finddialog.h
    src/dialog/framelessdialogbase.cpp
    src/dialog/framelessdialogbase.h
    src/dialog/framelessmainwindow.cpp
    src/dialog/framelessmainwindow.h
    src/dialog/historydeldialog.cpp
    src/dialog/historydeldialog.h
    src/dialog/historydeldialog.ui
    src/dialog/layoutdeldialog.cpp
    src/dialog/layoutdeldialog.h
    src/dialog/layoutdeldialog.ui
    src/dialog/mainwindow.cpp
    src/dialog/mainwindow.h
    src/dialog/metadialog.cpp
    src/dialog/metadialog.h
    src/dialog/metaheaderdialog.cpp
    src/dialog/metaheaderdialog.h
    src/dialog/mutisavedialog.cpp
    src/dialog/mutisavedialog.h
    src/dialog/mutisavedialog.ui
    src/dialog/scriptingdialog.cpp
    src/dialog/scriptingdialog.h
    src/dialog/settingdialog.cpp
    src/dialog/settingdialog.h
    src/dialog/settingdialog.ui
    src/dialog/splashdialog.cpp
    src/dialog/splashdialog.h
    src/dialog/splashdialog.ui)

set(CONTROL_SRC
    src/control/idbtreeview.cpp
    src/control/idbtreeview.h
    src/control/objtreewidget.cpp
    src/control/objtreewidget.h
    src/control/codeedit.cpp
    src/control/codeedit.h
    src/control/codeeditcontrolwidget.cpp
    src/control/codeeditcontrolwidget.h
    src/control/dockwidgettab.cpp
    src/control/dockwidgettab.h
    src/control/editorview.cpp
    src/control/editorview.h
    src/control/gotolinewidget.cpp
    src/control/gotolinewidget.h
    src/control/gotowidget.cpp
    src/control/gotowidget.h
    src/control/gotowidget.ui
    src/control/hexlineedit.cpp
    src/control/hexlineedit.h
    src/control/huecolorpickerslider.cpp
    src/control/huecolorpickerslider.h
    src/control/qcolorpicker_slider.cpp
    src/control/qcolorpicker_slider.hpp
    src/control/qtableviewext.cpp
    src/control/qtableviewext.h
    src/control/scripteditor.cpp
    src/control/scripteditor.h
    src/control/scriptinfobroswer.cpp
    src/control/scriptinfobroswer.h
    src/control/scriptingconsole.cpp
    src/control/scriptingconsole.h
    src/control/scriptingconsolebase.cpp
    src/control/scriptingconsolebase.h
    src/control/searchreplacewidget.cpp
    src/control/searchreplacewidget.h
    src/control/settingspopup.cpp
    src/control/settingspopup.h
    src/control/toast.cpp
    src/control/toast.h)

set(CLASS_SRC
    src/class/appmanager.cpp
    src/class/appmanager.h
    src/class/ascompletion.cpp
    src/class/ascompletion.h
    src/class/asconsolecompletion.cpp
    src/class/asconsolecompletion.h
    src/class/changedstringlist.cpp
    src/class/changedstringlist.h
    src/class/codeinfotip.cpp
    src/class/codeinfotip.h
    src/class/compositeiconengine.cpp
    src/class/compositeiconengine.h
    src/class/consolehighlighanim.cpp
    src/class/consolehighlighanim.h
    src/class/crashhandler.cpp
    src/class/crashhandler.h
    src/class/cryptographichash.cpp
    src/class/cryptographichash.h
    src/class/dockcomponentsfactory.cpp
    src/class/dockcomponentsfactory.h
    src/class/editorinfo.h
    src/class/editorlspevent.cpp
    src/class/editorlspevent.h
    src/class/editorviewcontext.cpp
    src/class/editorviewcontext.h
    src/class/eventfilter.h
    src/class/fileaccesscheck.cpp
    src/class/fileaccesscheck.h
    src/class/inspectqtloghelper.cpp
    src/class/inspectqtloghelper.h
    src/class/luauscheduler.h
    src/class/luauscheduler.cpp
    src/class/languagemanager.cpp
    src/class/languagemanager.h
    src/class/layoutmanager.cpp
    src/class/layoutmanager.h
    src/class/logger.cpp
    src/class/logger.h
    src/class/lsp.h
    src/class/lspeditorinterface.cpp
    src/class/lspeditorinterface.h
    src/class/mutisavebuttondelegate.cpp
    src/class/mutisavebuttondelegate.h
    src/class/pluginsystem.cpp
    src/class/pluginsystem.h
    src/class/qkeysequences.cpp
    src/class/qkeysequences.h
    src/class/recentfilemanager.cpp
    src/class/recentfilemanager.h
    src/class/resettabletimer.cpp
    src/class/resettabletimer.h
    src/class/richtextitemdelegate.cpp
    src/class/richtextitemdelegate.h
    src/class/scriptmachine.cpp
    src/class/scriptmachine.h
    src/class/scriptmanager.cpp
    src/class/scriptmanager.h
    src/class/scriptsettings.cpp
    src/class/scriptsettings.h
    src/class/settingmanager.cpp
    src/class/settingmanager.h
    src/class/showinshell.cpp
    src/class/showinshell.h
    src/class/skinmanager.cpp
    src/class/skinmanager.h
    src/class/snippetprocessor.cpp
    src/class/snippetprocessor.h
    src/class/wingangel.cpp
    src/class/wingangel.h
    src/class/wingconsolehighligher.cpp
    src/class/wingconsolehighligher.h
    src/class/wingfiledialog.cpp
    src/class/wingfiledialog.h
    src/class/winginputdialog.cpp
    src/class/winginputdialog.h
    src/class/wingmessagebox.cpp
    src/class/wingmessagebox.h
    src/class/wingplugincert.cpp
    src/class/wingplugincert.h
    src/class/wingupdater.cpp
    src/class/wingupdater.h
    src/class/workspacemanager.cpp
    src/class/workspacemanager.h)

set(LUAU_DBG_SRC
    src/debugger/luauutil.h
    src/debugger/luauutil.cpp
    src/debugger/luaufile.h
    src/debugger/luaufile.cpp
    src/debugger/breakpoint.h
    src/debugger/breakpoint.cpp
    src/debugger/luauinspector.h
    src/debugger/luauinspector.cpp
    src/debugger/luauscope.h
    src/debugger/luauscope.cpp
    src/debugger/luaudebugger.h
    src/debugger/luaudebugger.cpp
    src/debugger/luautypes.h
    src/debugger/luautypes.cpp
    src/debugger/luauvariable.h
    src/debugger/luauvariable.cpp
    src/debugger/luauvariableregistry.h
    src/debugger/luauvariableregistry.cpp)

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
    src/model/idbtreemodel.cpp
    src/model/idbtreemodel.h
    src/model/idbwatchmodel.cpp
    src/model/idbwatchmodel.h
    src/model/bookmarksmodel.cpp
    src/model/bookmarksmodel.h
    src/model/checksummodel.cpp
    src/model/checksummodel.h
    src/model/codecompletionmodel.cpp
    src/model/codecompletionmodel.h
    src/model/dbgcallstackmodel.cpp
    src/model/dbgcallstackmodel.h
    src/model/findresultmodel.cpp
    src/model/findresultmodel.h
    src/model/metadatamodel.cpp
    src/model/metadatamodel.h
    src/model/numshowmodel.cpp
    src/model/numshowmodel.h)

set(SETTING_SRC
    src/settings/editorsettingdialog.cpp
    src/settings/editorsettingdialog.h
    src/settings/editorsettingdialog.ui
    src/settings/generalsettingdialog.cpp
    src/settings/generalsettingdialog.h
    src/settings/generalsettingdialog.ui
    src/settings/lspsettingdialog.cpp
    src/settings/lspsettingdialog.h
    src/settings/lspsettingdialog.ui
    src/settings/othersettingsdialog.cpp
    src/settings/othersettingsdialog.h
    src/settings/othersettingsdialog.ui
    src/settings/pluginsettingdialog.cpp
    src/settings/pluginsettingdialog.h
    src/settings/pluginsettingdialog.ui
    src/settings/qeditconfig.cpp
    src/settings/qeditconfig.h
    src/settings/qeditconfig.ui
    src/settings/scriptsettingdialog.cpp
    src/settings/scriptsettingdialog.h
    src/settings/scriptsettingdialog.ui
    src/settings/settings.h)

set(SCRIPT_ADDON_SRC
    # src/scriptaddon/scriptenv.cpp
    # src/scriptaddon/scriptenv.h
    # src/scriptaddon/scriptjson.cpp
    # src/scriptaddon/scriptjson.h
    # src/scriptaddon/scriptregex.cpp
    # src/scriptaddon/scriptregex.h
    # src/scriptaddon/scripturl.cpp
    # src/scriptaddon/scripturl.h
)

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

    set(LANG_COMPRESS_CONTENT "about.md" "devs.md" "winghex_${LANG_NAME}.qm")

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
    ${ANTLR4_GRAMMAR}
    ${CLASS_SRC}
    ${CONTROL_SRC}
    ${LUAU_DBG_SRC}
    ${DIALOG_SRC}
    ${MODEL_SRC}
    ${QCONSOLEWIDGET_SRC}
    ${QM_FILES}
    ${RIBBON_SRC}
    ${SCRIPT_ADDON_SRC}
    ${SETTING_SRC}
    ${WIDGET_FRAME_SRC}
    main.cpp
    resources.qrc
    src/dbghelper.h
    src/define.h
    src/predefgen.h
    src/utilities.h
    theme/breeze.qrc)

add_custom_target(
    build-time-make-directory ALL
    COMMAND
        ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/lang"
        "${CMAKE_CURRENT_BINARY_DIR}/scripts"
        "${CMAKE_CURRENT_BINARY_DIR}/.tmp")

if(WIN32)
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/favicon.rc")
    find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION ${PROJECT_SOURCES}
                      ${app_icon_resource_windows})

    target_link_libraries(WingHexExplorer2 PRIVATE Qt6::Core)
else()
    qt_add_executable(WingHexExplorer2 MANUAL_FINALIZATION ${PROJECT_SOURCES})
endif()

if(Qt6_VERSION VERSION_GREATER_EQUAL 6.7.0)
    target_compile_definitions(
        WingHexExplorer2
        PRIVATE QT_NO_CONTEXTLESS_CONNECT
                QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
                QT_RESTRICTED_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
endif()

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    PRIVATE Qt6::Widgets
            Qt6::Network
            Qt6::Concurrent
            Qt6::PrintSupport
            Qt6::Xml
            Qt6::GuiPrivate
            Qt6::CorePrivate
            OpenSSL::SSL
            OpenSSL::Crypto
            QtSingleApplication::QtSingleApplication
            cpptrace::cpptrace
            Luau.Compiler
            Luau.VM
            Luau.Ast
            Luau.Analysis
            Luau.CodeGen
            LuaBridge
            cffi
            antlr4_static
            qtadvanceddocking-qt6
            QHexView
            WingCodeEdit
            WingPlugin)

if(WINGHEX_USE_FRAMELESS)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE QWKCore QWKWidgets)
endif()

if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::AxContainer)
endif()

if(LINUX)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::DBus)
    set_target_properties(
        ${CMAKE_PROJECT_NAME} PROPERTIES BUILD_RPATH "\$ORIGIN" INSTALL_RPATH
                                                                "\$ORIGIN")
endif()

# emit marco in Qt conflicts with antlr4-cpp
target_compile_definitions(WingHexExplorer2 PRIVATE QT_NO_EMIT)
# for QString contraction optimization
target_compile_definitions(WingHexExplorer2 PRIVATE QT_USE_QSTRINGBUILDER)

target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
                                  "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_target_properties(
    WingHexExplorer2
    PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
               ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} MACOSX_BUNDLE
               TRUE WIN32_EXECUTABLE
               TRUE)

qt_finalize_executable(WingHexExplorer2)

add_dependencies(WingHexExplorer2 post_build_tasks build-time-make-directory)

install(
    TARGETS WingHexExplorer2
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

# Generate the deployment script.
qt6_generate_deploy_app_script(TARGET WingHexExplorer2 OUTPUT_SCRIPT
                               deploy_script NO_UNSUPPORTED_PLATFORM_ERROR)

# Call the deployment script on "cmake --install".
install(SCRIPT ${deploy_script})

add_subdirectory(WingPlugin)

target_compile_options(
    WingHexExplorer2
    PRIVATE
        $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-g1>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/Z7>
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:
        -ffunction-sections
        -fdata-sections
        -fvisibility=hidden
        -fvisibility-inlines-hidden
        -fno-var-tracking
        -fno-var-tracking-assignments
        >
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:
        -ffunction-sections
        -fdata-sections
        -fvisibility=hidden
        -fvisibility-inlines-hidden
        -fno-var-tracking
        >)

target_link_options(
    WingHexExplorer2
    PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/DEBUG
    /OPT:REF
    /OPT:ICF>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-Wl,--gc-sections>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-Wl,--gc-sections>)

enable_reproducible_paths(WingHexExplorer2)
