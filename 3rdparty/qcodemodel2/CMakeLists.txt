cmake_minimum_required(VERSION 3.6)
project(QCodeModel2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui)

add_definitions(-D_QCODE_MODEL_BUILD_STATIC_)

add_library(
    QCodeModel2 STATIC
    qcm-config.h
    qcodemodel.h
    qcodeproxymodel.h
    qcodenode.h
    qcodenodepool.h
    qsourcecodewatcher.h
    qcodeview.h
    qcodeserializer.h
    qcodestream.h
    qcodedevice.h
    qcodebuffer.h
    qcodelexer.h
    qcodeparser.h
    qcodeloader.h
    qcodemodel.cpp
    qcodeproxymodel.cpp
    qcodenode.cpp
    qcodenodepool.cpp
    qsourcecodewatcher.cpp
    qcodeview.cpp
    qcodeserializer.cpp
    qcodestream.cpp
    qcodedevice.cpp
    qcodebuffer.cpp
    qcodelexer.cpp
    qcodeparser.cpp
    qcodeloader.cpp)

target_include_directories(QCodeModel2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(QCodeModel2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                          Qt${QT_VERSION_MAJOR}::Gui)

target_compile_definitions(QCodeModel2 PUBLIC -D_QCODE_MODEL_BUILD_STATIC_)
