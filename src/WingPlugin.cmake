# Function: add_wing_plugin
# Parameters:
#> NAME <string> - Plugin name (required)
#> TYPE <PLUGIN/DEVDRV> - Plugin Type (required)
#> SOURCES <list> - Source file list (required)
#> RESOURCES <list> - .qrc resource file list (optional, default empty)
#> TS_FILES <list> - Translation .ts file list (optional, default empty)
#> CXX_STANDARD <string> - C++ standard, default 17
#> TEST_MODE <bool> - Enable test mode (copy plugin to target directory), default
#  OFF
#> WINGHEX_PATH <path> - WingHex main program directory for test mode, default
#  ${CMAKE_BINARY_DIR}
#> LINK_LIBRARIES <list> - Additional libraries to link (optional, defaults to
#  Qt::Widgets and WingPlugin)
#> TRANSLATION_UTILS_ALREADY_INCLUDED <bool> - Whether TranslationUtils.cmake has
#  already been included, default OFF
#> TRANSLATION_UTILS_PATH <path> - Path to TranslationUtils.cmake, default
#  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TranslationUtils.cmake
#> DEP_SUBDIR <string> - Subdirectory for dependency libraries (relative to
#  plugin location), default ${NAME}Dep. E.g., if plugin name is Foo,
#  subdirectory is FooDep
#> SET_RPATH <bool> - Whether to set RPATH for relative dependency lookup,
#  default ON (effective only on Linux/Unix)
#> KEEP_DEBUG_SYMBOLS <bool> - Keep debug symbols in Release builds, default ON
#  TRANSLATION_OPTIONS <list> - Options passed to lrelease (e.g., -no-obsolete,
#  -compress), default empty

function(_winghex_to_bool input_var default_value output_var)
    if(DEFINED ${input_var})
        set(_value "${${input_var}}")
        string(TOUPPER "${_value}" _value_upper)

        if(_value_upper STREQUAL "OFF"
           OR _value_upper STREQUAL "FALSE"
           OR _value_upper STREQUAL "NO"
           OR _value_upper STREQUAL "N"
           OR _value STREQUAL "0")
            set(${output_var}
                OFF
                PARENT_SCOPE)
        else()
            set(${output_var}
                ON
                PARENT_SCOPE)
        endif()
    else()
        set(${output_var}
            ${default_value}
            PARENT_SCOPE)
    endif()
endfunction()

function(_winghex_normalize_paths output_var)
    set(_result "")
    foreach(_path IN LISTS ARGN)
        if(_path STREQUAL "")
            continue()
        endif()

        if(IS_ABSOLUTE "${_path}")
            list(APPEND _result "${_path}")
        else()
            list(APPEND _result "${CMAKE_CURRENT_SOURCE_DIR}/${_path}")
        endif()
    endforeach()

    set(${output_var}
        "${_result}"
        PARENT_SCOPE)
endfunction()

