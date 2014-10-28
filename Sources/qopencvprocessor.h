/*------------------------------------------------------------------------------------------------------
Taranov Alex, 2014									     HEADER FILE
Class that wraps opencv functions into Qt SIGNAL/SLOT interface
The simplest way to use it - rewrite appropriate section in QOpencvProcessor::custom_algorithm(...) slot
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
    void frame_was_processed(const cv::Mat& value, double frame_period); //should be emited in the end of each frame processing
    void colors_were_evaluated(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double period);
    void no_regions_selected(const char * string); // emit it if no objects has been detected or no regions are selected

public slots:
    void custom_algorithm(const cv::Mat &input);    // just a template of how a program logic should work
    void update_timeCounter();                      // use it in the beginning of any time-measurement operations
    void setRect(const cv::Rect &input_rect);       // sets m_cvrect
    void pulse_processing_with_classifier(const cv::Mat &input); // an algorithm that evaluates PPG from skin region, region evaluates by means of opencv's cascadeclassifier functions
    void pulse_processing_custom_region(const cv::Mat &input); // an algorithm that evaluates PPG from skin region defined by user
    bool load_cascadecalssifier_file(const std::string& filename); // an interface to CascadeClassifier::load(...) function
    bool check_classifier_isempty(); // an interface to CascadeClsssifier::empty() method
    void setFullFaceFlag(bool value); // interface to define if algorithm will process full rectangle region returned by detectmultiscale(...) or parts them

private:
    bool m_fullFaceFlag;
    int64 m_timeCounter;    // stores time of application/computer start
    double m_framePeriod;   // stores time of frame processing
    cv::Rect m_cvRect;      // this rect is used by process_rectregion_pulse slot
    cv::CascadeClassifier m_classifier; //object that manages opencv's image recognition functions
};

//------------------------------------------------------------------------------------------------------
#endif // QOPENCVPROCESSOR_H
