#include <QDateTime>
#include "mainwindow.h"
#include "qprocessingdialog.h"
//------------------------------------------------------------------------------------

#define FRAME_MARGIN 5
#define MS_INTERVAL 1000

//------------------------------------------------------------------------------------
const char * MainWindow::QPlotDialogName[]=
{
    QT_TR_NOOP("Heart signal vs frame"),
    QT_TR_NOOP("Heart amplitude spectrum"),
    QT_TR_NOOP("Frame time vs frame"),
    QT_TR_NOOP("PCA 1-st projection"),
    QT_TR_NOOP("Filter output vs frame"),
    QT_TR_NOOP("Heart signal phase diagram"),
    QT_TR_NOOP("Breath signal vs frame"),
    QT_TR_NOOP("Breath amplitude spectrum")
};
//------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    setWindowTitle(APP_NAME);
    setMinimumSize(600, 400);

    pt_centralWidget = new QBackgroundWidget(NULL, palette().color(backgroundRole()));
    pt_centralWidgetLayout = new QVBoxLayout();
    this->setCentralWidget(pt_centralWidget);
    pt_centralWidget->setLayout(pt_centralWidgetLayout);
    pt_centralWidgetLayout->setMargin(FRAME_MARGIN);

    //--------------------------------------------------------------
    pt_display = new QImageWidget(); // Widgets without a parent are “top level” (independent) widgets. All other widgets are drawn within their parent
    pt_mainLayout = new QVBoxLayout();
    pt_display->setLayout(pt_mainLayout);
    pt_centralWidgetLayout->addWidget(pt_display);

    //--------------------------------------------------------------
    pt_infoLabel = new QLabel(tr("<i>Start new measurement session</i>"));
    pt_infoLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    pt_infoLabel->setAlignment(Qt::AlignCenter);
    pt_infoLabel->setWordWrap( true );
    pt_infoLabel->setFont( QFont("MS Shell Dlg 2", 14, QFont::Normal) );
    pt_mainLayout->addWidget(pt_infoLabel);

    /*pt_statusLabel = new QLabel();
    this->statusBar()->addPermanentWidget(pt_statusLabel);*/

    //--------------------------------------------------------------
    createActions();
    createMenus();
    createThreads();

    //--------------------------------------------------------------
    m_dialogSetCounter = 0;
    m_sessionsCounter = 0;
    pt_videoSlider = NULL;

    //--------------------------------------------------------------
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(MS_INTERVAL);
    m_timer.stop();

    //--------------------------------------------------------------
    resize(570, 480);
    statusBar()->showMessage(tr("A context menu is available by right-clicking"));
}
//------------------------------------------------------------------------------------

