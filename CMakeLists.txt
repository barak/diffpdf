cmake_minimum_required(VERSION 3.1.0)

project(diffpdf)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Core
    Gui
    LinguistTools
    PrintSupport
    Widgets)

find_package(Poppler COMPONENTS Qt5)

set(diffpdf_SRCS
    aboutform.cpp
    generic.cpp
    helpform.cpp
    label.cpp
    lineedit.cpp
    main.cpp
    mainwindow.cpp
    optionsform.cpp
    saveform.cpp
    sequence_matcher.cpp
    textitem.cpp)

file(GLOB diffpdf_TS *.ts)
qt5_add_translation(diffpdf_QM ${diffpdf_TS})

configure_file(resources.qrc.cmake resources.qrc)
qt5_add_resources(diffpdf_SRCS ${CMAKE_BINARY_DIR}/resources.qrc)

add_executable(diffpdf ${diffpdf_SRCS} ${diffpdf_QM})

target_link_libraries(diffpdf
    Poppler::Qt5
    Qt5::PrintSupport
    Qt5::Widgets)

target_compile_definitions(diffpdf PRIVATE -DPOPPLER_VERSION="${Poppler_VERSION}")
