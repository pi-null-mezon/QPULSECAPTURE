#include <QDateTime>
#include "mainwindow.h"
//------------------------------------------------------------------------------------

#define FRAME_WIDTH 480
#define FRAME_HEIGHT 320
#define FRAME_MARGIN 5

//------------------------------------------------------------------------------------
const char * MainWindow::QPlotDialogName[]=
{
    "Signal vs frame",
    "Amplitude spectrum",
    "Frame time vs frame",
    "PCA 1-st projection",
    "Filter output vs frame",
    "Signal phase diagram"
};
//------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    setWindowTitle(APP_NAME);
    setMinimumSize(FRAME_WIDTH, FRAME_HEIGHT);

    pt_centralWidget = new QBackgroundWidget(NULL, palette().color(backgroundRole()));
    pt_centralWidgetLayout = new QVBoxLayout();
    this->setCentralWidget(pt_centralWidget);
    pt_centralWidget->setLayout(pt_centralWidgetLayout);
    pt_centralWidgetLayout->setMargin(0);

    //--------------------------------------------------------------
    pt_display = new QImageWidget(); // Widgets without a parent are “top level” (independent) widgets. All other widgets are drawn within their parent
    pt_mainLayout = new QVBoxLayout();
    pt_display->setLayout(pt_mainLayout);
    pt_centralWidgetLayout->addWidget(pt_display);

    //--------------------------------------------------------------
    pt_infoLabel = new QLabel(tr("<i>Choose a menu option, or make right-click to invoke a context menu</i>"));
    pt_infoLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    pt_infoLabel->setAlignment(Qt::AlignCenter);
    pt_infoLabel->setWordWrap( true );
    pt_infoLabel->setFont( QFont("MS Shell Dlg 2", 14, QFont::Normal) );
    pt_mainLayout->addWidget(pt_infoLabel);

    //--------------------------------------------------------------
    createActions();
    createMenus();
    createThreads();

    //--------------------------------------------------------------
    m_dialogSetCounter = 0;

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
    pt_exitAct->setStatusTip(tr("Application exit"));
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
    connect(pt_fastVisualizationAct, SIGNAL(triggered(bool)),pt_display, SLOT(toggle_advancedvisualization(bool)));

    pt_changeColorsAct = new QAction(tr("Co&lor"), this);
    pt_changeColorsAct->setStatusTip(tr("Switch between black or white color of text on the screen"));
    connect(pt_changeColorsAct, SIGNAL(triggered()), pt_display, SLOT(switchColorScheme()));

    pt_openPlotDialog = new QAction(tr("&New plot"), this);
    pt_openPlotDialog->setStatusTip(tr("Create a new window for the visualization of appropriate process"));
    connect(pt_openPlotDialog, SIGNAL(triggered()), this, SLOT(createPlotDialog()));

    pt_recordAct = new QAction(tr("&Record"), this);
    pt_recordAct->setStatusTip(tr("Start to record current measurement ssession in to output text file"));
    connect(pt_recordAct, SIGNAL(triggered()), this, SLOT(startRecord()));
    pt_recordAct->setCheckable(true);

    pt_colorActGroup = new QActionGroup(this);
    pt_colorMapper = new QSignalMapper(this);
    pt_redAct = new QAction(tr("Red"), pt_colorActGroup);
    pt_redAct->setStatusTip(tr("Enroll only red channel"));
    pt_redAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_redAct,QHarmonicProcessor::Red);
    connect(pt_redAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_blueAct = new QAction(tr("Blue"), pt_colorActGroup);
    pt_blueAct->setStatusTip(tr("Enroll only blue channel"));
    pt_blueAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_blueAct,QHarmonicProcessor::Blue);
    connect(pt_blueAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_greenAct = new QAction(tr("Green"), pt_colorActGroup);
    pt_greenAct->setStatusTip(tr("Enroll only green channel"));
    pt_greenAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_greenAct,QHarmonicProcessor::Green);
    connect(pt_greenAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    pt_allAct = new QAction(tr("RGB"), pt_colorActGroup);
    pt_allAct->setStatusTip(tr("Enroll all channels"));
    pt_allAct->setCheckable(true);
    pt_colorMapper->setMapping(pt_allAct,QHarmonicProcessor::All);
    connect(pt_allAct, SIGNAL(triggered()), pt_colorMapper, SLOT(map()));
    connect(pt_colorMapper,SIGNAL(mapped(int)), this, SLOT(SwitchColorMode(int)));
    pt_greenAct->setChecked(true);

    pt_pcaAct = new QAction(tr("PCA align"), this);
    pt_pcaAct->setStatusTip(tr("Control PCA alignment, affects on result only in harmonic analysis mode"));
    pt_pcaAct->setCheckable(true);
    connect(pt_pcaAct, SIGNAL(triggered(bool)), this, SLOT(SwitchPCA(bool)));

    pt_mapAct = new QAction(tr("Mapping"), this);
    pt_mapAct->setStatusTip(tr("Start to mapping pulse signal on image"));
    pt_mapAct->setCheckable(true);
    connect(pt_mapAct, SIGNAL(triggered()), this, SLOT(openMapDialog()));
}

