#include "mainwindow.h"
//------------------------------------------------------------------------------------
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480
//------------------------------------------------------------------------------------

MainWindow::MainWindow()
{
        setWindowTitle(tr("Video processig project"));
        setMinimumSize(240, 240);
        resize(FRAME_WIDTH, FRAME_HEIGHT);

        imagedisplay = new QCVImageWidget(); // Widgets without a parent are “top level” (independent) widgets. All other widgets are drawn within their parent
        setCentralWidget(imagedisplay);

        /* QWidget *topFiller = new QWidget;
        topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);*/
        infoLabel = new QLabel(tr("<i>Choose a menu option, or right-click to invoke a context menu</i>"));
        infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        infoLabel->setAlignment(Qt::AlignCenter);
        QFont lable_font;
        lable_font.setPointSize( 20 );
        lable_font.setBold( true );
        infoLabel->setWordWrap( true );
        infoLabel->setFont( lable_font );
       /* QWidget *bottomFiller = new QWidget;
        bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);*/

        layout = new QVBoxLayout();
        layout->setMargin(10);
       // layout->addWidget(topFiller);
        layout->addWidget(infoLabel);
       // layout->addWidget(bottomFiller);
        imagedisplay->setLayout(layout);

        QString message = tr("A context menu is available by right-clicking");
        statusBar()->showMessage(message);

        createActions();
        createMenus();
        createTimers();
        createThreads();

        ptcppwriter = NULL;
        ptsignalwriter = NULL;
        ptRealTimeProcess = NULL;
}

//------------------------------------------------------------------------------------

void MainWindow::createActions()
{
    newrecordAct = new QAction(tr("&New record"), this);
    newrecordAct->setStatusTip(tr("Start a new record session"));
    connect(newrecordAct, SIGNAL(triggered()), this, SLOT(onpause()));
    connect(newrecordAct, SIGNAL(triggered()), ptdialog, SLOT(exec()));
    connect(ptdialog, SIGNAL(accepted()), this, SLOT(startnewrecord()));

    pauseAct = new QAction(tr("&Pause"), this);
    pauseAct->setStatusTip(tr("Stop a processing session"));
    connect(pauseAct, SIGNAL(triggered()), this, SLOT(onpause()));

    resumeAct = new QAction(tr("&Resume"), this);
    resumeAct->setStatusTip(tr("Resume a processing session"));
    connect(resumeAct, SIGNAL(triggered()), this, SLOT(onresume()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Application exit"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    helpAct = new QAction(tr("&Help"), this);
    helpAct->setStatusTip(tr("Show the application's Help"));
    connect(helpAct, SIGNAL(triggered()), this, SLOT(showhelp()));

    deviceresAct = new QAction(tr("&CamResolution"), this);
    deviceresAct->setStatusTip(tr("Open a video device resolution dialog"));
    connect(deviceresAct, SIGNAL(triggered()), this, SLOT(opendeviceresolutiondialog()));

    devicesetAct = new QAction(tr("&CamSettings"), this);
    devicesetAct->setStatusTip(tr("Open a video device settings dialog"));
    connect(devicesetAct, SIGNAL(triggered()), this, SLOT(opendevicesettingsdialog()));

    directshowAct = new QAction(tr("&DS_dialog"), this);
    directshowAct->setStatusTip(tr("Try to open a device-driver embedded settings dialog"));
    connect(directshowAct, SIGNAL(triggered()), this, SLOT(opendsdialog()));

}

//------------------------------------------------------------------------------------

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newrecordAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAct);
    helpMenu->addAction(aboutAct);
}

//------------------------------------------------------------------------------------

