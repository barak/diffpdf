cmake_minimum_required(VERSION 3.1)

set(DIFFPDF_VERSION 2.1.3.1)
project(DiffPDF VERSION ${DIFFPDF_VERSION})

set(QT_MIN_VERSION 5.7.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMSetupVersion)
include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    LinguistTools
    PrintSupport
    Widgets)

find_package(Poppler REQUIRED COMPONENTS Core Qt5)

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

ecm_setup_version(PROJECT
    VARIABLE_PREFIX DIFFPDF
    VERSION_HEADER diffpdfversion.h)

configure_file(diffpdf.1.cmake diffpdf.1)

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


file(GLOB diffpdf_DOCS *.html)
install(FILES ${diffpdf_DOCS} DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES README CHANGES DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(TARGETS diffpdf DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_BINARY_DIR}/diffpdf.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES eu.qtrac.diffpdf.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES images/icon.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps
        RENAME diffpdf.png)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
