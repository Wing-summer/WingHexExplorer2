cmake_minimum_required(VERSION 3.12.0)

project(QtSingleApplication LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network)

add_library(
    ${PROJECT_NAME} STATIC
    src/qtlocalpeer.h
    src/qtsingleapplication.h
    src/qtsinglecoreapplication.h
    src/qtlocalpeer.cpp
    src/qtsingleapplication.cpp
    src/qtsinglecoreapplication.cpp
    src/qtlockedfile.h
    src/qtlockedfile.cpp)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
