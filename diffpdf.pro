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
exists($(HOME)/opt/poppler016/) {
    message(Using locally built Poppler library)
    INCLUDEPATH += $(HOME)/opt/poppler016/include/poppler/qt4
    LIBS += -Wl,-rpath -Wl,$(HOME)/opt/poppler016/lib -Wl,-L$(HOME)/opt/poppler016/lib
} else {
    exists(/usr/include/poppler/qt4) {
	INCLUDEPATH += /usr/include/poppler/qt4
    } else {
	INCLUDEPATH += /usr/local/include/poppler/qt4
    }
}