void MainWindow::createActions()
{
    pt_openSessionAct = new QAction(tr("New &session"),this);
    pt_openSessionAct->setStatusTip(tr("Open new measurement session"));
    connect(pt_openSessionAct, SIGNAL(triggered()), this, SLOT(configure_and_start_session()));

    pt_pauseAct = new QAction(tr("&Pause"), this);
    pt_pauseAct->setStatusTip(tr("Stop a measurement session"));
    connect(pt_pauseAct, SIGNAL(triggered()), this, SLOT(onpause()));

    pt_resumeAct = new QAction(tr("&Resume"), this);
    pt_resumeAct->setStatusTip(tr("Resume a measurement session"));
    connect(pt_resumeAct, SIGNAL(triggered()), this, SLOT(onresume()));

    pt_exitAct = new QAction(tr("E&xit"), this);
    pt_exitAct->setStatusTip(tr("See You next time ;)"));
    connect(pt_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    pt_aboutAct = new QAction(tr("&About"), this);
    pt_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(pt_aboutAct, SIGNAL(triggered()), this, SLOT(show_about()));

    pt_helpAct = new QAction(tr("&Help"), this);
    pt_helpAct->setStatusTip(tr("Show the application's Help"));
    connect(pt_helpAct, SIGNAL(triggered()), this, SLOT(show_help()));

    pt_deviceResAct = new QAction(tr("&Resolution"), this);
    pt_deviceResAct->setStatusTip(tr("Open a video device resolution dialog"));
    connect(pt_deviceResAct, SIGNAL(triggered()), this, SLOT(opendeviceresolutiondialog()));

    pt_deviceSetAct = new QAction(tr("&Preset"), this);
    pt_deviceSetAct->setStatusTip(tr("Open a video device settings dialog"));
    connect(pt_deviceSetAct, SIGNAL(triggered()), this, SLOT(opendevicesettingsdialog()));

    pt_DirectShowAct = new QAction(tr("&DSpreset"), this);
    pt_DirectShowAct->setStatusTip(tr("Open a device-driver embedded settings dialog"));
    connect(pt_DirectShowAct, SIGNAL(triggered()), this, SLOT(callDirectShowSdialog()));

    pt_fastVisualizationAct = new QAction(tr("Co&ntour"), this);
    pt_fastVisualizationAct->setStatusTip(tr("Switch between contoured or uncontoured style of text on the screen"));
    pt_fastVisualizationAct->setCheckable(true);
    pt_fastVisualizationAct->setChecked(true);
    connect(pt_fastVisualizationAct, SIGNAL(triggered(bool)),pt_display, SLOT(toggle_advancedvisualization(bool)));

    pt_changeColorsAct = new QAction(tr("TextCo&lor"), this);
    pt_changeColorsAct->setStatusTip(tr("Switch between black or white color of text on the screen"));
    connect(pt_changeColorsAct, SIGNAL(triggered()), pt_display, SLOT(switchColorScheme()));

    pt_openPlotDialog = new QAction(tr("&New plot"), this);
    pt_openPlotDialog->setStatusTip(tr("Create a new window for the visualization of appropriate process"));
    connect(pt_openPlotDialog, SIGNAL(triggered()), this, SLOT(createPlotDialog()));

    pt_recordAct = new QAction(tr("&Signals"), this);
    pt_recordAct->setStatusTip(tr("Start to record signals counts in to output text file"));
    connect(pt_recordAct, SIGNAL(triggered()), this, SLOT(startRecord()));
    pt_recordAct->setCheckable(true);

    pt_colorActGroup = new QActionGroup(this);
    pt_colorMapper = new QSignalMapper(this);
    pt_redAct = new QAction(tr("Red"), pt_colorActGroup);
    pt_redAct->setStatusTip(tr("Enroll only red channel"));
    pt_redAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_redAct,0);
    connect(pt_redAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_greenAct = new QAction(tr("Green"), pt_colorActGroup);
    pt_greenAct->setStatusTip(tr("Enroll only green channel"));
    pt_greenAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_greenAct,1);
    connect(pt_greenAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_blueAct = new QAction(tr("Blue"), pt_colorActGroup);
    pt_blueAct->setStatusTip(tr("Enroll only blue channel"));
    pt_blueAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_blueAct,2);
    connect(pt_blueAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_allAct = new QAction(tr("RGB"), pt_colorActGroup);
    pt_allAct->setStatusTip(tr("Enroll all channels"));
    pt_allAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_allAct,3);
    connect(pt_allAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_experimentalAct = new QAction(tr("Experimental"), pt_colorActGroup);
    pt_experimentalAct->setStatusTip(tr("Experimental color enrollment scheme"));
    pt_experimentalAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_experimentalAct, 4);
    connect(pt_experimentalAct,SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_greenAct->setChecked(true);

    pt_pcaAct = new QAction(tr("PCA align"), this);
    pt_pcaAct->setStatusTip(tr("Control PCA alignment, affects on result only in harmonic analysis mode"));
    pt_pcaAct->setCheckable(true);

    pt_mapAct = new QAction(tr("Mapping"), this);
    pt_mapAct->setStatusTip(tr("Map clarity of a pulse signal on image"));
    pt_mapAct->setCheckable(true);
    connect(pt_mapAct, SIGNAL(triggered()), this, SLOT(openMapDialog()));

    pt_selectAllAct = new QAction(tr("&Select all"), this);
    pt_selectAllAct->setStatusTip(tr("Select whole image"));
    connect(pt_selectAllAct, SIGNAL(triggered()), pt_display, SLOT(selectWholeImage()));

    pt_skinAct = new QAction(tr("&Only skin"), this);
    pt_skinAct->setStatusTip(tr("Enroll pixels wih color close to skin only"));
    pt_skinAct->setCheckable(true);
    pt_skinAct->setChecked(true);

    pt_adjustAct = new QAction(tr("&Timing"), this);
    pt_adjustAct->setStatusTip(tr("Allows to adjust time between frequency evaluations & data normalization interval"));
    connect(pt_adjustAct, SIGNAL(triggered()), this, SLOT(openProcessingDialog()));

    pt_imageAct = new QAction(tr("&Image"), this);
    pt_imageAct->setStatusTip(tr("View captured image on main window"));
    pt_imageAct->setCheckable(true);
    pt_imageAct->setChecked(true);
    connect(pt_imageAct, SIGNAL(triggered(bool)), pt_display, SLOT(setImageFlag(bool)));

    pt_calibAct = new QAction(tr("&Calibrate"), this);
    pt_calibAct->setStatusTip(tr("Calibrate color screening algorithm on selected region"));
    pt_calibAct->setCheckable(true);
    pt_calibAct->setChecked(false);

    pt_measRecAct = new QAction(tr("&Measurements"), this);
    pt_measRecAct->setStatusTip(tr("Start to record heart rate & breath rate in to output text file"));
    pt_measRecAct->setCheckable(true);
    connect(pt_measRecAct, SIGNAL(triggered()), this, SLOT(startMeasurementsRecord()));

    pt_prunAct = new QAction(tr("Pruning"), this);
    pt_prunAct->setStatusTip(tr("Toggles experimental color pruning algorithm"));
    pt_prunAct->setCheckable(true);
    pt_prunAct->setChecked(false);
}

//------------------------------------------------------------------------------------

void MainWindow::createMenus()
{
    pt_fileMenu = this->menuBar()->addMenu(tr("&Session"));
    pt_fileMenu->addAction(pt_openSessionAct);
    pt_fileMenu->addSeparator();
    pt_fileMenu->addAction(pt_exitAct);

    //------------------------------------------------
    pt_optionsMenu = menuBar()->addMenu(tr("&Options"));
    pt_optionsMenu->addAction(pt_openPlotDialog);
    pt_optionsMenu->addAction(pt_mapAct);
    pt_optionsMenu->addSeparator();
    pt_colormodeMenu = pt_optionsMenu->addMenu(tr("&Light"));
    pt_colormodeMenu->addActions(pt_colorActGroup->actions());
    pt_modeMenu = pt_optionsMenu->addMenu(tr("&Mode"));
    pt_modeMenu->addAction(pt_pcaAct);
    pt_modeMenu->addSeparator();
    pt_modeMenu->addAction(pt_skinAct);
    pt_modeMenu->addAction(pt_calibAct);
    pt_modeMenu->addSeparator();
    pt_modeMenu->addAction(pt_prunAct);
    pt_optionsMenu->setEnabled(false);

    pt_RecordsMenu = this->menuBar()->addMenu(tr("&Records"));
    pt_RecordsMenu->addAction(pt_recordAct);
    pt_RecordsMenu->addAction(pt_measRecAct);
    pt_RecordsMenu->setEnabled(false);

    pt_appearenceMenu = menuBar()->addMenu(tr("&Appearence"));
    pt_appearenceMenu->addAction(pt_fastVisualizationAct);
    pt_appearenceMenu->addAction(pt_changeColorsAct);
    pt_appearenceMenu->addAction(pt_imageAct);

    //-------------------------------------------------
    pt_deviceMenu = menuBar()->addMenu(tr("&Device"));
    pt_deviceMenu->addAction(pt_deviceSetAct);
    pt_deviceMenu->addAction(pt_deviceResAct);
    pt_deviceMenu->addSeparator();
    pt_deviceMenu->addAction(pt_DirectShowAct);

    //--------------------------------------------------
    pt_helpMenu = menuBar()->addMenu(tr("&Help"));
    pt_helpMenu->addAction(pt_helpAct);
    pt_helpMenu->addAction(pt_aboutAct);
}

//------------------------------------------------------------------------------------

void MainWindow::createThreads()
{
    //-------------------Pointers for objects------------------------
    pt_improcThread = new QThread(this); // Make an own QThread for opencv interface
    pt_opencvProcessor = new QOpencvProcessor();
    pt_opencvProcessor->moveToThread( pt_improcThread );
    connect(pt_improcThread, SIGNAL(finished()), pt_opencvProcessor, SLOT(deleteLater()));
    connect(pt_skinAct, SIGNAL(triggered(bool)), pt_opencvProcessor, SLOT(setSkinSearchingFlag(bool)));
    connect(pt_calibAct, SIGNAL(triggered(bool)), pt_opencvProcessor, SLOT(calibrate(bool)));
    //---------------------------------------------------------------

    pt_harmonicProcessor = NULL;
    pt_harmonicThread = NULL;
    pt_map = NULL;
    pt_mapThread = NULL;

    //--------------------QVideoCapture------------------------------
    pt_videoThread = new QThread(this);
    pt_videoCapture = new QVideoCapture();
    pt_videoCapture->moveToThread(pt_videoThread);
    connect(pt_videoThread, &QThread::started, pt_videoCapture, &QVideoCapture::initiallizeTimer);
    connect(pt_videoThread, &QThread::finished, pt_videoCapture, &QVideoCapture::close);
    connect(pt_videoThread, &QThread::finished, pt_videoCapture, &QVideoCapture::deleteLater);

    //----------Register openCV types in Qt meta-type system---------
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<cv::Rect>("cv::Rect");

    //----------------------Connections------------------------------
    connect(pt_opencvProcessor, SIGNAL(frameProcessed(cv::Mat,double,quint32)), pt_display, SLOT(updateImage(cv::Mat,double,quint32)), Qt::BlockingQueuedConnection);
    connect(pt_display, SIGNAL(rect_was_entered(cv::Rect)), pt_opencvProcessor, SLOT(setRect(cv::Rect)));
    connect(pt_opencvProcessor, SIGNAL(selectRegion(const char*)), pt_display, SLOT(set_warning_status(const char*)));
    connect(pt_opencvProcessor, SIGNAL(mapRegionUpdated(cv::Rect)), pt_display, SLOT(updadeMapRegion(cv::Rect)));
    connect(this, &MainWindow::pauseVideo, pt_videoCapture, &QVideoCapture::pause);
    connect(this, &MainWindow::resumeVideo, pt_videoCapture, &QVideoCapture::resume);
    connect(this, &MainWindow::closeVideo, pt_videoCapture, &QVideoCapture::close);
    connect(this, &MainWindow::updateTimer, pt_opencvProcessor, &QOpencvProcessor::updateTime);
    //----------------------Thread start-----------------------------
    pt_improcThread->start();
    pt_videoThread->start();
}

//------------------------------------------------------------------------------------

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(pt_selectAllAct);
    //menu.addAction(pt_openSessionAct);
    menu.addSeparator();
    menu.addAction(pt_pauseAct);
    menu.addAction(pt_resumeAct);
    menu.addSeparator();
    menu.addAction(pt_adjustAct);
    menu.addSeparator();   
    menu.exec(event->globalPos());
}

