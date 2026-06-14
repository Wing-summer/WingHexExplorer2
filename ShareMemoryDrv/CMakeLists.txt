cmake_minimum_required(VERSION 3.16)

project(ShareMemoryDrv)

option(WINGHEX_TEST_MODE TRUE)

add_wing_plugin(
    NAME
    ShareMemoryDrv

    TYPE
    DEVDRV

    SOURCES
    sharememorydrv.h
    sharememorydrv.cpp
    ShareMemoryDrv.json
    sharedmemory.h
    sharedmemory.cpp

    RESOURCES
    resources.qrc

    TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/lang/shmem_zh_CN.ts"

    TEST_MODE
    ${WINGHEX_TEST_MODE}

    TRANSLATION_UTILS_ALREADY_INCLUDED
    TRUE

    TRANSLATION_OPTIONS
    -no-obsolete
)
