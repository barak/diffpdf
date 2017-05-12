cmake_minimum_required(VERSION 3.1.0)

project(diffpdf)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})


find_package(Qt5Widgets REQUIRED)
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

add_executable(diffpdf ${diffpdf_SRCS})

target_link_libraries(diffpdf
    Poppler::Qt5
    Qt5::Widgets)