void MainWindow::createThreads()
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<cv::Rect>("cv::Rect");

    ptFrameProcess = new QThread();
    ptImageprocessor = new QOpencvprocessor();
    ptImageprocessor->moveToThread( ptFrameProcess );

    //ptFrameCapture = new QThread(); //It was an attempt to drive application in many threads (but currenntly without this thread application works more predictable)
    ptCapture = new QVideoCapture();
    //ptCapture->moveToThread( ptFrameCapture );


    /*connect(ptCapture, SIGNAL(frame_was_captured(cv::Mat)),
            ptImageprocessor, SLOT(process_face_pulse(cv::Mat)),Qt::BlockingQueuedConnection);*/ //uncomment it and application will work slower, but mainwindow GUI will be stoped untill thread return

    connect(ptImageprocessor, SIGNAL(frame_was_processed(cv::Mat)),
                imagedisplay, SLOT(showImage(cv::Mat)));

    connect(ptImageprocessor, SIGNAL(values_were_evaluated(uint,ulong,ulong,ulong,ulong,double)),
                        this, SLOT(oncountready(uint,ulong,ulong,ulong,ulong,double)));

    connect(imagedisplay, SIGNAL(rect_was_entered(cv::Rect)), ptImageprocessor, SLOT(actualizerect(cv::Rect)));

    //connect(ptFrameCapture, SIGNAL(finished()), ptCapture, SLOT (deleteLater()));
    connect(ptFrameProcess, SIGNAL(finished()), ptImageprocessor, SLOT(deleteLater()));
    connect(ptFrameProcess, &QThread::finished, ptFrameProcess, &QObject::deleteLater);

    //ptFrameCapture->start();
    ptFrameProcess->start();
}

//------------------------------------------------------------------------------------

void MainWindow::createTimers()
{
    evaluation_timer = new QTimer(this);
    evaluation_timer->setTimerType(Qt::PreciseTimer);
    connect(evaluation_timer, SIGNAL(timeout()), this, SLOT(evaluatepulse()));
}

//------------------------------------------------------------------------------------

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(newrecordAct);
    menu.addAction(pauseAct);
    menu.addAction(resumeAct);
    menu.addSeparator();
    menu.addAction(deviceresAct);
    menu.addAction(devicesetAct);
    menu.addAction(directshowAct);
    menu.addSeparator();
    menu.addAction(exitAct);
    menu.exec(event->globalPos());
}

//------------------------------------------------------------------------------------

