cmake_minimum_required(VERSION 3.6)
project(QCodeEditor2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui PrintSupport Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui PrintSupport
                                                       Xml)

add_definitions(-DQNFA_BUILD)

set(DOCUMENT_SRC
    lib/document/qdocumentline_p.h
    lib/document/qdocumentbuffer.h
    lib/document/qdocumentcommand.h
    lib/document/qdocumentline.h
    lib/document/qdocumentcursor.h
    lib/document/qdocumentcursor_p.h
    lib/document/qdocument_p.h
    lib/document/qdocument.h
    lib/document/qdocumentbuffer.cpp
    lib/document/qdocumentline.cpp
    lib/document/qdocumentcursor.cpp
    lib/document/qdocumentcommand.cpp
    lib/document/qdocument.cpp)

set(WIDGETS_SRC
    lib/widgets/qcalltip.cpp
    lib/widgets/qcalltip.h
    lib/widgets/qfoldpanel.cpp
    lib/widgets/qfoldpanel.h
    lib/widgets/qlinechangepanel.cpp
    lib/widgets/qlinechangepanel.h
    lib/widgets/qlinemarkpanel.cpp
    lib/widgets/qlinemarkpanel.h
    lib/widgets/qlinenumberpanel.cpp
    lib/widgets/qlinenumberpanel.h
    lib/widgets/qpanel.cpp
    lib/widgets/qpanel.h
    lib/widgets/qsimplecolorpicker.cpp
    lib/widgets/qsimplecolorpicker.h)

set(QNFA_SRC
    lib/qnfa/light_vector.h lib/qnfa/qnfadefinition.h lib/qnfa/qnfa.h
    lib/qnfa/xml2qnfa.cpp lib/qnfa/qnfa.cpp lib/qnfa/qnfadefinition.cpp)

set(SNIPPET_SRC
    lib/snippets/qsnippet.cpp
    lib/snippets/qsnippet.h
    lib/snippets/qsnippet_p.h
    lib/snippets/qsnippetbinding.cpp
    lib/snippets/qsnippetbinding.h
    lib/snippets/qsnippetmanager.cpp
    lib/snippets/qsnippetmanager.h
    lib/snippets/qsnippetpatternloader.h)

set(SOURCE_FILES
    lib/qce-config.h
    lib/qcodecompletionengine.h
    lib/qcodeedit.h
    lib/qeditor.h
    lib/qeditorinputbinding.h
    lib/qeditorinputbindinginterface.h
    lib/qeditsession.h
    lib/qformat.h
    lib/qformatfactory.h
    lib/qformatscheme.h
    lib/qlanguagedefinition.h
    lib/qlanguagefactory.h
    lib/qlinemarksinfocenter.h
    lib/qpanellayout.h
    lib/qreliablefilewatch.h
    lib/qcodecompletionengine.cpp
    lib/qcodeedit.cpp
    lib/qeditor.cpp
    lib/qeditorinputbinding.cpp
    lib/qeditsession.cpp
    lib/qformatscheme.cpp
    lib/qlanguagedefinition.cpp
    lib/qlanguagefactory.cpp
    lib/qlinemarksinfocenter.cpp
    lib/qpanellayout.cpp
    lib/qreliablefilewatch.cpp)

add_library(QCodeEditor2 STATIC ${SOURCE_FILES} ${DOCUMENT_SRC} ${WIDGETS_SRC}
                                ${QNFA_SRC} ${SNIPPET_SRC})

target_compile_definitions(QCodeEditor2 PUBLIC _QCODE_EDIT_BUILD_
                                               _QCODE_EDIT_EMBED_)

target_include_directories(
    QCodeEditor2
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib"
           "${CMAKE_CURRENT_SOURCE_DIR}/lib/document"
           "${CMAKE_CURRENT_SOURCE_DIR}/lib/widgets"
           "${CMAKE_CURRENT_SOURCE_DIR}/lib/qnfa"
           "${CMAKE_CURRENT_SOURCE_DIR}/lib/snippets")

target_link_libraries(
    QCodeEditor2
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui
            Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::PrintSupport)
