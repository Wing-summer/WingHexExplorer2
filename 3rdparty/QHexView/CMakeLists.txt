cmake_minimum_required(VERSION 3.12)

project(QHexView LANGUAGES CXX)

find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Widgets Gui
    REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

add_library(
    QHexView STATIC
    document/buffer/qfilebuffer.cpp
    document/buffer/qfilebuffer.h
    document/buffer/qfileregionbuffer.cpp
    document/buffer/qfileregionbuffer.h
    document/buffer/qhexbuffer.cpp
    document/buffer/qhexbuffer.h
    document/buffer/qmemorybuffer.cpp
    document/buffer/qmemorybuffer.h
    document/buffer/qmemoryrefbuffer.cpp
    document/buffer/qmemoryrefbuffer.h
    document/commands/hex/hexcommand.cpp
    document/commands/hex/hexcommand.h
    document/commands/hex/insertcommand.cpp
    document/commands/hex/insertcommand.h
    document/commands/hex/removecommand.cpp
    document/commands/hex/removecommand.h
    document/commands/hex/replacecommand.cpp
    document/commands/hex/replacecommand.h
    document/commands/meta/metashowcommand.h
    document/commands/meta/metaremovecommand.cpp
    document/commands/meta/metaremovecommand.h
    document/commands/meta/metaremoveposcommand.cpp
    document/commands/meta/metaremoveposcommand.h
    document/commands/meta/metareplacecommand.cpp
    document/commands/meta/metareplacecommand.h
    document/commands/meta/metashowcommand.cpp
    document/commands/meta/metaclearcommand.cpp
    document/commands/meta/metaclearcommand.h
    document/commands/meta/metaaddcommand.h
    document/commands/meta/metaaddcommand.cpp
    document/commands/meta/metacommand.cpp
    document/commands/meta/metacommand.h
    document/commands/bookmark/bookmarkaddcommand.cpp
    document/commands/bookmark/bookmarkaddcommand.h
    document/commands/bookmark/bookmarkclearcommand.cpp
    document/commands/bookmark/bookmarkclearcommand.h
    document/commands/bookmark/bookmarkcommand.cpp
    document/commands/bookmark/bookmarkcommand.h
    document/commands/bookmark/bookmarkremovecommand.cpp
    document/commands/bookmark/bookmarkremovecommand.h
    document/commands/bookmark/bookmarkreplacecommand.cpp
    document/commands/bookmark/bookmarkreplacecommand.h
    document/commands/baseaddrcommand.cpp
    document/commands/baseaddrcommand.h
    document/commands/encodingchangecommand.cpp
    document/commands/encodingchangecommand.h
    document/buffer/qwindriverbuffer.h
    document/buffer/qwindriverbuffer.cpp
    document/qstoragedevice.h
    document/qstoragedevice.cpp
    document/qhexcursor.cpp
    document/qhexcursor.h
    document/qhexdocument.cpp
    document/qhexdocument.h
    document/qhexmetadata.cpp
    document/qhexmetadata.h
    document/qhexrenderer.cpp
    document/qhexrenderer.h
    QHexEdit2/chunks.cpp
    QHexEdit2/chunks.h
    QHexEdit2/chunks_win.cpp
    QHexEdit2/chunks_win.h
    qhexview.h
    qhexview.cpp)

set_target_properties(
    QHexView
    PROPERTIES AUTOMOC ON
               CXX_STANDARD 11
               CXX_STANDARD_REQUIRED ON)

target_link_libraries(QHexView PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                       Qt${QT_VERSION_MAJOR}::Gui)
