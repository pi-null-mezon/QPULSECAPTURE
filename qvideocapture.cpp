#include "qvideocapture.h"

QVideoCapture::QVideoCapture(QObject *parent) :
    QObject(parent)
{

    ptframetimer = new QTimer();
    ptframetimer->setTimerType(Qt::PreciseTimer);
    connect(ptframetimer, SIGNAL( timeout() ), this, SLOT( read_frame() )); // makes a connection between timer signal and class slot
}

bool QVideoCapture::openfile(const QString &filename)
{
    ptframetimer->stop();
    if( cvcapture.open( filename.toLocal8Bit().constData() ) )
    {
        device_flag = false;
        ptframetimer->setInterval( (int)1000/cvcapture.get(CV_CAP_PROP_FPS) ); // CV_CAP_PROP_FPS - Frame rate
        return true;
    }
    return false;
}

bool QVideoCapture::opendevice(int device_id, int period) // period should be entered in ms
{
    ptframetimer->stop();
    if( cvcapture.open( device_id ) )
    {
        device_flag = true;
        ptframetimer->setInterval( period );
        return true;
    }
    return false;
}

bool QVideoCapture::set(int propertyID, double value)
{
    return cvcapture.set( propertyID, value);
}

bool QVideoCapture::start()
{
    if( cvcapture.isOpened() )
    {
        ptframetimer->start();
        return true;
    }
    return false;
}

bool QVideoCapture::close()
{
    if( cvcapture.isOpened() )
    {
        ptframetimer->stop();
        cvcapture.release();

        return true;
    }
    return false;
}

bool QVideoCapture::pause()
{
    if( cvcapture.isOpened() )
    {
        ptframetimer->stop();
        return true;
    }
    return false;
}

bool QVideoCapture::read_frame()
{
    //there is no need to cvcapture.isOpen check because a class interface guarantees that timer could only was launched after cvcapture has been opened
    if( ( cvcapture.read(frame) ) && ( !frame.empty() ) )
    {
        emit frame_was_captured(frame);
        return true;
    }
    else
    {
        pause();
        return false;
    }
}

