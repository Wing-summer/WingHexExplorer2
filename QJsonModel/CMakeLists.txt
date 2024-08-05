cmake_minimum_required(VERSION 3.26)

project(
  QJsonModel
  VERSION 0.0.2
  LANGUAGES C CXX
  # Save this for later: HOMEPAGE_URL <URL>
  DESCRIPTION
    "QJsonModel is a json tree model class for Qt6/C++17 based on QAbstractItemModel. MIT License."
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(MSVC)
  string(APPEND CMAKE_CXX_FLAGS " /utf-8")
  string(APPEND CMAKE_C_FLAGS " /utf-8")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(QJsonModel STATIC include/details/QUtf8.hpp include/QJsonModel.hpp
                              QJsonModel.cpp)

target_link_libraries(QJsonModel PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
