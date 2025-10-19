cmake_minimum_required(VERSION 3.16)

project(TestStructGen LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

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

add_executable(TestStructGen main.cpp TestHeader.h)

target_link_libraries(TestStructGen Qt${QT_VERSION_MAJOR}::Core)
