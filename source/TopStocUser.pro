#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T18:23:46
#
#-------------------------------------------------

QT       += core gui xml opengl

CONFIG += x86_64\
					qtestlib # wäre besser ohne, aber wie?

TARGET = TopStoc
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    viewer.cpp \
    controlpanel.cpp \
    topstoc.cpp \
    boundingbox.cpp

HEADERS  += mainwidget.h \
    viewer.h \
    controlpanel.h \
    topstoc.h \
    debug.h \
    boundingbox.h


FORMS += \
    controlpanel.ui

# INCLUDEPATH *= Developer/Applications/libQGLViewer-2.3.9
LIBS += -framework QGLViewer\
				-L/usr/local/lib/OpenMesh -lOpenMeshCore
# unterschied zwischen *= und +=


mac: LIBS += -F$$PWD/../../../../../../System/Library/Frameworks/ -framework GLUT

INCLUDEPATH += $$PWD/../../../../../../System/Library/Frameworks
DEPENDPATH += $$PWD/../../../../../../System/Library/Frameworks