function(add_wing_plugin)
    cmake_parse_arguments(
        ARG
        ""
        "NAME;TYPE;CXX_STANDARD;WINGHEX_PATH;TEST_MODE;TRANSLATION_UTILS_ALREADY_INCLUDED;TRANSLATION_UTILS_PATH;DEP_SUBDIR;SET_RPATH;KEEP_DEBUG_SYMBOLS"
        "SOURCES;RESOURCES;TS_FILES;LINK_LIBRARIES;TRANSLATION_OPTIONS"
        ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "add_wing_plugin: NAME is required")
    endif()

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "add_wing_plugin: SOURCES is required")
    endif()

    if(NOT ARG_TYPE)
        set(ARG_TYPE PLUGIN)
    endif()
    string(TOUPPER "${ARG_TYPE}" ARG_TYPE)
    if(ARG_TYPE STREQUAL "PLUGIN")
        set(_wing_plugin_suffix ".wingplg")
        set(_wing_is_plugin TRUE)
    elseif(ARG_TYPE STREQUAL "DEVDRV")
        set(_wing_plugin_suffix ".wingdrv")
        set(_wing_is_plugin FALSE)
    else()
        message(
            FATAL_ERROR
                "add_wing_plugin: TYPE must be PLUGIN or DEVDRV, got '${ARG_TYPE}'"
        )
    endif()

    _winghex_to_bool(ARG_TEST_MODE OFF ARG_TEST_MODE)
    _winghex_to_bool(ARG_TRANSLATION_UTILS_ALREADY_INCLUDED OFF
                     ARG_TRANSLATION_UTILS_ALREADY_INCLUDED)
    _winghex_to_bool(ARG_SET_RPATH ON ARG_SET_RPATH)
    _winghex_to_bool(ARG_KEEP_DEBUG_SYMBOLS ON ARG_KEEP_DEBUG_SYMBOLS)

    if(NOT ARG_CXX_STANDARD)
        set(ARG_CXX_STANDARD 17)
    endif()

    if(NOT ARG_TRANSLATION_UTILS_PATH)
        set(ARG_TRANSLATION_UTILS_PATH
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TranslationUtils.cmake")
    endif()

    if(NOT ARG_DEP_SUBDIR)
        set(ARG_DEP_SUBDIR "${ARG_NAME}Dep")
    endif()

    # --- Qt6 ---
    set(QT_COMPONENTS Widgets)
    if(ARG_TS_FILES)
        list(APPEND QT_COMPONENTS LinguistTools)
    endif()
    find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

    # --- Translation handling ---
    set(QM_FILES "")
    set(QM_RES "")

    if(ARG_TS_FILES)
        # Normalize and validate TS files
        _winghex_normalize_paths(_valid_ts_files ${ARG_TS_FILES})

        set(_filtered_ts_files "")
        foreach(_ts IN LISTS _valid_ts_files)
            if(NOT _ts STREQUAL "")
                if(EXISTS "${_ts}")
                    list(APPEND _filtered_ts_files "${_ts}")
                else()
                    message(
                        WARNING
                            "add_wing_plugin: TS file '${_ts}' does not exist, ignoring."
                    )
                endif()
            endif()
        endforeach()

        if(NOT _filtered_ts_files)
            message(FATAL_ERROR "add_wing_plugin: No valid .ts files provided.")
        endif()

        # Load TranslationUtils only when needed
        if(NOT COMMAND add_plugin_translations_resource)
            if(ARG_TRANSLATION_UTILS_ALREADY_INCLUDED)
                message(
                    FATAL_ERROR
                        "add_wing_plugin: TRANSLATION_UTILS_ALREADY_INCLUDED=ON, but command 'add_plugin_translations_resource' is still not available."
                )
            endif()

            if(EXISTS "${ARG_TRANSLATION_UTILS_PATH}")
                include("${ARG_TRANSLATION_UTILS_PATH}")
            else()
                message(
                    FATAL_ERROR
                        "add_wing_plugin: TranslationUtils.cmake not found at '${ARG_TRANSLATION_UTILS_PATH}'."
                )
            endif()
        endif()

        if(NOT COMMAND add_plugin_translations_resource)
            message(
                FATAL_ERROR
                    "add_wing_plugin: command 'add_plugin_translations_resource' is not defined after including TranslationUtils.cmake."
            )
        endif()

        # Build source list for lupdate
        set(_lupdate_sources "")
        foreach(_src IN LISTS ARG_SOURCES)
            if(_src STREQUAL "")
                continue()
            endif()

            if(_src MATCHES "\\.(cpp|cxx|cc|h|hpp|hh|ui|qml|js)$")
                if(IS_ABSOLUTE "${_src}")
                    list(APPEND _lupdate_sources "${_src}")
                else()
                    list(APPEND _lupdate_sources
                         "${CMAKE_CURRENT_SOURCE_DIR}/${_src}")
                endif()
            endif()
        endforeach()

        list(REMOVE_DUPLICATES _lupdate_sources)

        if(NOT _lupdate_sources)
            message(
                WARNING
                    "add_wing_plugin: No lupdate source files detected from SOURCES. Translation generation may be empty."
            )
        endif()

        # Generate QM files
        if(ARG_TRANSLATION_OPTIONS)
            qt6_create_translation(
                _qm_from_this ${_filtered_ts_files} ${_lupdate_sources} OPTIONS
                ${ARG_TRANSLATION_OPTIONS})
        else()
            qt6_create_translation(_qm_from_this ${_filtered_ts_files}
                                   ${_lupdate_sources})
        endif()

        set(QM_FILES ${_qm_from_this})

        add_plugin_translations_resource(QM_RES ${ARG_NAME} ${QM_FILES})
    endif()

    # --- Library ---
    add_library(${ARG_NAME} SHARED ${ARG_SOURCES} ${ARG_RESOURCES} ${QM_FILES}
                                   ${QM_RES})

    set_target_properties(
        ${ARG_NAME}
        PROPERTIES SUFFIX "${_wing_plugin_suffix}"
                   CXX_STANDARD ${ARG_CXX_STANDARD}
                   CXX_STANDARD_REQUIRED ON
                   CXX_EXTENSIONS OFF
                   AUTOMOC ON
                   AUTOUIC ON
                   AUTORCC ON)

    target_include_directories(
        ${ARG_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
                            "${CMAKE_CURRENT_BINARY_DIR}")

    # --- RPATH ---
    if(ARG_SET_RPATH
       AND (CMAKE_SYSTEM_NAME STREQUAL "Linux"
            OR CMAKE_SYSTEM_NAME STREQUAL "Darwin"
            OR CMAKE_SYSTEM_NAME MATCHES "BSD"))
        set_target_properties(
            ${ARG_NAME} PROPERTIES BUILD_RPATH "\$ORIGIN/${ARG_DEP_SUBDIR}"
                                   INSTALL_RPATH "\$ORIGIN/${ARG_DEP_SUBDIR}")
    endif()

    # --- Debug symbols in Release ---
    if(ARG_KEEP_DEBUG_SYMBOLS)
        target_compile_options(
            ${ARG_NAME}
            PRIVATE
                "$<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-g1>"
                "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/Z7>"
                "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-ffunction-sections;-fdata-sections;-fvisibility=hidden;-fvisibility-inlines-hidden;-fno-var-tracking;-fno-var-tracking-assignments>"
                "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-ffunction-sections;-fdata-sections;-fvisibility=hidden;-fvisibility-inlines-hidden;-fno-var-tracking>"
        )

        target_link_options(
            ${ARG_NAME}
            PRIVATE
            "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/DEBUG;/OPT:REF;/OPT:ICF>"
            "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-Wl,--gc-sections>"
            "$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-Wl,--gc-sections>"
        )
    endif()

    # --- Link libraries ---
    target_link_libraries(${ARG_NAME} PRIVATE Qt6::Widgets WingPlugin
                                              ${ARG_LINK_LIBRARIES})

    # --- Test mode copy ---
    if(ARG_TEST_MODE)
        if(NOT ARG_WINGHEX_PATH)
            set(ARG_WINGHEX_PATH "${CMAKE_BINARY_DIR}")
        endif()

        if(${_wing_is_plugin})
            set(WINGHEX_DRV_PATH "${ARG_WINGHEX_PATH}/plugin")
        else()
            set(WINGHEX_DRV_PATH "${ARG_WINGHEX_PATH}/devdrv")
        endif()
        add_custom_command(
            TARGET ${ARG_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${WINGHEX_DRV_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:${ARG_NAME}> "${WINGHEX_DRV_PATH}"
            COMMENT "Copying plugin ${ARG_NAME} to ${WINGHEX_DRV_PATH}")
    endif()
endfunction()
