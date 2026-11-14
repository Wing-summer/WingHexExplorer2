# Copyright (C) 2026 WingSummer SPDX-License-Identifier: AGPL-3.0-only
cmake_minimum_required(VERSION 3.20)
project(HeadlessServiceTests LANGUAGES NONE)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(WINGHEX_SERVICE_BINARY "" CACHE FILEPATH "Isolated-settings headless process probe")
enable_testing()
if(WINGHEX_SERVICE_BINARY)
    add_test(NAME script_service COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_service.py" --binary "${WINGHEX_SERVICE_BINARY}")
endif()
