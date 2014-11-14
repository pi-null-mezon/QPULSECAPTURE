/*------------------------------------------------------------------------------------------------------
Taranov Alex, 2014									     HEADER FILE
Class that wraps opencv functions into Qt SIGNAL/SLOT interface
The simplest way to use it - rewrite appropriate section in QOpencvProcessor::customProcess(...) slot
------------------------------------------------------------------------------------------------------*/

#ifndef QOPENCVPROCESSOR_H
#define QOPENCVPROCESSOR_H
//------------------------------------------------------------------------------------------------------

#include <QObject>
#include <opencv2/opencv.hpp>

//------------------------------------------------------------------------------------------------------

class QOpencvProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QOpencvProcessor(QObject *parent = 0);

signals:
    void frameProcessed(const cv::Mat& value, double frame_period); //should be emited in the end of each frame processing
    void dataCollected(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double period);
    void selectRegion(const char * string);     // emit it if no objects has been detected or no regions are selected
    void mapCellProcessed(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double period);
    void mapRegionUpdated(const cv::Rect& rect);

public slots:
    void customProcess(const cv::Mat &input);   // just a template of how a program logic should work
    void updateTime();                          // use it in the beginning of any time-measurement operations
    void setRect(const cv::Rect &input_rect);   // sets m_cvrect
    void faceProcess(const cv::Mat &input);     // an algorithm that evaluates PPG from skin region, region evaluates by means of opencv's cascadeclassifier functions
    void rectProcess(const cv::Mat &input);     // an algorithm that evaluates PPG from skin region defined by user
    bool loadClassifier(const std::string& filename); // an interface to CascadeClassifier::load(...) function
    void setFullFaceFlag(bool value);           // interface to define if algorithm will process full rectangle region returned by detectmultiscale(...) or parts them
    void mapProcess(const cv::Mat &input);

    cv::Rect getRect(); // returns current m_cvRect
    void setMapRegion(const cv::Rect &input_rect); // sets up map region, see m_mapRect
    void setMapCellSize(quint16 sizeX, quint16 sizeY);

private:
    bool m_fullFaceFlag;
    int64 m_timeCounter;    // stores time of application/computer start
    double m_framePeriod;   // stores time of frame processing
    cv::Rect m_cvRect;      // this rect is used by process_rectregion_pulse slot
    cv::CascadeClassifier m_classifier; //object that manages opencv's image recognition functions

    quint16 m_mapCellSizeX;
    quint16 m_mapCellSizeY;
    cv::Rect m_mapRect;
    unsigned char **v_pixelSet; // memory should be allocated in setMapCellSize() call
};

//------------------------------------------------------------------------------------------------------
#endif // QOPENCVPROCESSOR_H
