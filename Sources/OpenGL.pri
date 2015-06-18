#Some QWidgets in app will be replaced by QOpenGLWidget class if REPLACE_WIDGET_TO_OPENGLWIDGET is defined
CONFIG += opengl

opengl {
    DEFINES += REPLACE_WIDGET_TO_OPENGLWIDGET\
               SAMPLES_FOR_OPENGLWIDGET=3
    message(Compiles with OpenGL support)
} else {
    message(Compiles without OpenGL support)
}
#---------------------------------------------------------------------------------------------------------
