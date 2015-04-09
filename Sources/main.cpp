#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTranslator qtTrans;
    if(qtTrans.load("Translation_Rus")) {
        app.installTranslator(&qtTrans);
    }

    MainWindow window;
    window.show();
    return app.exec();
}
