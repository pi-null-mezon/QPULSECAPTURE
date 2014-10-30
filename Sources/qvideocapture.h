/*-----------------------------------------------------------------------------------------------------
Taranov Alex, 2014 									    HEADER FILE
QVideoCapture class was created as Qt SIGNAL/SLOT wrapper for OpenCV VideoCapture class
Work with: call openfile(...) or opendevice(...) and on success, object will start to emit
frame_was_captured(const cv::Mat& value) signal with determined period of time.
To stop frame capturing use pause() or close(). Also
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
#include <QCamera>
#include <QList>
#include <QByteArray>

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

#define MIN_WHITE_BALANCE 1000
#define MAX_WHITE_BALANCE 9999
#define DEFAULT_WHITE_BALANCE 4000

#define MIN_GAIN 0
#define MAX_GAIN 255
#define DEFAULT_GAIN 1

#define MIN_EXPOSURE -14
#define MAX_EXPOSURE -1
#define DEFAULT_EXPOSURE -5

#define DEFAULT_FRAME_PERIOD 35 // in ms
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
    bool openfile(const QString &filename);             // this function should to call cv::VideoCapture::open(filename)
    bool opendevice(int period = DEFAULT_FRAME_PERIOD); // this function should to call cv::VideoCapture::open(device)
    bool isOpened();                                    // return true if cv::VideoCapture is opened
    bool start();                                       // starts the grabbing
    bool resume();                                      // starts the grabbing with determined period of time in ms
    bool close();                                       // stops the grabbing and closes video file or capturing device
    bool pause();                                       // stops the frametimer
    int open_deviceSelectDialog();                      // On success it should return device id for opendevice(...) function, thus you should use it in than way: class_instance->opendevice( device_select_dialog(), some_value )
    bool open_resolutionDialog();                       // creates an QDialog instance with video device resolution-controls, should be used as a GUI implementation of the camera controll functions
    bool open_settingsDialog();                         // creates an QDialog instance with video device characteristic-controls, should be used as a GUI implementation of the camera controll functions
    //------------------------------------------
    bool set(int propertyID , double value);// this function should to call cv::VideoCapture::set(propertyID, value)
    bool set_brightness(int value);
    bool set_contrast(int value);
    bool set_saturation(int value);
    bool set_gain(int value);
    bool set_exposure(int value);
    bool set_white_balanceU(int value);
    bool set_white_balanceV(int value);
    void set_default_settings();
    //------------------------------------------

private:
    cv::VideoCapture m_cvCapture;           // an OpenCV's object for video capturing from video files or cameras
    cv::Mat m_frame;                        // an OpenCV's object for image storing
    QTimer *pt_timer;                       // a timer for frames grabbing
    bool deviceFlag;                        // this flag should to show when frames are grabbing from video device [true] and when from video file [false] (I was using it for settings and resolution dialogs, which can't be called for video files playback)
    int device_id;                          // stores the curent device identifier, 0 on default

private slots:
    bool read_frame();                      // calls cv::VideoCapture::read()
};

#endif // QVIDEOCAPTURE_H
