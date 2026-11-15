cmake_minimum_required(VERSION 3.20)
project(QHexViewTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(
    Qt6 REQUIRED
    COMPONENTS Widgets
               Gui
               Concurrent
               Test
)

get_filename_component(WING_SOURCE_ROOT "../.." ABSOLUTE)
add_subdirectory("${WING_SOURCE_ROOT}/3rdparty/QHexView" QHexView)

add_executable(qhexview_tests tst_qhexview.cpp
    "${WING_SOURCE_ROOT}/src/model/numshowmodel.cpp"
    "${WING_SOURCE_ROOT}/src/control/delegate/binarytextitemdelegate.cpp")
target_include_directories(
    qhexview_tests PRIVATE "${WING_SOURCE_ROOT}/3rdparty/QHexView"
    "${WING_SOURCE_ROOT}/3rdparty" "${WING_SOURCE_ROOT}/src"
)
target_link_libraries(
    qhexview_tests
    PRIVATE QHexView
            Qt6::Widgets
            Qt6::Concurrent
            Qt6::Test
)

enable_testing()
add_test(NAME qhexview_tests COMMAND qhexview_tests)
set_tests_properties(
    qhexview_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
