# Copyright (C) 2026 WingSummer SPDX-License-Identifier: AGPL-3.0-only
add_wing_plugin(
    NAME
    HeadlessProbe
    TYPE
    PLUGIN
    SOURCES
    probe.cpp
    HeadlessProbe.json
    TEST_MODE
    TRUE
)
add_wing_plugin(
    NAME
    HeadlessDeviceProbe
    TYPE
    DEVDRV
    SOURCES
    probe.cpp
    HeadlessDeviceProbe.json
    TEST_MODE
    TRUE
)
target_compile_definitions(HeadlessDeviceProbe PRIVATE HEADLESS_DEVICE_PROBE)
