QT += widgets

HEADERS += $$PWD/document/commands/hex/hexcommand.h \
           $$PWD/document/commands/hex/insertcommand.h \
           $$PWD/document/commands/hex/removecommand.h \
           $$PWD/document/commands/hex/replacecommand.h \
           $$PWD/document/buffer/qhexbuffer.h \
           $$PWD/document/buffer/qmemoryrefbuffer.h \
           $$PWD/document/buffer/qmemorybuffer.h \
           $$PWD/document/buffer/qfilebuffer.h \
           $$PWD/document/qhexcursor.h \
           $$PWD/document/qhexdocument.h \
           $$PWD/document/qhexmetadata.h \
           $$PWD/document/qhexrenderer.h \
           $$PWD/qhexview.h \
    $$PWD/document/commands/meta/metacommand.h \
    $$PWD/document/commands/meta/metaaddcommand.h \
    $$PWD/document/commands/meta/metaremovecommand.h \
    $$PWD/document/commands/meta/metareplacecommand.h \
    $$PWD/document/commands/meta/metaclearcommand.h \
    $$PWD/document/commands/meta/metaremoveposcommand.h \
    $$PWD/document/commands/bookmark/bookmarkcommand.h \
    $$PWD/document/commands/bookmark/bookmarkaddcommand.h \
    $$PWD/document/commands/bookmark/bookmarkremovecommand.h \
    $$PWD/document/commands/bookmark/bookmarkreplacecommand.h \
    $$PWD/document/commands/bookmark/bookmarkclearcommand.h \
    $$PWD/QHexEdit2/chunks.h \
    $$PWD/document/commands/meta/metashowcommand.h \
    $$PWD/document/commands/baseaddrcommand.h \
    $$PWD/document/commands/encodingchangecommand.h \
    $$PWD/document/buffer/qfileregionbuffer.h

SOURCES += $$PWD/document/commands/hex/hexcommand.cpp \
           $$PWD/document/commands/hex/insertcommand.cpp \
           $$PWD/document/commands/hex/removecommand.cpp \
           $$PWD/document/commands/hex/replacecommand.cpp \
           $$PWD/document/buffer/qhexbuffer.cpp \
           $$PWD/document/buffer/qmemoryrefbuffer.cpp \
           $$PWD/document/buffer/qmemorybuffer.cpp \
           $$PWD/document/buffer/qfilebuffer.cpp \
           $$PWD/document/qhexcursor.cpp \
           $$PWD/document/qhexdocument.cpp \
           $$PWD/document/qhexmetadata.cpp \
           $$PWD/document/qhexrenderer.cpp \
           $$PWD/qhexview.cpp \
    $$PWD/document/commands/meta/metacommand.cpp \
    $$PWD/document/commands/meta/metaaddcommand.cpp \
    $$PWD/document/commands/meta/metaremovecommand.cpp \
    $$PWD/document/commands/meta/metareplacecommand.cpp \
    $$PWD/document/commands/meta/metaclearcommand.cpp \
    $$PWD/document/commands/meta/metaremoveposcommand.cpp \
    $$PWD/document/commands/bookmark/bookmarkcommand.cpp \
    $$PWD/document/commands/bookmark/bookmarkaddcommand.cpp \
    $$PWD/document/commands/bookmark/bookmarkremovecommand.cpp \
    $$PWD/document/commands/bookmark/bookmarkreplacecommand.cpp \
    $$PWD/document/commands/bookmark/bookmarkclearcommand.cpp \
    $$PWD/QHexEdit2/chunks.cpp \
    $$PWD/document/commands/meta/metashowcommand.cpp \
    $$PWD/document/commands/baseaddrcommand.cpp \
    $$PWD/document/commands/encodingchangecommand.cpp \
    $$PWD/document/buffer/qfileregionbuffer.cpp

INCLUDEPATH += $$PWD
