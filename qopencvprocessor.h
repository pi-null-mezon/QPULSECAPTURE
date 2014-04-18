#ifndef QOPENCVPROCESSOR_H
#define QOPENCVPROCESSOR_H
//------------------------------------------------------------------------------------------------------

#include <QObject>
#include <opencv2/opencv.hpp>

#define MIN_WIDTH_FOR_FACE_PROJECTION 169

//------------------------------------------------------------------------------------------------------

class QOpencvprocessor : public QObject
{
    Q_OBJECT
public:
    explicit QOpencvprocessor(QObject *parent = 0);
    ~QOpencvprocessor();

signals:
    void frame_was_processed(const cv::Mat& value);
    void values_were_evaluated(unsigned int channum, const unsigned long sumred, const unsigned long sumgreen, const unsigned long sumblue, const unsigned long area, const double time);

public slots:
    void process_custom_rule(const cv::Mat& input); // just a template of how a program logic should work
    void process_face_pulse(const cv::Mat &input_image); // performs a channelwise intensity averaging across current image with region determination by means of OpenCV cascadeclassifier
    void process_rectregion_pulse(const cv::Mat &input_image); // performs a channelwise intensity averaging across the rect on current image

    bool loadclassifier(const std::string&); //loads classifier from suitable xml file
    void actualizetimecounter(); // use it in the beginning of any time-measurement operations
    void actualizerect(const cv::Rect &input_rect);

private:
    long double timecounter;
    double frametime;
    cv::CascadeClassifier classifier;
    cv::Rect rect; // this rect is used by process_rectregion_pulse slot
};

//------------------------------------------------------------------------------------------------------
#endif // QOPENCVPROCESSOR_H
