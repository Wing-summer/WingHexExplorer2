cmake_minimum_required(VERSION 3.6)
project(QCodeEditor2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui PrintSupport Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui PrintSupport
                                                       Xml)

add_definitions(-DQNFA_BUILD -D_QCODE_EDIT_BUILD_ -D_QCODE_EDIT_EMBED_)

set(DOCUMENT_SRC
    lib/document/qdocumentsearch.h
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
    lib/document/qdocumentsearch.cpp
    lib/document/qdocumentcursor.cpp
    lib/document/qdocumentcommand.cpp
    lib/document/qdocument.cpp)

set(WIDGETS_SRC
    lib/widgets/qgotolinedialog.h
    lib/widgets/qlinechangepanel.h
    lib/widgets/qfoldpanel.h
    lib/widgets/qlinenumberpanel.h
    lib/widgets/qstatuspanel.h
    lib/widgets/qcalltip.h
    lib/widgets/qsimplecolorpicker.h
    lib/widgets/qlinemarkpanel.h
    lib/widgets/qgotolinepanel.h
    lib/widgets/qgotolinedialog.cpp
    lib/widgets/qformatconfig.h
    lib/widgets/gotoline.ui
    lib/widgets/qeditconfig.h
    lib/widgets/qsimplecolorpicker.cpp
    lib/widgets/qsearchreplacepanel.h
    lib/widgets/qlinechangepanel.cpp
    lib/widgets/formatconfig.ui
    lib/widgets/qpanel.h
    lib/widgets/qgotolinepanel.cpp
    lib/widgets/gotolinedialog.ui
    lib/widgets/qstatuspanel.cpp
    lib/widgets/qlinemarkpanel.cpp
    lib/widgets/qlinenumberpanel.cpp
    lib/widgets/qcalltip.cpp
    lib/widgets/qpanel.cpp
    lib/widgets/qfoldpanel.cpp
    lib/widgets/editconfig.ui
    lib/widgets/qsearchreplacepanel.cpp
    lib/widgets/qformatconfig.cpp
    lib/widgets/qeditconfig.cpp
    lib/widgets/searchreplace.ui)

set(QNFA_SRC
    lib/qnfa/light_vector.h lib/qnfa/qnfadefinition.h lib/qnfa/qnfa.h
    lib/qnfa/xml2qnfa.cpp lib/qnfa/qnfa.cpp lib/qnfa/qnfadefinition.cpp)

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
                                ${QNFA_SRC})

target_include_directories(
  QCodeEditor2
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib"
         "${CMAKE_CURRENT_SOURCE_DIR}/lib/document"
         "${CMAKE_CURRENT_SOURCE_DIR}/lib/widgets"
         "${CMAKE_CURRENT_SOURCE_DIR}/lib/qnfa")
target_link_libraries(
  QCodeEditor2
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui
          Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::PrintSupport)