//------------------------------------------------------------------------------------

bool MainWindow::openvideofile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open video file"), "/video", tr("Video (*.avi *.mp4 *.wmv *.mpeg1)"));
    while( !pt_videoCapture->openfile(fileName) ) {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open video file!"), QMessageBox::Open | QMessageBox::Cancel, this, Qt::Dialog);
        if( msgBox.exec() == QMessageBox::Open )
        {
            fileName = QFileDialog::getOpenFileName(this, tr("Open video file"), "/video", tr("Video (*.avi *.mp4 *.wmv *.mpeg1)"));
        } else {
            return false;
        }
    }
    if ( pt_infoLabel ) {
        pt_mainLayout->removeWidget(pt_infoLabel);
        delete pt_infoLabel;
        pt_infoLabel = NULL;
    }

    delete pt_videoSlider;
    pt_videoSlider = NULL;
    pt_videoSlider = new QVideoSlider(this);
    pt_videoSlider->setRange(0, (int)pt_videoCapture->getFrameCounts());
    pt_videoSlider->setOrientation(Qt::Horizontal);
    pt_videoSlider->setTickPosition(QSlider::TicksBothSides);
    pt_videoSlider->setTickInterval(32);
    pt_centralWidgetLayout->addWidget(pt_videoSlider);
    connect(pt_videoCapture, SIGNAL(capturedFrameNumber(int)), pt_videoSlider, SLOT(setValue(int)));
    connect(pt_videoSlider, SIGNAL(sliderPressed()), this, SLOT(onpause()));
    connect(pt_videoSlider, SIGNAL(sliderReleased(int)), pt_videoCapture, SLOT(setFrameNumber(int)));
    connect(pt_videoSlider, SIGNAL(sliderReleased(int)), this, SLOT(onresume()));
    return true;
}


