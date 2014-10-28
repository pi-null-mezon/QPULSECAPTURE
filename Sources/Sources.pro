#-------------------------------------------------
#
# Project created by QtCreator 2014-02-08T18:00:16
#
#-------------------------------------------------

QT +=   core \
        gui \
        multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    QPULSECAPTURE
TEMPLATE =  app
VERSION =   1.0.0.3

SOURCES +=  main.cpp\
            mainwindow.cpp \
            qimagewidget.cpp \
            qopencvprocessor.cpp \
            qvideocapture.cpp \
            qharmonicprocessor.cpp \
            qsettingsdialog.cpp \
            qeasyplot.cpp \
            qbackgroundwidget.cpp

HEADERS  += mainwindow.h \
            qimagewidget.h \
            qopencvprocessor.h \
            qvideocapture.h \
            about.h \
            qharmonicprocessor.h \
            qsettingsdialog.h \
            qeasyplot.h \
            qbackgroundwidget.h

FORMS += qsettingsdialog.ui

RC_ICONS = $$PWD/../Resources/Images/App_ico.ico
#RESOURCES = $$PWD/../Resources/Images/App_resources.qrc
#CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT

include(OPENCV.pri)
include(FFTW.pri)
include(ALGLIB.pri)





