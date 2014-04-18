#include "mainwindow.h"
#include "settingsdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QPixmap pixmap(":/App_logo.png");
    QSplashScreen splash(pixmap);
    splash.show();

    ptdialog = new Settingsdialog();
    MainWindow window;

    QTimer::singleShot(2000, &splash, SLOT(close()));
    QTimer::singleShot(2100, &window, SLOT(show()));
    return app.exec();
}