bool MainWindow::openvideofile(QString videofileName)
{
    if ( videofileName.length() != 0 )
    {
        if ( ptCapture->openfile(videofileName) )
        {
            if ( infoLabel ) // just remove label
            {
                layout->removeWidget(infoLabel);
                delete infoLabel;
                infoLabel = NULL;
            }            
            return true;
        }
        else
        {
            QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "Can not open video cv::capture with this file", QMessageBox::Ok, this, Qt::Dialog);
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "No files had been chosen", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    return false;
}

//------------------------------------------------------------------------------------

bool MainWindow::opencascadefile(QString cascadefileName)
{
    if(ptImageprocessor)
    {
        if( cascadefileName.length() != 0 )
        {
            if( ptImageprocessor->loadclassifier(cascadefileName.toLocal8Bit().constData()) )
            {
                return true;//only if file had been chosen and was opened
            }
            else
            {
                QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "The cv::CascadeClassifier can't load this cascade!", QMessageBox::Ok, this, Qt::Dialog);
                msgBox.exec();
            }
        }
        else
        {
            QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "No files had been chosen", QMessageBox::Ok, this, Qt::Dialog);
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "There is no objects for cascade file loading!", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    return false;
}

//------------------------------------------------------------------------------------

bool MainWindow::opendevice(int devID, int period)
{
    if ( ptCapture->opendevice( devID, period) )
    {
        if ( infoLabel ) // just remove label
        {
            layout->removeWidget(infoLabel);
            delete infoLabel;
            infoLabel = NULL;
        }
        return true;
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "Can not open video capture device", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    return false;
}

//------------------------------------------------------------------------------------

void MainWindow::about()
{
   QDialog *aboutdialog = new QDialog();
   aboutdialog->setWindowTitle("About");
   aboutdialog->setFixedSize(256,128);

   QVBoxLayout *templayout = new QVBoxLayout();
   QLabel *projectname = new QLabel("Pulse capture project, v.3.0.0.0");
   QLabel *projectauthors = new QLabel("Biomedical department of BMSTU, 2014");
   QLabel *hyperlink = new QLabel("<a href='mailto:pi-null-mezon@yandex.ru?subject=Pulse_capture_project'>Contact us at pi-null-mezon@yandex.ru");
   hyperlink->setToolTip("Try to open your default mail client");
   hyperlink->setOpenExternalLinks(true);

   templayout->addWidget(projectname,1,Qt::AlignLeft);
   templayout->addWidget(projectauthors,1,Qt::AlignLeft);
   templayout->addWidget(hyperlink,2,Qt::AlignLeft);

   aboutdialog->setLayout(templayout);
   aboutdialog->exec();

   delete hyperlink;
   delete projectauthors;
   delete projectname;
   delete templayout;
   delete aboutdialog;
}

//------------------------------------------------------------------------------------

void MainWindow::showhelp()
{
    if (!QDesktopServices::openUrl(QUrl("file:///" + QDir::currentPath() + "/myLicense.txt", QUrl::TolerantMode))) // runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "Can not open help file", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    ptCapture->close();

    //ptFrameCapture->quit();
    //ptFrameCapture->wait();

    ptFrameProcess->quit();
    ptFrameProcess->wait();

    if(ptcppwriter)
    {
        ptcppwriter->close();
        delete ptcppwriter;
    }
    if(ptsignalwriter)
    {
        ptsignalwriter->close();
        delete ptsignalwriter;
    }
    delete ptRealTimeProcess;
}

//------------------------------------------------------------------------------------

int MainWindow::startnewrecord()
{

    if(!ptdialog->isCBvideo_checked())
    {
        if( !opendevice( ptCapture->device_select_dialog() , DEFAULT_FRAME_PERIOD) ) return -2; // error code,
    }
    else
    {
        if( !openvideofile( ptdialog->getEvideo_text() ) ) return -3; // error code
    }

    ptCapture->disconnect(); // Disconnect everything connected to an object's signals
    if( ptdialog->isCBcascade_checked() )
    {
        if(!opencascadefile( ptdialog->getEcascade_text() )) return -1; // error code

        connect(ptCapture, SIGNAL(frame_was_captured(cv::Mat)),
                ptImageprocessor, SLOT(process_face_pulse(cv::Mat)),Qt::BlockingQueuedConnection); // process face region
        imagedisplay->setrectflag( false ); // allow to draw aim rect only if CBcascade has been checked
        setStatusTip("Processing was started, face recognition mode");
    }
    else
    {
        connect(ptCapture, SIGNAL(frame_was_captured(cv::Mat)),
                ptImageprocessor, SLOT(process_rectregion_pulse(cv::Mat)),Qt::BlockingQueuedConnection); // process face region
        imagedisplay->setrectflag( true ); // allow to draw aim rect only if CBcascade has been checked
        setStatusTip("Processing was started, select interesting region by mouse");
    }


    startRealTimeProcess(ptdialog->get_datalength(),ptdialog->get_bufferlength(),ptdialog->get_countsperscreen(),ptdialog->isCBrecord_checked(), ptdialog->isCBoutputsignal_checked());
    evaluation_timer->setInterval( ptdialog->getSrecord_position()*1000 );
    onresume();
    return 0; // ok code
}

//------------------------------------------------------------------------------------

void MainWindow::onpause()
{
    evaluation_timer->stop();
    ptCapture->pause();
}

//------------------------------------------------------------------------------------

void MainWindow::onresume()
{
    ptCapture->start();
    evaluation_timer->start();
    ptImageprocessor->actualizetimecounter();
}

//-----------------------------------------------------------------------------------

void MainWindow::startRealTimeProcess(unsigned int datalength, unsigned int bufferlength, unsigned int countsperscreen, bool createoutputfile, bool outputsignal)
{
    if(ptcppwriter) // clean memory
    {
        ptcppwriter->close();
        delete ptcppwriter;
        ptcppwriter = NULL;
    }
    if(ptsignalwriter)
    {
        ptsignalwriter->close();
        delete ptsignalwriter;
        ptsignalwriter = NULL;
    }
    if(ptRealTimeProcess)
    {
        delete ptRealTimeProcess;
        ptRealTimeProcess = NULL;
        imagedisplay->unsetCountsPointer();
    }

    ptRealTimeProcess = new TRealTimeProcessing(datalength, bufferlength);
    imagedisplay->setCountsPointer(ptRealTimeProcess->ptCNSignal, datalength, countsperscreen, ptRealTimeProcess->ptSpectrum, bufferlength); // for CNSignal-trace drawing

    if(createoutputfile)
    {
        ptcppwriter = new std::ofstream(ptdialog->getErecord_text().toLocal8Bit().constData(), std::ios::out);
        *ptcppwriter << "PULSE record\n";
        *ptcppwriter << "dd.MM.yyyy HH:mm:ss    pulse,bpm\n";
    }

    if(outputsignal)
    {
        ptsignalwriter = new std::ofstream( ptdialog->getEsignal_text().toLocal8Bit().constData() , std::ios::out);
        *ptsignalwriter << "PHOTOPLETHYSMOGRAPHY record\n";
        *ptsignalwriter << "dd.MM.yyyy HH:mm:ss.zzz    signal,gen.unit\n";
    }
}

//----------------------------------------------------------------------------------

void MainWindow::oncountready(unsigned int channum, const unsigned long sumred, const unsigned long sumgreen, const unsigned long sumblue, const unsigned long area, const double time)
{
    if(ptRealTimeProcess)
    {
        if((channum == 3) && (ptdialog->isCBcolor_checked()))
        {
            ptRealTimeProcess->WriteToDataColor(sumred, sumgreen, sumblue, area, time);
        }
        else
        {
            ptRealTimeProcess->WriteToDataBW(sumgreen, area, time);
        }

        if(ptsignalwriter)
        {
            *ptsignalwriter << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss.zzz").toLocal8Bit().constData()  << "    " << QString::number( ptRealTimeProcess->getCurentSignalCount(), 'f', 6 ).toLocal8Bit().constData() << "\n";
        }

    imagedisplay->putTextOnPeriod(  QString::number( time, 'g', 4 ) + " ms" );
    }
}

//-----------------------------------------------------------------------------------

void MainWindow::evaluatepulse()
{
    if(ptRealTimeProcess)
    {
        if(ptdialog->isCBtrigger_checked())
        {
            pulse_estimation = (unsigned int)ptRealTimeProcess->EvaluateFrequency();
            imagedisplay->putTextOnSNR( "" );
            imagedisplay->putTextOnPulse(QString::number( pulse_estimation ), QColor(225,225,225));
        }
        else
        {
            pulse_estimation = (unsigned int)ptRealTimeProcess->ComputeFrequency( ptdialog->isCBpca_checked() );
            SNR = ptRealTimeProcess->SNRE;
            imagedisplay->putTextOnSNR( QString::number( SNR, 'f', 3 ) + " dB" );
            if( SNR > SNR_TRESHOLD + 1.0 )
            {
                imagedisplay->putTextOnPulse(QString::number( pulse_estimation ), QColor(0,225,0));
            }
            else
            {
                imagedisplay->putTextOnPulse(QString::number( pulse_estimation ), QColor(225,0,0));
            }
        }

        if( ptcppwriter )
        {
            *ptcppwriter << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss").toLocal8Bit().constData()  << "    " << QString::number( pulse_estimation ).toLocal8Bit().constData() << "\n";
        }   
    }
}

//-----------------------------------------------------------------------------------

void MainWindow::opendeviceresolutiondialog()
{
    onpause();
    if( !ptCapture->open_resolution_dialog() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "Can not open device resolution dialog!", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    onresume();
}

//-----------------------------------------------------------------------------------

void MainWindow::opendevicesettingsdialog()
{
    if( !ptCapture->open_settings_dialog() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "Can not open device settings dialog!", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//-----------------------------------------------------------------------------------

void MainWindow::opendsdialog()
{   
    if (!QProcess::startDetached(QString("WVCF_utility.exe"),QStringList("-l")))// runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), "Can not open utility...", QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    } 
}

//----------------------------------------------------------------------------------------



