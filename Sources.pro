#-------------------------------------------------
#
# Project created by QtCreator 2014-02-08T18:00:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QPulse_capture
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    cvimagewidget.cpp \
    qopencvprocessor.cpp \
    settingsdialog.cpp \
    resources/Algorithmic.cpp \
    resources/ALGLIB_SOURCES/linalg.cpp \
    resources/ALGLIB_SOURCES/specialfunctions.cpp \
    resources/ALGLIB_SOURCES/statistics.cpp \
    resources/ALGLIB_SOURCES/alglibinternal.cpp \
    resources/ALGLIB_SOURCES/alglibmisc.cpp \
    resources/ALGLIB_SOURCES/ap.cpp \
    resources/ALGLIB_SOURCES/dataanalysis.cpp \
    resources/ALGLIB_SOURCES/optimization.cpp \
    resources/ALGLIB_SOURCES/fasttransforms.cpp \
    resources/ALGLIB_SOURCES/solvers.cpp \
    qvideocapture.cpp


HEADERS  += mainwindow.h \
    CVImageWidget.h \
    qopencvprocessor.h \
    settingsdialog.h \
    resources/Algorithmic.h \
    qvideocapture.h

FORMS += \
    settingsdialog.ui

RC_ICONS = App_ico.ico
RESOURCES += \
    Application.qrc


#---------------------------------------OPENCV2.4.8.0------------------------------------------------------

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../opencv/build/x86/vc10/lib/ -lopencv_core248 \
                                                                                       -lopencv_highgui248 \
                                                                                       -lopencv_imgproc248 \
                                                                                       -lopencv_objdetect248


else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../opencv/build/x86/vc10/lib/ -lopencv_core248d \
                                                                                          -lopencv_highgui248d \
                                                                                          -lopencv_imgproc248d \
                                                                                          -lopencv_objdetect248d

INCLUDEPATH += $$PWD/../../opencv/build/include
DEPENDPATH += $$PWD/../../opencv/build/include

#---------------------------------------FFTW import library-----------------------------------------------

win32: LIBS += -L$$PWD/resources/ -llibfftw3f-3

INCLUDEPATH += $$PWD/resources
DEPENDPATH += $$PWD/resources

#---------------------------------------ALGLIB INCLUDE PATH-----------------------------------------------

INCLUDEPATH += $$PWD/resources/ALGLIB_SOURCES