bool QVideoCapture::open_resolution_dialog()
{
    if (cvcapture.isOpened() && device_flag)
    {
    //GUI CONSTRUCTION//
    QDialog dialog;
    dialog.setFixedSize(256,256);
    dialog.setWindowTitle("Camcorder resolution");

    QVBoxLayout toplevellayout;
    toplevellayout.setMargin(5);

    QVBoxLayout layout;
    layout.setMargin(5);
        QGroupBox groupbox;
        groupbox.setTitle("Resolution/framerate");
            QVBoxLayout comboboxes;
            comboboxes.setMargin(5);
                QComboBox CBresolution;
                CBresolution.addItem("640 x 480"); // 0
                CBresolution.addItem("800 x 600"); // 1
                CBresolution.addItem("1024 x 768"); // 2
                CBresolution.addItem("1280 x 960"); // 3
                CBresolution.addItem("1600 x 1200"); // 4
                CBresolution.setSizeAdjustPolicy(QComboBox::AdjustToContents);
                CBresolution.setCurrentIndex(0);
                QLabel Lresolution("Set resolution:");
                QComboBox CBframerate(&dialog);
                CBframerate.addItem("30 fps"); // 0
                CBframerate.addItem("25 fps"); // 1
                CBframerate.addItem("20 fps"); // 2
                CBframerate.addItem("15 fps"); // 3
                CBframerate.addItem("10 fps"); // 4
                CBframerate.setSizeAdjustPolicy(QComboBox::AdjustToContents);
                CBframerate.setCurrentIndex(0);
                QLabel Lframerate("Set framerate:");
            comboboxes.addWidget(&Lresolution,1,Qt::AlignLeft);
            comboboxes.addWidget(&CBresolution);
            comboboxes.addWidget(&Lframerate,1,Qt::AlignLeft);
            comboboxes.addWidget(&CBframerate);            
        groupbox.setLayout(&comboboxes);
    layout.addWidget(&groupbox);

    QHBoxLayout buttons;
    buttons.setMargin(5);
        QPushButton Baccept;
        Baccept.setText("Accept");
        connect(&Baccept, SIGNAL(clicked()), &dialog, SLOT(accept()));
        QPushButton Bcancel;
        Bcancel.setText("Cancel");
        connect(&Bcancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
    buttons.addWidget(&Baccept);
    buttons.addWidget(&Bcancel);

    toplevellayout.addLayout(&layout);
    toplevellayout.addLayout(&buttons);
    dialog.setLayout(&toplevellayout);
    //GUI CONSTRUCTION END//

        if(dialog.exec() == QDialog::Accepted)
        {
            cvcapture.set(CV_CAP_PROP_FRAME_WIDTH, CBresolution.itemText(CBresolution.currentIndex()).section(" x ",0,0).toDouble());
            cvcapture.set(CV_CAP_PROP_FRAME_HEIGHT, CBresolution.itemText(CBresolution.currentIndex()).section(" x ",1,1).toDouble());
            ptframetimer->setInterval((int)1000/CBframerate.itemText(CBframerate.currentIndex()).section(" ",0,0).toDouble());
            return true;
        }
    }
    return false; // false if device was not opened or dialog.exec() == QDialog::Rejected
}

bool QVideoCapture::open_settings_dialog()
{
    if (cvcapture.isOpened() && device_flag)
    {
    //GUI CONSTRUCTION//
    QDialog dialog;
    dialog.setFixedSize(384,256);
    dialog.setWindowTitle("Camcorder settings");

    QVBoxLayout toplevellayout;
    toplevellayout.setMargin(5);
    QTabWidget tabwidget;

    QWidget page1; // a widget for a cam general settings selection
        QHBoxLayout centralbox;
        centralbox.setMargin(5);
            QVBoxLayout sliders;
            sliders.setMargin(5);
            QVBoxLayout lineedits;
            lineedits.setMargin(5);
            QVBoxLayout names;
            names.setMargin(5);

                QSlider Sbrightness;
                Sbrightness.setOrientation(Qt::Horizontal);
                Sbrightness.setMinimum(MIN_BRIGHTNESS);
                Sbrightness.setMaximum(MAX_BRIGHTNESS);
                Sbrightness.setValue( (int)cvcapture.get(CV_CAP_PROP_BRIGHTNESS) );
                QLabel Lbrightness;
                Lbrightness.setNum( (int)cvcapture.get(CV_CAP_PROP_BRIGHTNESS) );
                connect(&Sbrightness, SIGNAL(valueChanged(int)), &Lbrightness, SLOT(setNum(int)));
                connect(&Sbrightness, SIGNAL(valueChanged(int)), this, SLOT(set_brightness(int)));
                connect(this, SIGNAL(set_default_brightness(int)), &Sbrightness, SLOT(setValue(int)));
                QLabel Nbrightness("Brightness:");

                QSlider Scontrast;
                Scontrast.setOrientation(Qt::Horizontal);
                Scontrast.setMinimum(MIN_CONTRAST);
                Scontrast.setMaximum(MAX_CONTRAST);
                Scontrast.setValue( (int)cvcapture.get(CV_CAP_PROP_CONTRAST) );
                QLabel Lcontrast;
                Lcontrast.setNum( (int)cvcapture.get(CV_CAP_PROP_CONTRAST) );
                connect(&Scontrast, SIGNAL(valueChanged(int)), &Lcontrast, SLOT(setNum(int)));
                connect(&Scontrast,SIGNAL(valueChanged(int)), this, SLOT(set_contrast(int)));
                connect(this, SIGNAL(set_default_contrast(int)), &Scontrast, SLOT(setValue(int)));
                QLabel Ncontrast("Contrast:");

                QSlider Ssaturation;
                Ssaturation.setOrientation(Qt::Horizontal);
                Ssaturation.setMinimum(MIN_SATURATION);
                Ssaturation.setMaximum(MAX_SATURATION);
                Ssaturation.setValue( (int)cvcapture.get(CV_CAP_PROP_SATURATION) );
                QLabel Lsaturation;
                Lsaturation.setNum( (int)cvcapture.get(CV_CAP_PROP_SATURATION) );
                connect(&Ssaturation, SIGNAL(valueChanged(int)), &Lsaturation, SLOT(setNum(int)));
                connect(&Ssaturation,SIGNAL(valueChanged(int)), this, SLOT(set_saturation(int)));
                connect(this, SIGNAL(set_default_saturation(int)), &Ssaturation, SLOT(setValue(int)));
                QLabel Nsaturation("Saturation:");

                QSlider SwhitebalanceU;
                SwhitebalanceU.setOrientation(Qt::Horizontal);
                SwhitebalanceU.setMinimum(MIN_WHITE_BALANCE);
                SwhitebalanceU.setMaximum(MAX_WHITE_BALANCE);
                SwhitebalanceU.setValue( (int)cvcapture.get(CV_CAP_PROP_WHITE_BALANCE_BLUE_U) );
                QLabel LwhitebalanceU;
                LwhitebalanceU.setNum( (int)cvcapture.get(CV_CAP_PROP_WHITE_BALANCE_BLUE_U) );
                connect(&SwhitebalanceU, SIGNAL(valueChanged(int)), &LwhitebalanceU, SLOT(setNum(int)));
                connect(&SwhitebalanceU,SIGNAL(valueChanged(int)), this, SLOT(set_white_balanceU(int)));
                connect(this, SIGNAL(set_default_white_balanceU(int)), &SwhitebalanceU, SLOT(setValue(int)));
                QLabel NwhitebalanceU("White_balanceU:");

                QSlider SwhitebalanceV;
                SwhitebalanceV.setOrientation(Qt::Horizontal);
                SwhitebalanceV.setMinimum(MIN_WHITE_BALANCE);
                SwhitebalanceV.setMaximum(MAX_WHITE_BALANCE);
                SwhitebalanceV.setValue( (int)cvcapture.get(CV_CAP_PROP_WHITE_BALANCE_RED_V) );
                QLabel LwhitebalanceV;
                LwhitebalanceV.setNum( (int)cvcapture.get(CV_CAP_PROP_WHITE_BALANCE_RED_V) );
                connect(&SwhitebalanceV, SIGNAL(valueChanged(int)), &LwhitebalanceV, SLOT(setNum(int)));
                connect(&SwhitebalanceV,SIGNAL(valueChanged(int)), this, SLOT(set_white_balanceV(int)));
                connect(this, SIGNAL(set_default_white_balanceV(int)), &SwhitebalanceV, SLOT(setValue(int)));
                QLabel NwhitebalanceV("White_balanceV:");

            sliders.addWidget(&Sbrightness);
            lineedits.addWidget(&Lbrightness, 2);
            names.addWidget(&Nbrightness, 1);

            sliders.addWidget(&Scontrast);
            lineedits.addWidget(&Lcontrast, 2);
            names.addWidget(&Ncontrast, 1);

            sliders.addWidget(&Ssaturation);
            lineedits.addWidget(&Lsaturation, 2);
            names.addWidget(&Nsaturation, 1);

            sliders.addWidget(&SwhitebalanceU);
            lineedits.addWidget(&LwhitebalanceU, 2);
            names.addWidget(&NwhitebalanceU, 1);

            sliders.addWidget(&SwhitebalanceV);
            lineedits.addWidget(&LwhitebalanceV, 2);
            names.addWidget(&NwhitebalanceV, 1);

        centralbox.addLayout(&names);
        centralbox.addLayout(&sliders);
        centralbox.addLayout(&lineedits);
    page1.setLayout( &centralbox );

    QWidget page2; // a widget for a cam gain/exposure selection
    QHBoxLayout centralbox2;
    centralbox2.setMargin(5);
        QVBoxLayout sliders2;
        sliders2.setMargin(5);
        QVBoxLayout lineedits2;
        lineedits2.setMargin(5);
        QVBoxLayout names2;
        names2.setMargin(5);

            QSlider Sgain;
            Sgain.setOrientation(Qt::Horizontal);
            Sgain.setMinimum(MIN_GAIN);
            Sgain.setMaximum(MAX_GAIN);
            Sgain.setValue( (int)cvcapture.get(CV_CAP_PROP_GAIN) );
            QLabel Lgain;
            Lgain.setNum( (int)cvcapture.get(CV_CAP_PROP_GAIN) );
            connect(&Sgain, SIGNAL(valueChanged(int)), &Lgain, SLOT(setNum(int)));
            connect(&Sgain,SIGNAL(valueChanged(int)), this, SLOT(set_gain(int)));
            connect(this, SIGNAL(set_default_gain(int)), &Sgain, SLOT(setValue(int)),Qt::DirectConnection);
            QLabel Ngain("Gain:");

            QSlider Sexposure;
            Sexposure.setOrientation(Qt::Horizontal);
            Sexposure.setMinimum(MIN_EXPOSURE);
            Sexposure.setMaximum(MAX_EXPOSURE);
            Sexposure.setValue( (int)cvcapture.get(CV_CAP_PROP_EXPOSURE) );
            QLabel Lexposure;
            Lexposure.setNum( (int)cvcapture.get(CV_CAP_PROP_EXPOSURE) );
            connect(&Sexposure, SIGNAL(valueChanged(int)), &Lexposure, SLOT(setNum(int)));
            connect(&Sexposure,SIGNAL(valueChanged(int)), this, SLOT(set_exposure(int)));
            connect(this, SIGNAL(set_default_exposure(int)), &Sexposure, SLOT(setValue(int)));
            QLabel Nexposure("Exposure:");

        sliders2.addWidget(&Sgain);
        lineedits2.addWidget(&Lgain, 2);
        names2.addWidget(&Ngain, 1);

        sliders2.addWidget(&Sexposure);
        lineedits2.addWidget(&Lexposure, 2);
        names2.addWidget(&Nexposure, 1);

        centralbox2.addLayout(&names2);
        centralbox2.addLayout(&sliders2);
        centralbox2.addLayout(&lineedits2);
    page2.setLayout( &centralbox2 );


    QHBoxLayout buttons;
    buttons.setMargin(5);
        QPushButton Baccept;
        Baccept.setText("Accept");
        connect(&Baccept, SIGNAL(clicked()), &dialog, SLOT(accept()));
        QPushButton Bcancel;
        Bcancel.setText("Cancel");
        connect(&Bcancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
        QPushButton Bdefault;
        Bdefault.setText("Default");
        connect(&Bdefault, SIGNAL(clicked()), this, SLOT(set_default_settings()));
    buttons.addWidget(&Baccept);
    buttons.addWidget(&Bcancel);
    buttons.addWidget(&Bdefault);

    tabwidget.addTab(&page1, "Brightness/Contrast");
    tabwidget.addTab(&page2, "Gain/Exposure");
    toplevellayout.addWidget(&tabwidget);
    toplevellayout.addLayout(&buttons);
    dialog.setLayout(&toplevellayout);
    //GUI CONSTRUCTION END//
    dialog.exec();
    return true;
    }
    return false; // false if device was not opened
}

bool QVideoCapture::isOpened()
{
    return cvcapture.isOpened();
}

QVideoCapture::~QVideoCapture()
{
    delete ptframetimer;
}

bool QVideoCapture::set_brightness(int value)
{
    return cvcapture.set(CV_CAP_PROP_BRIGHTNESS, (double)value);
}

bool QVideoCapture::set_contrast(int value)
{
    return cvcapture.set(CV_CAP_PROP_CONTRAST, (double)value);
}

bool QVideoCapture::set_saturation(int value)
{
    return cvcapture.set(CV_CAP_PROP_SATURATION, (double)value);
}

bool QVideoCapture::set_gain(int value)
{
    return cvcapture.set(CV_CAP_PROP_GAIN, (double)value);
}

bool QVideoCapture::set_exposure(int value)
{
    return cvcapture.set(CV_CAP_PROP_EXPOSURE, (double)value);
}

bool QVideoCapture::set_white_balanceU(int value)
{
    return cvcapture.set(CV_CAP_PROP_WHITE_BALANCE_BLUE_U, (double)value);
}

bool QVideoCapture::set_white_balanceV(int value)
{
    return cvcapture.set(CV_CAP_PROP_WHITE_BALANCE_RED_V, (double)value);
}

void QVideoCapture::set_default_settings()
{
    emit set_default_brightness(DEFAULT_BRIGHTNESS);
    emit set_default_contrast(DEFAULT_CONTRAST);
    emit set_default_saturation(DEFAULT_SATURATION);
    emit set_default_white_balanceU(DEFAULT_WHITE_BALANCE);
    emit set_default_white_balanceV(DEFAULT_WHITE_BALANCE);
    emit set_default_gain(DEFAULT_GAIN);
    emit set_default_exposure(DEFAULT_EXPOSURE);
}

int QVideoCapture::device_select_dialog()
{
    int device_id = 0;

    QDialog dialog;
    dialog.setFixedSize(256,128);
    dialog.setWindowTitle("Device select dialog");
    QVBoxLayout layout;
        QGroupBox GBid("Select device from list");
            QVBoxLayout Lid;
            QComboBox CBid;
            for(int i = 0; i < DEFAULT_DEVICES_AMMOUNT; i++)
            {
                CBid.addItem("Device №" + QString::number(i));
            }
            Lid.addWidget( &CBid );
       GBid.setLayout(&Lid);

       QHBoxLayout Lbuttons;
            QPushButton Baccept("Accept");
            connect(&Baccept, &QPushButton::clicked, &dialog, &QDialog::accept);
            QPushButton Breject("Reject");
            connect(&Breject, &QPushButton::clicked, &dialog, &QDialog::reject);
       Lbuttons.addWidget(&Baccept);
       Lbuttons.addWidget(&Breject);
   layout.addWidget(&GBid);
   layout.addLayout(&Lbuttons);
   dialog.setLayout(&layout);

    if( dialog.exec() == QDialog::Accepted )
    {
        device_id = CBid.currentIndex();
    }
    return device_id;
}
