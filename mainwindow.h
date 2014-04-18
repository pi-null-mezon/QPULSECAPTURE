#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//------------------------------------------------------------------------------------------------------

#include <QtWidgets>
#include "CVImageWidget.h"
#include "qopencvprocessor.h"
#include "settingsdialog.h"
#include <fstream>
#include "Algorithmic.h"
#include "qvideocapture.h"

//------------------------------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

public slots:
    bool openvideofile(QString videofileName); // for video file open
    bool opendevice(int devID, int period); // for video device open
    bool opencascadefile(QString cascadefileName);
    void opendeviceresolutiondialog();
    void opendevicesettingsdialog();
    void opendsdialog();
    int startnewrecord();
    void about();
    void showhelp();
    void oncountready(unsigned int channum, const unsigned long sumred, const unsigned long sumgreen, const unsigned long sumblue, const unsigned long area, const double time);
    void onpause();
    void onresume();
    void evaluatepulse();

private:
    void createActions();
    void createMenus();
    void createTimers();
    void createThreads();
    void startRealTimeProcess(unsigned int datalength, unsigned int bufferlength, unsigned int countsperscreen, bool createoutputfile,  bool outputsignal);

    QCVImageWidget *imagedisplay; // a pointer to my image display widget, memory for a class instance will be allocated in a constructor
    QVBoxLayout *layout;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *exitAct; 
    QAction *aboutAct;
    QAction *helpAct;
    QAction *newrecordAct;
    QAction *pauseAct;
    QAction *resumeAct;
    QAction *deviceresAct;
    QAction *devicesetAct;
    QAction *directshowAct;
    QLabel *infoLabel;

    QTimer *evaluation_timer;
    QVideoCapture *ptCapture;
    QOpencvprocessor *ptImageprocessor;

    QThread *ptFrameProcess;
    //QThread *ptFrameCapture;
    std::ofstream *ptcppwriter; // pointer for output file processor
    std::ofstream *ptsignalwriter; // a pointer to output file with ptCNSignal[...] processor
    TRealTimeProcessing *ptRealTimeProcess;
    unsigned int pulse_estimation;
    unsigned long count;
    double SNR;
};
//------------------------------------------------------------------------------------------------------
#endif // MAINWINDOW_H
