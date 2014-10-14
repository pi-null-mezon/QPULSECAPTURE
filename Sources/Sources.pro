#-------------------------------------------------
#
# Project created by QtCreator 2014-02-08T18:00:16
#
#-------------------------------------------------

QT +=   core \
        gui \
        multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET =    APV
TEMPLATE =  app
VERSION =   1.0.0.2

SOURCES +=  main.cpp\
            mainwindow.cpp \
            qimagewidget.cpp \
            qopencvprocessor.cpp \
            qvideocapture.cpp \
            qharmonicprocessor.cpp \
    ../Resources/ALGLIB/statistics.cpp \
    ../Resources/ALGLIB/alglibinternal.cpp \
    ../Resources/ALGLIB/alglibmisc.cpp \
    ../Resources/ALGLIB/ap.cpp \
    ../Resources/ALGLIB/dataanalysis.cpp \
    ../Resources/ALGLIB/diffequations.cpp \
    ../Resources/ALGLIB/fasttransforms.cpp \
    ../Resources/ALGLIB/integration.cpp \
    ../Resources/ALGLIB/interpolation.cpp \
    ../Resources/ALGLIB/linalg.cpp \
    ../Resources/ALGLIB/optimization.cpp \
    ../Resources/ALGLIB/solvers.cpp \
    ../Resources/ALGLIB/specialfunctions.cpp \
    qsettingsdialog.cpp \
    qeasyplot.cpp


HEADERS  += mainwindow.h \
            qimagewidget.h \
            qopencvprocessor.h \
            qvideocapture.h \
            about.h \
            qharmonicprocessor.h \
    qsettingsdialog.h \
    qeasyplot.h


FORMS += \
    qsettingsdialog.ui


CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT

#---------------------------------------------OPENCV2.4.8.0------------------------------------------------------
#------------------------------------------------MSCV2010/2012--------------------------------------------------------
win32-msvc* {
        win32-msvc2012:contains(QMAKE_TARGET.arch, x86_64):{ message("Building for 64 bit, mscv2012 compiler")
        win32-msvc2012:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../opencv/build/x64/vc11/lib/ -lopencv_core248 \
                                                                                             -lopencv_highgui248 \
                                                                                             -lopencv_imgproc248 \
                                                                                             -lopencv_objdetect248

        else:win32-msvc2012:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../opencv/build/x64/vc11/lib/ -lopencv_core248d \
                                                                                                -lopencv_highgui248d \
                                                                                                -lopencv_imgproc248d \
                                                                                                -lopencv_objdetect248d
	} else { message("Building for 32 bit, mscv2010 compiler")
        win32-msvc*:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../opencv/build/x86/vc10/lib/ -lopencv_core248 \
                                                                                             -lopencv_highgui248 \
                                                                                             -lopencv_imgproc248 \
                                                                                             -lopencv_objdetect248

        else:win32-msvc*:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../opencv/build/x86/vc10/lib/ -lopencv_core248d \
                                                                                                -lopencv_highgui248d \
                                                                                                -lopencv_imgproc248d \
                                                                                                -lopencv_objdetect248d
	}
}
#-------------------------------------------------MINGW--------------------------------------------------------
win32-g++ {
            win32-g++:contains(QMAKE_TARGET.arch, x86_64):{ message("Building for 64 bit, MinGW compiler")
            win32-g++:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../opencv/build/x64/mingw/bin/ -lopencv_core248 \
                                                                                             -lopencv_highgui248 \
                                                                                             -lopencv_imgproc248 \
                                                                                             -lopencv_objdetect248

            else:win32-g++:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../opencv/build/x64/mingw/lib/ -lopencv_core248d \
                                                                                                -lopencv_highgui248d \
                                                                                                -lopencv_imgproc248d \
                                                                                                -lopencv_objdetect248d
	} else { message("Building for 32 bit, MinGW compiler")
        win32-g++:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../opencv/build/x86/mingw/bin/ -lopencv_core248 \
                                                                                                     -lopencv_highgui248 \
                                                                                                     -lopencv_imgproc248 \
                                                                                                     -lopencv_objdetect248

        else:win32-g++:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../opencv/build/x86/mingw/lib/ -lopencv_core248d \
                                                                                                -lopencv_highgui248d \
                                                                                                -lopencv_imgproc248d \
                                                                                                -lopencv_objdetect248d
	}
}
INCLUDEPATH += $$PWD/../../../opencv/build/include
DEPENDPATH += $$PWD/../../../opencv/build/include
#-------------------------------------------------------------------------------------------------------------

#-----------------------------------------------ALGLIB--------------------------------------------------------
INCLUDEPATH += $$PWD/../Resources/ALGLIB
#-------------------------------------------------------------------------------------------------------------

#------------------------------------------------FFTW---------------------------------------------------------
#for MSVC compiller
win32-msvc*{
        win32-msvc*:contains(QMAKE_TARGET.arch, x86_64):{
            message( "Building for 64 bit, mscv compiler" )
            LIBS += -L$$PWD/../Resources/FFTW/fftw3-64/ -llibfftw3-3
        } else {
            message( "Building for 32 bit, mscv compiler" )
            LIBS += -L$$PWD/../Resources/FFTW/fftw3-32/ -llibfftw3-3
        }
      }
#for MinGW compiller
win32-g++ {
        win32-g++:contains(QMAKE_TARGET.arch, x86_64):{
            message( "Building for 64 bit, MinGW compiler" )
            LIBS += -L$$PWD/../Resources/FFTW/fftw3-64/ -llibfftw3-3
        } else {
            message( "Building for 32 bit, MinGW compiler" )
            LIBS += -L$$PWD/../Resources/FFTW/fftw3-32/ -llibfftw3-3
        }
      }
INCLUDEPATH += $$PWD/../Resources/FFTW
#-------------------------------------------------------------------------------------------------------------