//------------------------------------------------------------------------------------

bool MainWindow::opendevice()
{
    pt_videoCapture->open_deviceSelectDialog();
    while( !pt_videoCapture->opendevice() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open device!"), QMessageBox::Open | QMessageBox::Cancel, this, Qt::Dialog);
        if( msgBox.exec() == QMessageBox::Open )
        {
            pt_videoCapture->open_deviceSelectDialog();
        } else {
            return false;
        }
    }
    if ( pt_infoLabel ) {
        pt_mainLayout->removeWidget(pt_infoLabel);
        delete pt_infoLabel;
        pt_infoLabel = NULL;
    }

    delete pt_videoSlider;
    pt_videoSlider = NULL;
    return true;
}

//------------------------------------------------------------------------------------

void MainWindow::show_about()
{
   QDialog *aboutdialog = new QDialog();
   aboutdialog->setWindowTitle("About dialog");
   aboutdialog->setFixedSize(232,128);

   QVBoxLayout *templayout = new QVBoxLayout();
   templayout->setMargin(5);

   QLabel *projectname = new QLabel( QString(APP_NAME) + " " + QString(APP_VERSION) );
   projectname->setFrameStyle(QFrame::Box | QFrame::Raised);
   projectname->setAlignment(Qt::AlignCenter);
   QLabel *projectauthors = new QLabel( QString(APP_AUTHOR) + "\n\n" + QString(APP_COMPANY) + "\n\n" + QString(APP_RELEASE_DATE) );
   projectauthors->setAlignment(Qt::AlignCenter);
   QLabel *hyperlink = new QLabel( APP_EMAIL );
   hyperlink->setToolTip("Tap here to send an email");
   hyperlink->setOpenExternalLinks(true);
   hyperlink->setAlignment(Qt::AlignCenter);

   templayout->addWidget(projectname);
   templayout->addWidget(projectauthors);
   templayout->addWidget(hyperlink);

   aboutdialog->setLayout(templayout);
   aboutdialog->exec();

   delete hyperlink;
   delete projectauthors;
   delete projectname;
   delete templayout;
   delete aboutdialog;
}

//------------------------------------------------------------------------------------

void MainWindow::show_help()
{
    if (!QDesktopServices::openUrl(QUrl("https://github.com/pi-null-mezon/qpulsecapture.git", QUrl::TolerantMode))) // runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open help file"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    if(m_signalsFile.isOpen())
    {
        m_signalsFile.close();
    }
    if(m_measurementsFile.isOpen())
    {
        m_measurementsFile.close();
    }

    pt_videoThread->quit();
    pt_videoThread->wait();

    pt_improcThread->quit();
    pt_improcThread->wait();

    if(pt_map)
    {
        pt_mapThread->quit();
        pt_mapThread->wait();
    }

    if(pt_harmonicThread)
    {
        pt_harmonicThread->quit();
        pt_harmonicThread->wait();
    }
}

//------------------------------------------------------------------------------------

void MainWindow::onpause()
{
    emit pauseVideo();
    m_timer.stop();
}

//------------------------------------------------------------------------------------

void MainWindow::onresume()
{
    emit resumeVideo();
    emit updateTimer();
    m_timer.start();
}

//-----------------------------------------------------------------------------------

