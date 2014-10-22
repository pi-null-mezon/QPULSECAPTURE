#include <QDateTime>
#include "mainwindow.h"
//------------------------------------------------------------------------------------

#define FRAME_WIDTH 480
#define FRAME_HEIGHT 320
#define FRAME_MARGIN 3

//------------------------------------------------------------------------------------
const char * MainWindow::QPlotDialogName[]=
{
    "Signal vs frame",
    "Amplitude spectrum",
    "Frame time vs frame",
    "PCA 1-st projection",
    "Filter output vs frame",
    "Signal phase diagram",
    "Breath curve vs frameSet"
};
//------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    setWindowTitle(APP_NAME);
    setMinimumSize(FRAME_WIDTH, FRAME_HEIGHT);

    pt_centralWidget = new QBackgroundWidget(NULL, palette().color(backgroundRole()), QColor(120,120,120), Qt::Dense6Pattern);
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

    //--------------------QVideoCapture------------------------------
    pt_videoCapture = new QVideoCapture(this);

    //----------Register openCV types in Qt meta-type system---------
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<cv::Rect>("cv::Rect");

    //----------------------Connections------------------------------
    connect(pt_opencvProcessor, SIGNAL(frame_was_processed(cv::Mat,double)), pt_display, SLOT(updateImage(cv::Mat,double)));
    connect(pt_display, SIGNAL(rect_was_entered(cv::Rect)), pt_opencvProcessor, SLOT(setRect(cv::Rect)));
    connect(pt_opencvProcessor, SIGNAL(no_regions_selected(const char*)), pt_display, SLOT(set_warning_status(const char*)));
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

bool MainWindow::openvideofile(const QString & videofileName)
{
    if ( !videofileName.isNull() )
    {
        m_timer.stop();
        if ( pt_videoCapture->openfile(videofileName) )
        {
            if ( pt_infoLabel ) // just remove label
            {
                pt_mainLayout->removeWidget(pt_infoLabel);
                delete pt_infoLabel;
                pt_infoLabel = NULL;
            }
            return true;
        }
        else
        {
            QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open video file!"), QMessageBox::Ok, this, Qt::Dialog);
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("No file has been chosen!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    return false;
}

//------------------------------------------------------------------------------------

bool MainWindow::opendevice()
{
    m_timer.stop();
    pt_videoCapture->open_deviceSelectDialog();

    if ( pt_videoCapture->opendevice(DEFAULT_FRAME_PERIOD) )
    {
        if ( pt_infoLabel ) // just remove label
        {
            pt_mainLayout->removeWidget(pt_infoLabel);
            delete pt_infoLabel;
            pt_infoLabel = NULL;
        }
        return true;
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open video capture device!"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
    return false;
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
    pt_opencvProcessor->update_timeCounter();
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
        pt_display->clearStrings();
        if(pt_harmonicProcessor)
        {
            pt_harmonicThread->quit();
            pt_harmonicThread->wait();
        }
        //------------------Close all opened plot dialogs---------------------
        for(qint8 i = m_dialogSetCounter; i > 0; i--)
        {
            pt_dialogSet[ i-1 ]->close(); // there is no need to explicitly decrement m_dialogSetCounter value because pt_dialogSet[i] was preset to Qt::WA_DeleteOnClose flag and on_destroy of pt_dialogSet[i] 'this' will decrease counter automatically
        };
        //---------------------Harmonic processor------------------------
        pt_harmonicThread = new QThread(this);
        pt_harmonicProcessor = new QHarmonicProcessor(NULL, dialog.get_datalength(), dialog.get_bufferlength());
        pt_harmonicProcessor->moveToThread(pt_harmonicThread);
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicProcessor, SLOT(deleteLater()));
        connect(pt_harmonicThread, SIGNAL(finished()),pt_harmonicThread, SLOT(deleteLater()));
        //---------------------------------------------------------------
        if(dialog.get_flagRecord())
        {
            if(m_saveFile.isOpen())
            {
                m_saveFile.close();
            }
            m_saveFile.setFileName(dialog.get_stringRecord());
            if(m_saveFile.open(QIODevice::WriteOnly))
            {
                m_textStream.setDevice(&m_saveFile);
            }
            m_textStream << "dd.MM.yyyy hh:mm:ss.zzz \t CNSignal \t MeanRed \t MeanGreen \t MeanBlue \t PulseRate, bpm \t SNR, dB\n";
            connect(pt_harmonicProcessor, SIGNAL(SignalActualValues(qreal,qreal,qreal,qreal,qreal,qreal)), this, SLOT(make_record_to_file(qreal,qreal,qreal,qreal,qreal,qreal)));
        }
        //---------------------------------------------------------------
        if(dialog.get_flagColor())
        {
            connect(pt_opencvProcessor, SIGNAL(colors_were_evaluated(ulong,ulong,ulong,ulong,double)), pt_harmonicProcessor, SLOT(WriteToDataRGB(ulong,ulong,ulong,ulong,double)));
        }
        else
        {
            connect(pt_opencvProcessor,SIGNAL(colors_were_evaluated(ulong,ulong,ulong,ulong,double)),pt_harmonicProcessor,SLOT(WriteToDataOneColor(ulong,ulong,ulong,ulong,double)));
        }
        //---------------------------------------------------------------
        connect(pt_harmonicProcessor, SIGNAL(frequencyOutOfRange()), pt_display, SLOT(clearStrings()));
        //---------------------------------------------------------------
        disconnect(pt_videoCapture,0,0,0);
        if(dialog.get_flagCascade())
        {
            QString filename = dialog.get_stringCascade();
            while(!pt_opencvProcessor->load_cascadecalssifier_file(filename.toStdString()))
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
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(pulse_processing_with_classifier(cv::Mat)), Qt::BlockingQueuedConnection);
        }
        else
        {
            connect(pt_videoCapture, SIGNAL(frame_was_captured(cv::Mat)), pt_opencvProcessor, SLOT(pulse_processing_custom_region(cv::Mat)), Qt::BlockingQueuedConnection);
        }
        //--------------------------------------------------------------
        pt_harmonicProcessor->set_PCA_flag(dialog.get_flagPCA());
        //--------------------------------------------------------------
        connect(pt_harmonicProcessor, SIGNAL(CNSignalWasUpdated(const qreal*,quint16)), pt_display, SLOT(updatePointer(const qreal*,quint16)));
        if(dialog.get_FFTflag())
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(ComputeFrequency()));
        }
        else
        {
            connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor, SLOT(CountFrequency()));
        }
        connect(pt_harmonicProcessor, SIGNAL(HRfrequencyWasUpdated(qreal,qreal,bool)), pt_display, SLOT(updateValues(qreal,qreal,bool)));
        //--------------------------------------------------------------
        pt_harmonicThread->start();
        pt_optionsMenu->setEnabled(true);
        if(dialog.get_flagVideoFile())
        {
            this->openvideofile(dialog.get_stringVideoFile());
        }
        else
        {
            this->opendevice();
        }
        m_timer.setInterval( dialog.get_timerValue() );
        this->onresume();
        this->statusBar()->showMessage(tr("Plot options available through Menu->Options->New plot"));
    }
    else
    {
        this->statusBar()->showMessage(tr("The new session was rejected, you can continue previous session by resume option"));
    }
}