//------------------------------------------------------------------------------------

void MainWindow::createMenus()
{
    pt_fileMenu = this->menuBar()->addMenu(tr("&File"));
    pt_fileMenu->addAction(pt_openSessionAct);
    pt_fileMenu->addSeparator();
    pt_fileMenu->addAction(pt_exitAct);

    //------------------------------------------------
    pt_optionsMenu = menuBar()->addMenu(tr("&Options"));
    pt_optionsMenu->addAction(pt_openPlotDialog);
    pt_optionsMenu->addAction(pt_recordAct);
    pt_optionsMenu->addSeparator();
    pt_modeMenu = pt_optionsMenu->addMenu(tr("&Mode"));
    pt_modeMenu->addActions(pt_colorActGroup->actions());
    pt_modeMenu->addSeparator();
    pt_modeMenu->addAction(pt_pcaAct);
    pt_modeMenu->addSeparator();
    pt_modeMenu->addAction(pt_mapAct);
    pt_optionsMenu->addSeparator();
    pt_optionsMenu->addAction(pt_fastVisualizationAct);
    pt_optionsMenu->addAction(pt_changeColorsAct);
    pt_optionsMenu->setEnabled(false);

    //-------------------------------------------------
    pt_deviceMenu = menuBar()->addMenu(tr("&Video"));
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
    connect(pt_improcThread, &QThread::finished, pt_opencvProcessor, &QObject::deleteLater);
    //---------------------------------------------------------------

    pt_harmonicProcessor = NULL;
    pt_harmonicThread = NULL;
    pt_map = NULL;

    //--------------------QVideoCapture------------------------------
    pt_videoCapture = new QVideoCapture(this);

    //----------Register openCV types in Qt meta-type system---------
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<cv::Rect>("cv::Rect");

    //----------------------Connections------------------------------
    connect(pt_opencvProcessor, SIGNAL(frameProcessed(cv::Mat,double)), pt_display, SLOT(updateImage(cv::Mat,double)));
    connect(pt_display, SIGNAL(rect_was_entered(cv::Rect)), pt_opencvProcessor, SLOT(setRect(cv::Rect)));
    connect(pt_opencvProcessor, SIGNAL(selectRegion(const char*)), pt_display, SLOT(set_warning_status(const char*)));
    connect(pt_opencvProcessor, SIGNAL(mapRegionUpdated(cv::Rect)), pt_display, SLOT(updadeMapRegion(cv::Rect)));
    //----------------------Thread start-----------------------------
    pt_improcThread->start();
}

//------------------------------------------------------------------------------------

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(pt_openSessionAct);
    menu.addSeparator();
    menu.addAction(pt_pauseAct);
    menu.addAction(pt_resumeAct);
    menu.exec(event->globalPos());
}

//------------------------------------------------------------------------------------

bool MainWindow::openvideofile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open video file"), "/video", tr("Video (*.avi *.mp4 *.wmv)"));
    while( !pt_videoCapture->openfile(fileName) ) {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open video file!"), QMessageBox::Open | QMessageBox::Cancel, this, Qt::Dialog);
        if( msgBox.exec() == QMessageBox::Open )
        {
            fileName = QFileDialog::getOpenFileName(this, tr("Open video file"), "/video", tr("Video (*.avi *.mp4 *.wmv)"));
        } else {
            return false;
        }
    }
    if ( pt_infoLabel ) {
        pt_mainLayout->removeWidget(pt_infoLabel);
        delete pt_infoLabel;
        pt_infoLabel = NULL;
    }
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
    return true;
}

