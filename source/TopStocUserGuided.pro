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

DEFINES += TETLIBRARY # notwendig damit tetgen nicht eigene mein baut

#ICON = icons.icns # kann benutzt werden wenn mit qmake, ansonsten händisch

SOURCES += main.cpp\
        mainwidget.cpp \
    viewer.cpp \
    controlpanel.cpp \
    topstoc.cpp \
    xalloc.c \
    reporting.c \
    geomutils.c \
    compute_error.c \
    boundingbox.cpp \
    topological_loops.cpp \
    triangulate.cpp \
    tetgen.cxx \
    predicates.cxx

HEADERS  += mainwidget.h \
    viewer.h \
    controlpanel.h \
    topstoc.h \
    boundingbox.h \
    3dmodel.h \
    xalloc.h \
    reporting.h \
    geomutils.h \
    compute_error.h \
    model_analysis.h \
    mymesh.h \
    topological_loops.h \
    triangulate.h \
    tetgen.h


FORMS += \
    controlpanel.ui

# INCLUDEPATH *= Developer/Applications/libQGLViewer-2.3.9
LIBS += -framework QGLViewer\
				-L/usr/local/lib/OpenMesh -lOpenMeshCore
# unterschied zwischen *= und +=


mac: LIBS += -F$$PWD/../../../../../../System/Library/Frameworks/ -framework GLUT

INCLUDEPATH += $$PWD/../../../../../../System/Library/Frameworks
DEPENDPATH += $$PWD/../../../../../../System/Library/Frameworks