void MainWindow::opendeviceresolutiondialog()
{
    onpause();
    if( !pt_videoCapture->open_resolutionDialog() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open device resolution dialog!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    onresume();
}

//-----------------------------------------------------------------------------------

void MainWindow::opendevicesettingsdialog()
{
    if( !pt_videoCapture->open_settingsDialog() )
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open device settings dialog!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//-----------------------------------------------------------------------------------

void MainWindow::callDirectShowSdialog()
{   
    if (!QProcess::startDetached(QString("WVCF_utility.exe"),QStringList("-l -c")))// runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open utility!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//----------------------------------------------------------------------------------------

void MainWindow::configure_and_start_session()
{
    this->onpause();
    if(m_settingsDialog.exec() == QDialog::Accepted)
    {     
        closeAllDialogs();
        if(pt_harmonicProcessor)
        {
            pt_harmonicThread->quit();
            pt_harmonicThread->wait();
        }       
        //---------------------Harmonic processor------------------------
        pt_harmonicThread = new QThread(this);
        pt_harmonicProcessor = new QHarmonicProcessor(NULL, m_settingsDialog.get_datalength(), m_settingsDialog.get_bufferlength());
        pt_harmonicProcessor->moveToThread(pt_harmonicThread);
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicProcessor, SLOT(deleteLater()));
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicThread, SLOT(deleteLater()));
        //---------------------------------------------------------------
        if(m_signalsFile.isOpen()) {
            m_signalsFile.close();
            pt_recordAct->setChecked(false);
        }
        if(m_measurementsFile.isOpen()) {
            disconnect(pt_harmonicProcessor, SIGNAL(measurementsUpdated(qreal,qreal,qreal,qreal)), this, SLOT(updateMeasurementsRecord(qreal,qreal,qreal,qreal)));
            m_measurementsFile.close();
            pt_measRecAct->setChecked(false);

        }
        //---------------------------------------------------------------
        if(m_settingsDialog.get_customPatientFlag())
        {
            if(pt_harmonicProcessor->loadWarningRates(m_settingsDialog.get_stringDistribution().toLocal8Bit().constData(),(QHarmonicProcessor::SexID)m_settingsDialog.get_patientSex(),m_settingsDialog.get_patientAge(),(QHarmonicProcessor::TwoSideAlpha)m_settingsDialog.get_patientPercentile()) == QHarmonicProcessor::FileExistanceError)
            {
                QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not find population distribution file"), QMessageBox::Ok, this, Qt::Dialog);
                msgBox.exec();
            }
        }
        //---------------------------------------------------------------
        disconnect(pt_videoCapture,SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(faceProcess(cv::Mat)));
        disconnect(pt_videoCapture,SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(rectProcess(cv::Mat)));
        if(m_settingsDialog.get_flagCascade())
        {
            QString filename = m_settingsDialog.get_stringCascade();
            while(!pt_opencvProcessor->loadClassifier(filename.toStdString()))
            {
                QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not load classifier file"), QMessageBox::Ok | QMessageBox::Open, this, Qt::Dialog);
                if(msgBox.exec() == QMessageBox::Open)
                {
                    QString temp_filename = QFileDialog::getOpenFileName(this, tr("Open file"), "haarcascades/", tr("Cascade (*.xml)"));
                    if(!temp_filename.isEmpty())
                    {
                        filename = temp_filename;
                    }
                }
                else
                {
                    break;
                }
            }
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(faceProcess(cv::Mat)), Qt::BlockingQueuedConnection);
        }
        else
        {
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(rectProcess(cv::Mat)), Qt::BlockingQueuedConnection);
        }
        //--------------------------------------------------------------      
        if(m_settingsDialog.get_FFTflag())
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(computeHeartRate()));
        }
        else
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(CountFrequency()));
        }
        connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(computeBreathRate()));

        connect(pt_opencvProcessor, SIGNAL(dataCollected(ulong,ulong,ulong,ulong,double)), pt_harmonicProcessor, SLOT(EnrollData(ulong,ulong,ulong,ulong,double)));
        connect(pt_harmonicProcessor, SIGNAL(heartTooNoisy(qreal)), pt_display, SLOT(clearFrequencyString(qreal)));
        connect(pt_harmonicProcessor, SIGNAL(heartRateUpdated(qreal,qreal,bool)), pt_display, SLOT(updateValues(qreal,qreal,bool)));
        connect(pt_harmonicProcessor, SIGNAL(breathRateUpdated(qreal,qreal)), pt_display, SLOT(updateBreathStrings(qreal,qreal)));
        connect(pt_harmonicProcessor, SIGNAL(breathTooNoisy(qreal)), pt_display, SLOT(clearBreathRateString(qreal)));
        connect(pt_harmonicProcessor, SIGNAL(spO2Updated(qreal)), pt_display, SLOT(updateSPO2(qreal)));
        connect(pt_colorMapper, SIGNAL(mapped(int)), pt_harmonicProcessor, SLOT(switchColorMode(int)));
        connect(pt_pcaAct, SIGNAL(triggered(bool)), pt_harmonicProcessor, SLOT(setPCAMode(bool)));
        connect(pt_prunAct, SIGNAL(triggered(bool)), pt_harmonicProcessor, SLOT(setPruning(bool)));
        connect(pt_harmonicProcessor, SIGNAL(CurrentValues(qreal,qreal,qreal,qreal)), this, SLOT(make_record_to_file(qreal,qreal,qreal,qreal)));
        pt_harmonicThread->start();

        m_timer.setInterval( m_settingsDialog.get_timerValue() );     
        pt_greenAct->trigger(); // because green channel is default in QHarmonicProcessor
        pt_prunAct->setChecked(false);
        pt_pcaAct->setChecked(false);

        if(m_sessionsCounter == 0)
        {
            pt_optionsMenu->setEnabled(true);
            pt_RecordsMenu->setEnabled(true);
        }

        if(m_settingsDialog.get_flagVideoFile())
        {
            if(this->openvideofile()) {
                if(m_sessionsCounter == 0)
                    QTimer::singleShot(1500, this, SLOT(onresume())); // should solve issue with first launch suspension
                else
                    this->onresume();
            }
        }
        else
        {
            if(this->opendevice()) {
                if(m_sessionsCounter == 0)
                    QTimer::singleShot(1500, this, SLOT(onresume())); // should solve issue with first launch suspension
                else
                    this->onresume();     // should solve issue with first launch suspension
            }
        }
        this->statusBar()->showMessage(tr("Plot options available through Menu->Options->New plot"));
        m_sessionsCounter++;
    } else {
        //pt_optionsMenu->setEnabled(false);
        //emit closeVideo();
        statusBar()->showMessage(tr("You can prolong Paused session by means of Resume option in context menu"));
    }
}

//------------------------------------------------------------------------------------------

