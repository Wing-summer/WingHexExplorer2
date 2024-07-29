cmake_minimum_required(VERSION 3.5)

project(
  QScintilla
  VERSION 2.14.1
  LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport
                                                  LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport
                                                       LinguistTools)

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /utf-8")
  string(APPEND CMAKE_C_FLAGS " /utf-8")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

file(GLOB_RECURSE SCINTILLA_H "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/*.h")
file(GLOB_RECURSE SCINTILLA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/*.cpp")
file(GLOB_RECURSE QSCINTILLA_H "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE QSCINTILLA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

add_library(QScintilla STATIC ${SCINTILLA_H} ${SCINTILLA_SRC} ${QSCINTILLA_H}
                              ${QSCINTILLA_SRC})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(QScintilla PRIVATE -w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(QScintilla PRIVATE -w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  string(REGEX REPLACE "(^| )([/-])W[0-9]( |$)" " " CMAKE_CXX_FLAGS
                       "${CMAKE_CXX_FLAGS}")
  target_compile_options(QScintilla PRIVATE /w)
endif()

target_include_directories(
  QScintilla
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
         "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
         "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/lexlib"
         "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src")

target_link_libraries(QScintilla PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                         Qt${QT_VERSION_MAJOR}::PrintSupport)
