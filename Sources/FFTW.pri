#------------------------------------------------FFTW---------------------------------------------------------
FFTW_DIR = C:/Programing/3rdParties/FFTW

INCLUDEPATH += $${FFTW_DIR}

win32:contains(QMAKE_TARGET.arch, x86_64){

    message( "FFTW library for 64 bit architecture will be used" )
    LIBS += -L$${FFTW_DIR}/fftw3-64/


} else {

    message( "FFTW library for 32 architecture will be used" )
    LIBS += -L$${FFTW_DIR}/fftw3-32/

}
LIBS += -llibfftw3-3
#-------------------------------------------------------------------------------------------------------------
