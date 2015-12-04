#Some QWidgets in app will be replaced by QOpenGLWidget class if REPLACE_WIDGET_TO_OPENGLWIDGET is defined
CONFIG += opengl
DEFINES += SAMPLES_FOR_OPENGLWIDGET=3

opengl {
    DEFINES += REPLACE_WIDGET_TO_OPENGLWIDGET
    message(Compiles with OpenGL support)
} else {
    message(Compiles without OpenGL support)
}
#---------------------------------------------------------------------------------------------------------