//------------------------------------------------------------------------------------

void MainWindow::show_about()
{
   QDialog *aboutdialog = new QDialog();
   aboutdialog->setWindowTitle("About dialog");
   aboutdialog->setFixedSize(256,128);

   QVBoxLayout *templayout = new QVBoxLayout();
   templayout->setMargin(5);

   QLabel *projectname = new QLabel( QString(APP_NAME) + " " + QString(APP_VERSION) );
   projectname->setFrameStyle(QFrame::Box | QFrame::Raised);
   projectname->setAlignment(Qt::AlignCenter);
   QLabel *projectauthors = new QLabel( QString(APP_AUTHOR) + "\n\n" + QString(APP_COMPANY) + "\n\n" + QString(APP_RELEASE_DATE) );
   projectauthors->setWordWrap(true);
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
    if (!QDesktopServices::openUrl(QUrl("file:///" + QDir::currentPath() + "/QVideoProcessing.txt", QUrl::TolerantMode))) // runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open help file"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

//------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    if(m_saveFile.isOpen())
    {
        m_saveFile.close();
    }

    pt_videoCapture->close();

    pt_improcThread->quit();
    pt_improcThread->wait();

    if(pt_harmonicThread)
    {
        pt_harmonicThread->quit();
        pt_harmonicThread->wait();
    }
}

//------------------------------------------------------------------------------------

void MainWindow::onpause()
{
    pt_videoCapture->pause();
    m_timer.stop();
}

//------------------------------------------------------------------------------------

void MainWindow::onresume()
{
    pt_videoCapture->resume();
    m_timer.start();
    pt_opencvProcessor->updateTime();
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
    if (!QProcess::startDetached(QString("WVCF_utility.exe"),QStringList("-l")))// runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open utility!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    } 
}

//----------------------------------------------------------------------------------------

