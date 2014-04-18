/*-----------------------------------------------------------------------------------------------------
 TARANOV ALEX, March 2014
 QVideoCapture class was created as Qt signal-slot wrapper for OpenCV VideoCapture class

 Work with: first call openfile or opendevice(...), than call start(...) and object will start to emit
 frame_was_captured(const cv::Mat& value) signal. To stop frame capturing use pause() or close(). Also
 class provides some GUI interface to cv::VideoCapture::set(...) function.
------------------------------------------------------------------------------------------------------*/
#ifndef QVIDEOCAPTURE_H
#define QVIDEOCAPTURE_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QDialog>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QVBoxLayout>

#include <opencv2/opencv.hpp>

//---------------------------In most cases the following values are suitable-------------------------
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128

#define MIN_CONTRAST 0
#define MAX_CONTRAST 255
#define DEFAULT_CONTRAST 32

#define MIN_SATURATION 0
#define MAX_SATURATION 255
#define DEFAULT_SATURATION 32

#define MIN_WHITE_BALANCE 0
#define MAX_WHITE_BALANCE 10000
#define DEFAULT_WHITE_BALANCE 4000

#define MIN_GAIN 0
#define MAX_GAIN 255
#define DEFAULT_GAIN 1

#define MIN_EXPOSURE -14
#define MAX_EXPOSURE -1
#define DEFAULT_EXPOSURE -5

#define DEFAULT_FRAME_PERIOD 34 // in ms
#define DEFAULT_DEVICES_AMMOUNT 3 // shows how many variants user can try, not all of this variants will work, for the instance when PC is connected to only one web cam, only one device can be opened
//--------------------------------------------------------------------------------------------------

class QVideoCapture : public QObject
{
    Q_OBJECT
public:
    explicit QVideoCapture(QObject *parent = 0);
    ~QVideoCapture();

signals:
    void frame_was_captured(const cv::Mat& value); // should be emmited right after a new frame was captured, to use in your own Qt-projects first do qRegisterMetaType<cv::Mat>("cv::Mat")
    //------------------------------------------
    void set_default_brightness(int value);
    void set_default_contrast(int value);
    void set_default_saturation(int value);
    void set_default_gain(int value);
    void set_default_exposure(int value);
    void set_default_white_balanceU(int value);
    void set_default_white_balanceV(int value);
    //------------------------------------------

public slots:
    bool openfile(const QString &filename); // this function should to call cv::VideoCapture::open(filename)
    bool opendevice(int device_id, int period); // this function should to call cv::VideoCapture::open(device)
    bool set(int propertyID , double value); // this function should to call cv::VideoCapture::set(propertyID, value)
    bool isOpened(); // return true if cv::VideoCapture is opened
    bool open_resolution_dialog(); // creates an QDialog instance with video device resolution-controls, should be used as a GUI implementation of the camera controll functions
    bool open_settings_dialog(); // creates an QDialog instance with video device characteristic-controls, should be used as a GUI implementation of the camera controll functions
    bool start(); // starts the grabbing with determined period of time in ms
    bool close(); // stops the grabbing and closes video file or capturing device
    bool pause(); // stops the frametimer
    int device_select_dialog(); // what else to say? Should return device id for opendevice(...) function, thus you should use it in than way: class_instance->opendevice( device_select_dialog(), some_value )

private:
    cv::VideoCapture cvcapture; // an OpenCV's object for video capturing from video files or cameras
    cv::Mat frame; // an OpenCV's object for image storing
    QTimer *ptframetimer; // a timer for frames grabbing
    bool device_flag; // this flag should to show when frames are grabbing from video device [true] and when from video file [false] (I was using it for settings and resolution dialogs, which can't be called for video files playback)

private slots:
    bool read_frame(); // calls cv::VideoCapture::read()
    //------------------------------------------
    bool set_brightness(int value);
    bool set_contrast(int value);
    bool set_saturation(int value);
    bool set_gain(int value);
    bool set_exposure(int value);
    bool set_white_balanceU(int value);
    bool set_white_balanceV(int value);
    void set_default_settings();
    //------------------------------------------

};

#endif // QVIDEOCAPTURE_H