void MainWindow::createPlotDialog()
{
    if(m_dialogSetCounter < LIMIT_OF_DIALOGS_NUMBER)
    {
        QDialog dialog;
        QVBoxLayout centralLayout;
        dialog.setLayout(&centralLayout);
        QGroupBox groupBox(tr("Select appropriate plot type:"));
        QHBoxLayout buttonsLayout;
        centralLayout.addWidget(&groupBox);
        centralLayout.addLayout(&buttonsLayout);
        QPushButton acceptButton(tr("Accept"));
        QPushButton rejectButton(tr("Cancel"));
        buttonsLayout.addWidget(&acceptButton);
        buttonsLayout.addWidget(&rejectButton);
        connect(&acceptButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(&rejectButton, &QPushButton::clicked, &dialog, &QDialog::reject);

        QVBoxLayout groupBoxLayout;
        groupBox.setLayout(&groupBoxLayout);
        QComboBox dialogTypeComboBox;
        groupBoxLayout.addWidget(&dialogTypeComboBox);
        for(quint8 i = 0; i < sizeof(QPlotDialogName)/sizeof(char*); i++)
        {
            dialogTypeComboBox.addItem( tr(QPlotDialogName[i]) );
        }
        dialogTypeComboBox.setCurrentIndex(0);

        dialog.setWindowTitle(tr("Plot select dialog"));
        dialog.setFixedSize(256,128);

        if(dialog.exec() == QDialog::Accepted)
        {
            pt_dialogSet[ m_dialogSetCounter ] = new QDialog(NULL, Qt::Window);
            pt_dialogSet[ m_dialogSetCounter ]->setWindowTitle(dialogTypeComboBox.currentText());
            pt_dialogSet[ m_dialogSetCounter ]->setAttribute(Qt::WA_DeleteOnClose, true);
            connect(pt_dialogSet[ m_dialogSetCounter ], SIGNAL(destroyed()), this, SLOT(decrease_dialogSetCounter()));
            pt_dialogSet[ m_dialogSetCounter ]->setMinimumSize(480, 320);

            QVBoxLayout *pt_layout = new QVBoxLayout( pt_dialogSet[ m_dialogSetCounter ] );
            pt_layout->setMargin(FRAME_MARGIN);
            QEasyPlot *pt_plot = new QEasyPlot( pt_dialogSet[ m_dialogSetCounter ] );
            pt_layout->addWidget( pt_plot );          
                switch(dialogTypeComboBox.currentIndex())
                {
                    case 0: // Signal trace
                        connect(pt_harmonicProcessor, SIGNAL(heartSignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Frame"),tr("Centered & normalized signal"));
                        pt_plot->set_vertical_Borders(-4.0,4.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        break;
                    case 1: // Spectrum trace
                        connect(pt_harmonicProcessor, SIGNAL(heartSpectrumUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Freq.count"),tr("DFT amplitude spectrum"));
                        pt_plot->set_vertical_Borders(0.0,1.0);
                        pt_plot->set_coordinatesPrecision(0,1);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        pt_plot->set_tracePen(QPen(Qt::NoBrush,1.0), QColor(255,0,0));
                        break;
                    case 2: // Time trace
                        connect(pt_harmonicProcessor, SIGNAL(TimeUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Frame"),tr("processing period per frame, ms"));
                        pt_plot->set_vertical_Borders(0.0,100.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        pt_plot->set_tracePen(QPen(Qt::NoBrush,1.0), QColor(255,0,255));
                        break;
                    case 3: // PCA 1st projection trace
                        connect(pt_harmonicProcessor, SIGNAL(PCAProjectionUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Frame"),tr("Normalised & centered projection on 1-st PCA direction"));
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 4: // Digital filter output
                        connect(pt_harmonicProcessor, SIGNAL(BinaryOutputUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Frame"),tr("Digital derivative after smoothing"));
                        pt_plot->set_vertical_Borders(-2.0,2.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        pt_plot->set_tracePen(QPen(Qt::NoBrush,1.0), QColor(255,255,0));
                    break;
                    case 5: // signal phase shift
                        connect(pt_harmonicProcessor, SIGNAL(heartSignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_DrawRegime(QEasyPlot::PhaseRegime);
                        pt_plot->set_axis_names(tr("Signal count"),tr("Signal count"));
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_horizontal_Borders(-5.0, 5.0);
                        pt_plot->set_X_Ticks(11);
                        pt_plot->set_coordinatesPrecision(2,2);
                    break;
                    case 6: // signal phase shift
                        connect(pt_harmonicProcessor, SIGNAL(breathSignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Frame"),tr("Signal count"));
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_X_Ticks(11);
                        pt_plot->set_coordinatesPrecision(0,2);
                        pt_plot->set_tracePen(QPen(Qt::NoBrush,1.0), QColor(0,255,255));
                    break;
                    case 7: // Spectrum trace
                        connect(pt_harmonicProcessor, SIGNAL(breathSpectrumUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names(tr("Freq.count"),tr("DFT amplitude spectrum"));
                        pt_plot->set_vertical_Borders(0.0,1.0);
                        pt_plot->set_coordinatesPrecision(0,1);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        pt_plot->set_tracePen(QPen(Qt::NoBrush,1.0), QColor(255,0,0));
                    break;
                }
            pt_dialogSet[ m_dialogSetCounter ]->setContextMenuPolicy(Qt::ActionsContextMenu);
            QAction *pt_actionFont = new QAction(tr("Axis font"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionFont, SIGNAL(triggered()), pt_plot, SLOT(open_fontSelectDialog()));
            QAction *pt_actionTraceColor = new QAction(tr("Trace color"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionTraceColor, SIGNAL(triggered()), pt_plot, SLOT(open_traceColorDialog()));
            QAction *pt_actionBGColor = new QAction(tr("BG color"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionBGColor, SIGNAL(triggered()), pt_plot, SLOT(open_backgroundColorDialog()));
            QAction *pt_actionCSColor = new QAction(tr("CS color"), pt_dialogSet[ m_dialogSetCounter ]);
            connect(pt_actionCSColor, SIGNAL(triggered()), pt_plot, SLOT(open_coordinatesystemColorDialog()));
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionTraceColor);
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionBGColor);
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionCSColor);
            pt_dialogSet[ m_dialogSetCounter ]->addAction(pt_actionFont);
            pt_dialogSet[ m_dialogSetCounter ]->show();
            m_dialogSetCounter++;
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("You came up to a limit of dialogs ailable"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }

}

//------------------------------------------------------------------------------------------

void MainWindow::decrease_dialogSetCounter()
{
    m_dialogSetCounter--;
}

//-------------------------------------------------------------------------------------------

void MainWindow::make_record_to_file(qreal signalValue, qreal meanRed, qreal meanGreen, qreal meanBlue)
{
    if(m_signalsFile.isOpen())
    {
        m_signalsStream << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                     << "\t" << signalValue << "\t" << meanRed << "\t" << meanGreen
                     << "\t" << meanBlue << "\n";
    }
}

//-------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent*)
{
    closeAllDialogs();
    disconnect(pt_opencvProcessor, SIGNAL(frameProcessed(cv::Mat,double,quint32)), pt_display, SLOT(updateImage(cv::Mat,double,quint32)));
}

//-------------------------------------------------------------------------------------------

void MainWindow::closeAllDialogs()
{
    for(qint8 i = m_dialogSetCounter; i > 0; i--)
    {
        pt_dialogSet[ i-1 ]->close(); // there is no need to explicitly decrement m_dialogSetCounter value because pt_dialogSet[i] was preset to Qt::WA_DeleteOnClose flag and on_destroy of pt_dialogSet[i] 'this' will decrease counter automatically
    };
}

//-------------------------------------------------------------------------------------------

void MainWindow::startRecord()
{
    if(m_signalsFile.isOpen()) {
        m_signalsFile.close();
        pt_recordAct->setChecked(false);
        QMessageBox msgBox(QMessageBox::Question, this->windowTitle(), tr("Another record?"), QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::No)
        {
            return;
        }
    }

    m_signalsFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to a file"),"Records/ID" + QString::number(m_sessionsCounter)+ "_signals.txt", "Text file (*.txt)"));

    while(!m_signalsFile.open(QIODevice::WriteOnly))   {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not save file, try another name"), QMessageBox::Save | QMessageBox::Cancel, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::Save) {
            m_signalsFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to a file"),"Records/ID" + QString::number(m_sessionsCounter)+ "_signals.txt", tr("Text file (*.txt)")));
        }
        else {
            pt_recordAct->setChecked(false);
            break;
        }
    }

    if(m_signalsFile.isOpen()) {
        pt_recordAct->setChecked(true);
        m_signalsStream.setDevice(&m_signalsFile);
        m_signalsStream.setRealNumberNotation(QTextStream::FixedNotation);
        m_signalsStream.setRealNumberPrecision(3);
        m_signalsStream << "QPULSECAPTURE SIGNALS RECORD of " << QDateTime::currentDateTime().toString("dd.MM.yyyy")
                        << "\nhh:mm:ss.zzz\tSignal\tRed\tGreen\tBlue\n";
    }
}

//----------------------------------------------------------------------------------------------

void MainWindow::openMapDialog()
{
    cv::Rect tempRect = pt_opencvProcessor->getRect();
    if((tempRect.width <= 0) || (tempRect.height <= 0))
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Select region on image first"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
        pt_mapAct->setChecked(false);
        return;
    }
    else
    {
            QMessageBox msgBox(QMessageBox::Question, this->windowTitle(), tr("Another map?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this, Qt::Dialog);
            int resultCode = msgBox.exec();
            if(resultCode == QMessageBox::Cancel)
            {
                pt_mapAct->setChecked(false);
                return;
            }
            //--------Clean memory and resources------
            if(pt_map)
            {
                disconnect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(mapProcess(cv::Mat)));
                disconnect(&m_timer, SIGNAL(timeout()), pt_map, SIGNAL(updateMap()));
                disconnect(pt_map, SIGNAL(mapUpdated(const qreal*,quint32,quint32,qreal,qreal)), pt_display, SLOT(updateMap(const qreal*,quint32,quint32,qreal,qreal)));
                pt_display->clearMap();
                if(pt_map)
                {
                    pt_mapThread->quit();
                    pt_mapThread->wait();
                }
                delete pt_map;
                pt_map = NULL;
            }
            pt_mapAct->setChecked(false);
            //-----------------End-------------------
            if(resultCode == QMessageBox::Yes)
            {
                mappingdialog dialog;
                dialog.setImageHeight(tempRect.height);
                dialog.setImageWidth(tempRect.width);

                if(dialog.exec() == QDialog::Accepted)
                {
                    pt_opencvProcessor->setMapCellSize(dialog.getCellSize(), dialog.getCellSize());
                    pt_opencvProcessor->setMapRegion(cv::Rect(tempRect.x,tempRect.y,dialog.getCellSize()*dialog.getMapWidth(),dialog.getCellSize()*dialog.getMapHeight()));

                    pt_mapThread = new QThread(this);
                    pt_map = new QHarmonicProcessorMap(NULL, dialog.getMapWidth(), dialog.getMapHeight());
                    pt_map->setMapType(dialog.getMapType(), dialog.getSNRControl());
                    pt_map->moveToThread(pt_mapThread);
                    connect(pt_opencvProcessor, SIGNAL(mapCellProcessed(ulong,ulong,ulong,ulong,double)), pt_map, SLOT(updateHarmonicProcessor(ulong,ulong,ulong,ulong,double)), Qt::BlockingQueuedConnection);
                    connect(&m_timer, SIGNAL(timeout()), pt_map, SIGNAL(updateMap()));
                    connect(pt_map, SIGNAL(mapUpdated(const qreal*,quint32,quint32,qreal,qreal)), pt_display, SLOT(updateMap(const qreal*,quint32,quint32,qreal,qreal)));
                    connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(mapProcess(cv::Mat)), Qt::BlockingQueuedConnection);
                    connect(pt_pcaAct, SIGNAL(triggered(bool)), pt_map, SIGNAL(updatePCAMode(bool)));
                    connect(pt_colorMapper, SIGNAL(mapped(int)), pt_map, SIGNAL(changeColorChannel(int)));
                    connect(pt_mapThread, SIGNAL(finished()), pt_mapThread, SLOT(deleteLater()));
                    pt_mapThread->start(QThread::HighestPriority);
                    pt_mapAct->setChecked(true);
                }
            }
    }
}

//-----------------------------------------------------------------------------------

void MainWindow::openProcessingDialog()
{
    if(pt_harmonicProcessor) {

        QProcessingDialog *dialog = new QProcessingDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
        dialog->setTimer(m_timer.interval());
        dialog->setLimits(pt_harmonicProcessor->getDataLength());
        dialog->setValues(pt_harmonicProcessor->getEstimationInterval(), pt_harmonicProcessor->getBreathStrobe(), pt_harmonicProcessor->getBreathAverage(), pt_harmonicProcessor->getBreathCNInterval());
        connect(dialog, &QProcessingDialog::timerValueUpdated, &m_timer, &QTimer::setInterval);
        connect(dialog, SIGNAL(intervalValueUpdated(int)), pt_harmonicProcessor, SLOT(setEstiamtionInterval(int)));
        connect(dialog, SIGNAL(breathStrobeUpdated(int)), pt_harmonicProcessor, SLOT(setBreathStrobe(int)));
        connect(dialog, SIGNAL(breathAverageUpdated(int)), pt_harmonicProcessor, SLOT(setBreathAverage(int)));
        connect(dialog, SIGNAL(breathCNIntervalUpdated(int)), pt_harmonicProcessor, SLOT(setBreathCNInterval(int)));
        if(pt_map)
        {
            connect(dialog, SIGNAL(intervalValueUpdated(int)), pt_map, SIGNAL(setEstimationInterval(int)));
        }
        dialog->show();

    } else {

        QMessageBox msg(QMessageBox::Information, tr("Warning"),tr("Start new session before!") );
        msg.exec();

    }
}

//------------------------------------------------------------------------------------

void MainWindow::startMeasurementsRecord()
{


    if(m_measurementsFile.isOpen())
    {
        m_measurementsFile.close();
        pt_measRecAct->setChecked(false);
        disconnect(pt_harmonicProcessor, SIGNAL(measurementsUpdated(qreal,qreal,qreal,qreal)), this, SLOT(updateMeasurementsRecord(qreal,qreal,qreal,qreal)));
        QMessageBox msgBox(QMessageBox::Question, this->windowTitle(), tr("Another record?"), QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::No)
        {
            return;
        }
    }

    m_measurementsFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save measurements into a file"),"Records/ID" + QString::number(m_sessionsCounter)+ "_meas.txt", "Text file (*.txt)"));

    while(!m_measurementsFile.open(QIODevice::WriteOnly))   {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not save file, try another name"), QMessageBox::Save | QMessageBox::Cancel, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::Save) {
            m_measurementsFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to a file"),"Records/ID" + QString::number(m_sessionsCounter)+ "_meas.txt", tr("Text file (*.txt)")));
        }
        else {
            pt_measRecAct->setChecked(false);
            break;
        }
    }

    if(m_measurementsFile.isOpen()) {
        pt_measRecAct->setChecked(true);
        m_measurementsStream.setDevice(&m_measurementsFile);
        m_measurementsStream.setRealNumberNotation(QTextStream::FixedNotation);
        m_measurementsStream.setRealNumberPrecision(3);
        m_measurementsStream << "QPULSECAPTURE MEASUREMENTS RECORD of " << QDateTime::currentDateTime().toString("dd.MM.yyyy")
                             << "\nhh:mm:ss\tHeartRate, bpm\tSNR, dB\tBreathRate, rpm\tSNR, dB\n";
        connect(pt_harmonicProcessor, SIGNAL(measurementsUpdated(qreal,qreal,qreal,qreal)), this, SLOT(updateMeasurementsRecord(qreal,qreal,qreal,qreal)));
    }
}

void MainWindow::updateMeasurementsRecord(qreal heartRate, qreal heartSNR, qreal breathRate, qreal breathSNR)
{
    if(m_measurementsFile.isOpen())
    {
        m_measurementsStream << QTime::currentTime().toString("hh:mm:ss")
                             << "\t" << qRound(heartRate) << "\t"
                             << heartSNR << "\t" << qRound(breathRate)
                             << "\t" << breathSNR << "\n";
    }
}

void MainWindow::updateStatus(qreal value)
{

}