void MainWindow::configure_and_start_session()
{
    this->onpause();
    QSettingsDialog dialog;
    if(dialog.exec() == QDialog::Accepted)
    {     
        if(pt_harmonicProcessor)
        {
            pt_harmonicThread->quit();
            pt_harmonicThread->wait();
        }
        //------------------Close all opened plot dialogs---------------------
        closeAllDialogs();
        //---------------------Harmonic processor------------------------
        pt_harmonicThread = new QThread(this);
        pt_harmonicProcessor = new QHarmonicProcessor(NULL, dialog.get_datalength(), dialog.get_bufferlength());
        pt_harmonicProcessor->moveToThread(pt_harmonicThread);
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicProcessor, SLOT(deleteLater()));
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicThread, SLOT(deleteLater()));
        //---------------------------------------------------------------
        if(m_saveFile.isOpen()) {
            m_saveFile.close();
            pt_recordAct->setChecked(false);
        }
        //---------------------------------------------------------------
        if(dialog.get_customPatientFlag())
        {
            if(pt_harmonicProcessor->loadWarningRates(dialog.get_stringDistribution().toLocal8Bit().constData(),(QHarmonicProcessor::SexID)dialog.get_patientSex(),dialog.get_patientAge(),(QHarmonicProcessor::TwoSideAlpha)dialog.get_patientPercentile()) == QHarmonicProcessor::FileExistanceError)
            {
                QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open population distribution file"), QMessageBox::Ok, this, Qt::Dialog);
                msgBox.exec();
            }
        }
        //---------------------------------------------------------------
        disconnect(pt_videoCapture,0,0,0);
        if(dialog.get_flagCascade())
        {
            QString filename = dialog.get_stringCascade();
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
        connect(pt_opencvProcessor, SIGNAL(dataCollected(ulong,ulong,ulong,ulong,double)), pt_harmonicProcessor, SLOT(EnrollData(ulong,ulong,ulong,ulong,double)));
        //connect(pt_harmonicProcessor, SIGNAL(SignalUpdated(const qreal*,quint16)), pt_display, SLOT(updatePointer(const qreal*,quint16)));
        connect(pt_harmonicProcessor, SIGNAL(TooNoisy(qreal)), pt_display, SLOT(clearFrequencyString(qreal)));
        //--------------------------------------------------------------      
        if(dialog.get_FFTflag())
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(ComputeFrequency()));
        }
        else
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(CountFrequency()));
        }
        connect(pt_harmonicProcessor, SIGNAL(HeartRateUpdated(qreal,qreal,bool)), pt_display, SLOT(updateValues(qreal,qreal,bool)));
        //--------------------------------------------------------------
        pt_harmonicThread->start();
        pt_optionsMenu->setEnabled(true);

        if(dialog.get_flagVideoFile())
            this->openvideofile();
        else
            this->opendevice();

        m_timer.setInterval( dialog.get_timerValue() );
        this->statusBar()->showMessage(tr("Plot options available through Menu->Options->New plot"));
    }
    this->onresume();
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
        QPushButton acceptButton("Accept");
        QPushButton rejectButton("Cancel");
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
            dialogTypeComboBox.addItem( QPlotDialogName[i] );
        }
        dialogTypeComboBox.setCurrentIndex(0);

        dialog.setWindowTitle("Plot select dialog");
        dialog.setMinimumSize(256,128);

        if(dialog.exec() == QDialog::Accepted)
        {
            pt_dialogSet[ m_dialogSetCounter ] = new QDialog(NULL, Qt::Window);
            pt_dialogSet[ m_dialogSetCounter ]->setWindowTitle(dialogTypeComboBox.currentText() + " plot");
            pt_dialogSet[ m_dialogSetCounter ]->setAttribute(Qt::WA_DeleteOnClose, true);
            connect(pt_dialogSet[ m_dialogSetCounter ], SIGNAL(destroyed()), this, SLOT(decrease_dialogSetCounter()));
            pt_dialogSet[ m_dialogSetCounter ]->setMinimumSize(FRAME_WIDTH, FRAME_HEIGHT);

            QVBoxLayout *pt_layout = new QVBoxLayout( pt_dialogSet[ m_dialogSetCounter ] );
            pt_layout->setMargin(FRAME_MARGIN);
            QEasyPlot *pt_plot = new QEasyPlot( pt_dialogSet[ m_dialogSetCounter ] );
            pt_layout->addWidget( pt_plot );          
                switch(dialogTypeComboBox.currentIndex())
                {
                    case 0: // Signal trace
                        connect(pt_harmonicProcessor, SIGNAL(SignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Centered & normalized signal");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        break;
                    case 1: // Spectrum trace
                        connect(pt_harmonicProcessor, SIGNAL(SpectrumUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Freq.count","DFT amplitude spectrum");
                        pt_plot->set_vertical_Borders(0.0,25000.0);
                        pt_plot->set_coordinatesPrecision(0,1);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        break;
                    case 2: // Time trace
                        connect(pt_harmonicProcessor, SIGNAL(TimeUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","processing period per frame, ms");
                        pt_plot->set_vertical_Borders(0.0,100.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        break;
                    case 3: // PCA 1st projection trace
                        connect(pt_harmonicProcessor, SIGNAL(PCAProjectionUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Normalised & centered projection on 1-st PCA direction");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 4: // Digital filter output
                        connect(pt_harmonicProcessor, SIGNAL(BinaryOutputUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Digital derivative after smoothing");
                        pt_plot->set_vertical_Borders(-2.0,2.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 5: // signal phase shift
                        connect(pt_harmonicProcessor, SIGNAL(SignalUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_DrawRegime(QEasyPlot::PhaseRegime);
                        pt_plot->set_axis_names("Signal count","Signal count");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_horizontal_Borders(-5.0, 5.0);
                        pt_plot->set_X_Ticks(11);
                        pt_plot->set_coordinatesPrecision(2,2);
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
    qWarning("Plot dialogs left: %d", LIMIT_OF_DIALOGS_NUMBER - m_dialogSetCounter);
}

//-------------------------------------------------------------------------------------------

void MainWindow::make_record_to_file(qreal signalValue, qreal meanRed, qreal meanGreen, qreal meanBlue, qreal freqValue, qreal snrValue)
{
    if(m_saveFile.isOpen())
    {
        m_textStream << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")
                     << "\t" << signalValue << "\t" << meanRed << "\t" << meanGreen
                     << "\t" << meanBlue << "\t" << freqValue << "\t" << snrValue << "\n";
    }
}

//-------------------------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent*)
{
    closeAllDialogs();
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
    if(m_saveFile.isOpen()) {
        m_saveFile.close();
        pt_recordAct->setChecked(false);
    }

    m_saveFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to file"),"/records", "Text file (*.txt)"));

    while(!m_saveFile.open(QIODevice::WriteOnly))   {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not save file with such name, try another name"), QMessageBox::Save | QMessageBox::Cancel, this, Qt::Dialog);
        if(msgBox.exec() == QMessageBox::Save) {
            m_saveFile.setFileName(QFileDialog::getSaveFileName(this,tr("Save record to a file"),"/Records/record.txt", tr("Text file (*.txt)")));
        }
        else {
            pt_recordAct->setChecked(false);
            break;
        }
    }

    if(m_saveFile.isOpen()) {
        pt_recordAct->setChecked(true);
        m_textStream.setDevice(&m_saveFile);
        m_textStream << "dd.MM.yyyy hh:mm:ss.zzz\tCNSignal\tMeanRed\tMeanGreen\tMeanBlue\tPulseRate,bpm\tSNR,dB\n";
        connect(pt_harmonicProcessor, SIGNAL(CurrentValues(qreal,qreal,qreal,qreal,qreal,qreal)), this, SLOT(make_record_to_file(qreal,qreal,qreal,qreal,qreal,qreal)));
    }
}

//----------------------------------------------------------------------------------------------

void MainWindow::SwitchColorMode(int value)
{
    if(pt_harmonicProcessor)
    {
        pt_harmonicProcessor->switchColorMode((QHarmonicProcessor::ColorChannel)value);
    }
}

//----------------------------------------------------------------------------------------------

void MainWindow::SwitchPCA(bool value)
{
    if(pt_harmonicProcessor)
    {
        pt_harmonicProcessor->setPCAMode(value);
    }
}

//----------------------------------------------------------------------------------------------

void MainWindow::openMapDialog()
{
    cv::Rect tempRect = pt_opencvProcessor->getRect();
    if((tempRect.width == 0) || (tempRect.height == 0))
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Select region on image first"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
        pt_mapAct->setChecked(false);
        return;
    }
    else
    {
        if(!pt_mapAct->isChecked())
        {
            QMessageBox msgBox(QMessageBox::Question, this->windowTitle(), tr("Do You want another mapping?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, this, Qt::Dialog);
            int resultCode = msgBox.exec();
            if(resultCode == QMessageBox::No)
            {
                if(pt_map)
                {
                    pt_display->updateMap(NULL,0,0,0.0,0.0);
                    delete pt_map;
                    pt_map = NULL;
                }
                pt_mapAct->setChecked(false);
                return;
            }
            if(resultCode == QMessageBox::Cancel)
            {
                pt_mapAct->setChecked(true);
                return;
            }
        }

        if(pt_map)
        {
            pt_display->updateMap(NULL,0,0,0.0,0.0);
            delete pt_map;
            pt_map = NULL;        
        }

        mappingdialog dialog;
        dialog.setImageHeight(tempRect.height);
        dialog.setImageWidth(tempRect.width);

        if(dialog.exec() == QDialog::Accepted)
        {
            pt_opencvProcessor->setMapRegion(tempRect);
            pt_opencvProcessor->setMapCellSize(dialog.getCellSize(), dialog.getCellSize());

            pt_map = new QHarmonicProcessorMap(this, dialog.getMapWidth(), dialog.getMapHeight());
            connect(pt_opencvProcessor, SIGNAL(mapCellProcessed(ulong,ulong,ulong,ulong,double)), pt_map, SLOT(updateHarmonicProcessor(ulong,ulong,ulong,ulong,double)));
            connect(&m_timer, SIGNAL(timeout()), pt_map, SIGNAL(updateMap()));
            connect(pt_map, SIGNAL(mapUpdated(const qreal*,quint32,quint32,qreal,qreal)), pt_display, SLOT(updateMap(const qreal*,quint32,quint32,qreal,qreal)));
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(mapProcess(cv::Mat)));

            pt_mapAct->setChecked(true);
        }
        else
        {
            pt_mapAct->setChecked(false);
        }
    }
}