//------------------------------------------------------------------------------------------

void MainWindow::createPlotDialog()
{
    if(m_dialogSetCounter < LIMIT_OF_DIALOGS_NUMBER)
    {
        QDialog dialog;
        QVBoxLayout centralLayout;
        centralLayout.setMargin(FRAME_MARGIN);
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
                        connect(pt_harmonicProcessor, SIGNAL(CNSignalWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Centered & normalized signal");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        break;
                    case 1: // Spectrum trace
                        connect(pt_harmonicProcessor, SIGNAL(SpectrumWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Freq.count","DFT amplitude spectrum");
                        pt_plot->set_vertical_Borders(0.0,25000.0);
                        pt_plot->set_coordinatesPrecision(0,1);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        break;
                    case 2: // Time trace
                        connect(pt_harmonicProcessor, SIGNAL(ptTimeWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","processing period per frame, ms");
                        pt_plot->set_vertical_Borders(0.0,100.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                        pt_plot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
                        break;
                    case 3: // PCA 1st projection trace
                        connect(pt_harmonicProcessor, SIGNAL(PCAProjectionWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Normalised & centered projection on 1-st PCA direction");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 4: // Digital filter output
                        connect(pt_harmonicProcessor, SIGNAL(pt_YoutputWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Frame","Digital derivative after smoothing");
                        pt_plot->set_vertical_Borders(-2.0,2.0);
                        pt_plot->set_coordinatesPrecision(0,2);
                    break;
                    case 5: // signal phase shift
                        connect(pt_harmonicProcessor, SIGNAL(CNSignalWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_DrawRegime(QEasyPlot::PhaseRegime);
                        pt_plot->set_axis_names("Signal count","Signal count");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_horizontal_Borders(-5.0, 5.0);
                        pt_plot->set_X_Ticks(11);
                        pt_plot->set_coordinatesPrecision(2,2);
                    break;
                    case 6: // breath curve
                        connect(pt_harmonicProcessor, SIGNAL(SlowPPGWasUpdated(const qreal*,quint16)), pt_plot, SLOT(set_externalArray(const qreal*,quint16)));
                        pt_plot->set_axis_names("Set of frames","SlowPPG");
                        pt_plot->set_vertical_Borders(-5.0,5.0);
                        pt_plot->set_X_Ticks(11);
                        pt_plot->set_coordinatesPrecision(0,2);
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
    for(qint8 i = m_dialogSetCounter; i > 0; i--)
    {
        pt_dialogSet[ i-1 ]->close(); // there is no need to explicitly decrement m_dialogSetCounter value because pt_dialogSet[i] was preset to Qt::WA_DeleteOnClose flag and on_destroy of pt_dialogSet[i] 'this' will decrease counter automatically
    };
}

