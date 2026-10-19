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
    src/dialog/common/wingfiledialog.cpp
    src/dialog/common/wingfiledialog.h
    src/dialog/common/winginputdialog.cpp
    src/dialog/common/winginputdialog.h
    src/dialog/common/wingmessagebox.cpp
    src/dialog/common/wingmessagebox.h
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
    src/control/common/compositeiconengine.cpp
    src/control/common/compositeiconengine.h
    src/control/common/eventfilter.h
    src/control/console/consolehighlighanim.cpp
    src/control/console/consolehighlighanim.h
    src/control/console/scriptingconsole.cpp
    src/control/console/scriptingconsole.h
    src/control/console/scriptingconsolebase.cpp
    src/control/console/scriptingconsolebase.h
    src/control/console/wingconsolehighligher.cpp
    src/control/console/wingconsolehighligher.h
    src/control/delegate/mutisavebuttondelegate.cpp
    src/control/delegate/mutisavebuttondelegate.h
    src/control/delegate/richtextitemdelegate.cpp
    src/control/delegate/richtextitemdelegate.h
    src/control/docking/dockcomponentsfactory.cpp
    src/control/docking/dockcomponentsfactory.h
    src/control/docking/dockwidgettab.cpp
    src/control/docking/dockwidgettab.h
    src/control/editor/codeedit.cpp
    src/control/editor/codeedit.h
    src/control/editor/codeeditcontrolwidget.cpp
    src/control/editor/codeeditcontrolwidget.h
    src/control/editor/codeinfotip.cpp
    src/control/editor/codeinfotip.h
    src/control/editor/editorinfo.h
    src/control/editor/editorlspevent.cpp
    src/control/editor/editorlspevent.h
    src/control/editor/editorviewcontext.cpp
    src/control/editor/editorviewcontext.h
    src/control/editor/lspeditorinterface.cpp
    src/control/editor/lspeditorinterface.h
    src/control/editor/luaucompletion.cpp
    src/control/editor/luaucompletion.h
    src/control/editor/luauindentation.cpp
    src/control/editor/luauindentation.h
    src/control/editor/scripteditor.cpp
    src/control/editor/scripteditor.h
    src/control/editor/scriptinfobroswer.cpp
    src/control/editor/scriptinfobroswer.h
    src/control/editor/searchreplacewidget.cpp
    src/control/editor/searchreplacewidget.h
    src/control/editor/snippetprocessor.cpp
    src/control/editor/snippetprocessor.h
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
    src/control/idbtreeview.cpp
    src/control/idbtreeview.h
    src/control/objtreewidget.cpp
    src/control/objtreewidget.h
    src/control/qcolorpicker_slider.cpp
    src/control/qcolorpicker_slider.hpp
    src/control/qtableviewext.cpp
    src/control/qtableviewext.h
    src/control/settingspopup.cpp
    src/control/settingspopup.h
    src/control/toast.cpp
    src/control/toast.h)

set(CLASS_SRC
    src/class/application/appmanager.cpp
    src/class/application/appmanager.h
    src/class/application/crashhandler.cpp
    src/class/application/crashhandler.h
    src/class/application/inspectqtloghelper.cpp
    src/class/application/inspectqtloghelper.h
    src/class/application/logger.cpp
    src/class/application/logger.h
    src/class/application/wingupdater.cpp
    src/class/application/wingupdater.h
    src/class/file/fileaccesscheck.cpp
    src/class/file/fileaccesscheck.h
    src/class/file/recentfilemanager.cpp
    src/class/file/recentfilemanager.h
    src/class/file/showinshell.cpp
    src/class/file/showinshell.h
    src/class/file/workspacemanager.cpp
    src/class/file/workspacemanager.h
    src/class/layout/layoutmanager.cpp
    src/class/layout/layoutmanager.h
    src/class/plugin/pluginsystem.cpp
    src/class/plugin/pluginsystem.h
    src/class/plugin/wingplugincert.cpp
    src/class/plugin/wingplugincert.h
    src/class/settings/languagemanager.cpp
    src/class/settings/languagemanager.h
    src/class/settings/settingmanager.cpp
    src/class/settings/settingmanager.h
    src/class/settings/skinmanager.cpp
    src/class/settings/skinmanager.h
    src/class/util/changedstringlist.cpp
    src/class/util/changedstringlist.h
    src/class/util/cryptographichash.cpp
    src/class/util/cryptographichash.h
    src/class/util/qkeysequences.cpp
    src/class/util/qkeysequences.h
    src/class/util/resettabletimer.cpp
    src/class/util/resettabletimer.h)

set(LUAU_LS
    src/luau/lsp/attachcommentsvisitor.cpp
    src/luau/lsp/attachcommentsvisitor.h
    src/luau/lsp/findnodetype.cpp
    src/luau/lsp/findnodetype.h
    src/luau/lsp/luaulanguageserver.cpp
    src/luau/lsp/luaulanguageserver.h
    src/luau/lsp/semantictokensvisitor.cpp
    src/luau/lsp/semantictokensvisitor.h
    src/luau/lsp/lsp.h
    src/luau/lsp/typehelpers.cpp
    src/luau/lsp/typehelpers.h)

set(LUAU_MOD_SRC
    ${LUAU_LS}
    src/luau/breakpoint.cpp
    src/luau/breakpoint.h
    src/luau/luaudebugger.cpp
    src/luau/luaudebugger.h
    src/luau/luaufile.cpp
    src/luau/luaufile.h
    src/luau/luauformatter.cpp
    src/luau/luauformatter.h
    src/luau/luauinspector.cpp
    src/luau/luauinspector.h
    src/luau/luauscope.cpp
    src/luau/luauscope.h
    src/luau/luautypes.cpp
    src/luau/luautypes.h
    src/luau/luauutil.cpp
    src/luau/luauutil.h
    src/luau/luauvariable.cpp
    src/luau/luauvariable.h
    src/luau/luauvariableregistry.cpp
    src/luau/luauvariableregistry.h
    src/luau/runtime/luauscheduler.cpp
    src/luau/runtime/luauscheduler.h
    src/luau/runtime/scriptmachine.cpp
    src/luau/runtime/scriptmachine.h
    src/luau/runtime/scriptmanager.cpp
    src/luau/runtime/scriptmanager.h
    src/luau/runtime/scriptsettings.cpp
    src/luau/runtime/scriptsettings.h
    src/luau/scriptbindingengine.cpp
    src/luau/scriptbindingengine.h
    src/luau/wingluaurequire.cpp
    src/luau/wingluaurequire.h)

if(WINGHEX_USE_FRAMELESS)
    set(WIDGET_FRAME_SRC
        src/widgetframe/windowbar.cpp src/widgetframe/windowbar.h
        src/widgetframe/windowbar_p.h src/widgetframe/windowbutton.cpp
        src/widgetframe/windowbutton.h src/widgetframe/windowbutton_p.h)
    list(APPEND WIDGET_FRAME_SRC src/widgetframe/framelesshelper.h
         src/widgetframe/framelesshelper.cpp)
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
    src/scriptaddon/luauhelper.h
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
    ${CLASS_SRC}
    ${CONTROL_SRC}
    ${LUAU_MOD_SRC}
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
            Luau.Require
            LuaBridge
            cffi
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
