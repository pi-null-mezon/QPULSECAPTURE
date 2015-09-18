#--------------------------------------ALGLIB---------------------------------
ALGLIB_DIR = C:/Programming/3rdParties/ALGLIB

INCLUDEPATH += $${ALGLIB_DIR}

SOURCES +=  $${ALGLIB_DIR}/statistics.cpp \
            $${ALGLIB_DIR}/alglibinternal.cpp \
            $${ALGLIB_DIR}/alglibmisc.cpp \
            $${ALGLIB_DIR}/ap.cpp \
            $${ALGLIB_DIR}/dataanalysis.cpp \
            $${ALGLIB_DIR}/diffequations.cpp \
            $${ALGLIB_DIR}/fasttransforms.cpp \
            $${ALGLIB_DIR}/integration.cpp \
            $${ALGLIB_DIR}/interpolation.cpp \
            $${ALGLIB_DIR}/linalg.cpp \
            $${ALGLIB_DIR}/optimization.cpp \
            $${ALGLIB_DIR}/solvers.cpp \
            $${ALGLIB_DIR}/specialfunctions.cpp
#-----------------------------------------------------------------------------
