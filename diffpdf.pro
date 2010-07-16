HEADERS	    += mainwindow.hpp
SOURCES     += mainwindow.cpp
HEADERS	    += optionsform.hpp
SOURCES     += optionsform.cpp
HEADERS	    += generic.hpp
SOURCES	    += generic.cpp
HEADERS	    += sequence_matcher.hpp
SOURCES     += sequence_matcher.cpp
SOURCES     += main.cpp
RESOURCES   += resources.qrc
LIBS	    += -lpoppler-qt4
exists($(HOME)/opt/poppler014/) {
    message(Using locally built Poppler library)
    INCLUDEPATH += $(HOME)/opt/poppler014/include/poppler/qt4
    LIBS += -Wl,-rpath -Wl,$(HOME)/opt/poppler014/lib -Wl,-L$(HOME)/opt/poppler014/lib
} else {
    INCLUDEPATH += /usr/include/poppler/qt4
}
