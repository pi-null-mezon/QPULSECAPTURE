/*--------------------------------------------------------------------------------------------
 Grab this from Internet                                                           HEADER FILE
 The Develnoter, 2014
 The way in which integration OpenCV with Qt was performed from Marcelo Mottalli - thanks dude
 * ------------------------------------------------------------------------------------------*/
#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H
//------------------------------------------------------------------------------------------------------

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <opencv2/opencv.hpp>
#include "fftw3.h"

//------------------------------------------------------------------------------------------------------

class QCVImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QCVImageWidget(QWidget *parent = 0);
    ~QCVImageWidget();
    QSize image_size() const;
    void setCountsPointer(double *pointer_to_data, unsigned int length_of_data, unsigned int data_counts_per_screen, fftwf_complex *pointer_to_spectrum, unsigned int length_of_spectrum);
    void unsetCountsPointer();

signals:
    void rect_was_entered(const cv::Rect &value);

public slots:
    void showImage(const cv::Mat &image); // Converts the image to the RGB888 format and calls repaint()

    void drawTexts(QPainter &painter, const QRect &rect);
    void drawSignalTrace(QPainter &painter, const QRect &rect);
    void drawSpectrumTrace(QPainter &painter, const QRect &rect);
    void putTextOnPulse(const QString &input_text, const QColor &input_color);
    void putTextOnSNR(const QString &input_text);
    void putTextOnPeriod(const QString &input_text);
    void setrectflag(bool value);

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QImage qt_image;
    cv::Mat opencv_image;
    //-------------------
    QRect aimrect;
    bool rect_flag; // this flag should enable/disable a drawing of the aimrect on a widget
    int x0;
    int y0;
    //-------------------
    QString stringSNR;
    QString stringPeriod;
    QString stringPulse;
    QColor color;
    //-------------------
    double *ptCounts;
    fftwf_complex *ptSPCounts;
    unsigned int lengthCounts;
    unsigned int lengthSPCounts;
    unsigned int currentCount;
    unsigned int perScreenCounts;
    //-------------------
    unsigned int loop(int difference);
    cv::Rect recalculate_rectregion();

};

//------------------------------------------------------------------------------------------------------

inline QRect get_proportional_rect(QRect rect, int width, int height) // returns rect inside input rect with the same center point, but with proportional sizes corresponding to width and height
{
    if( (width > 0) && (height > 0) )
    {
        int old_width = rect.width();
        int old_height = rect.height();
        if( ((double)width/height) > ((double)old_width/old_height) )
        {
            rect.setHeight( (int)( old_width * height/width ) );
            rect.moveTop( (int)((old_height - rect.height())/2) );
        }
        else
        {
            rect.setWidth( (int)( old_height * width/height ) );
            rect.moveLeft( (int)((old_width - rect.width())/2) );
        }
    }
    return rect;
}

//-------------------------------------------------------------------------------------------------------
inline unsigned int QCVImageWidget::loop(int difference)
{
    return ((lengthCounts + (difference % lengthCounts)) % lengthCounts); // have finded it on wikipedia ), it always returns positive result
}
//-------------------------------------------------------------------------------------------------------
#endif // CVIMAGEWIDGET_H
