/*--------------------------------------------------------------------------------------------
 Grabbed this from Internet                                                       CPP SOURCE FILE
 The Develnoter, 2014
 The way in which integration OpenCV with Qt was performed from Marcelo Mottalli - thanks dude
 * ------------------------------------------------------------------------------------------*/

#include "CVImageWidget.h"

//-----------------------------------------------------------------------------------

QCVImageWidget::QCVImageWidget(QWidget *parent): QWidget(parent)
{
    unsetCountsPointer();
    rect_flag = false;
}

//-----------------------------------------------------------------------------------

QCVImageWidget::~QCVImageWidget()
{

}

//-----------------------------------------------------------------------------------

void QCVImageWidget::showImage(const cv::Mat& image)
{
    switch ( image.type() )
    {
        case CV_8UC1:
            cv::cvtColor(image, opencv_image, CV_GRAY2RGB);
            break;

        case CV_8UC3:
            cv::cvtColor(image, opencv_image, CV_BGR2RGB);
            break;
    }

    assert(opencv_image.isContinuous()); // QImage needs the data to be stored continuously in memory
    qt_image = QImage(opencv_image.data, opencv_image.cols, opencv_image.rows, opencv_image.cols * 3, QImage::Format_RGB888);  // Assign OpenCV's image buffer to the QImage
    repaint();
}

//------------------------------------------------------------------------------------

void QCVImageWidget::paintEvent(QPaintEvent* )
{
    QPainter painter( this );
    QRect rect = get_proportional_rect(this->rect(), opencv_image.cols, opencv_image.rows);
    painter.drawImage( rect, qt_image); // Draw inside widget, the image is scaled to fit the rectangle
    drawTexts(painter, rect);
    if(ptCounts)
    {
        drawSignalTrace(painter, rect);
    }
    if(ptSPCounts)
    {
        drawSpectrumTrace(painter, rect);
    }
    /*if(rect_flag)
    {
        painter.drawRect( rect.intersected(aimrect) );
    }*/
}

//------------------------------------------------------------------------------------

void QCVImageWidget::mousePressEvent(QMouseEvent *event)
{
    if(rect_flag)
    {
        x0 = event->x();
        y0 = event->y();
        aimrect.setX( x0 );
        aimrect.setY( y0 );
    }
}

void QCVImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(rect_flag)
    {
        if( event->x() > x0)
        {
            aimrect.setWidth(event->x() - x0);
        }
        else
        {
            aimrect.setX( event->x() );
            aimrect.setWidth( x0 - event->x() );
        }
        if( event->y() > y0)
        {
            aimrect.setHeight(event->y() - y0);
        }
        else
        {
            aimrect.setY( event->y() );
            aimrect.setHeight( y0 - event->y() );
        }
        emit rect_was_entered( recalculate_rectregion() );
    }
}

//------------------------------------------------------------------------------------

void QCVImageWidget::drawTexts(QPainter &painter, const QRect &rect)
{
        painter.setPen(color);
        int point_size = rect.height()/10;
        painter.setFont(QFont("Calibri", point_size, QFont::Bold));
        painter.drawText(rect.x()+5, rect.y() + point_size, stringPulse);
        point_size /= 2;
        if( stringPulse.length()!= 0 )
        {
            painter.setFont(QFont("Calibri", point_size, QFont::Bold));
             painter.setPen(QColor(225,225,225));
            painter.drawText(rect.x() + stringPulse.length()*1.5*point_size + 5, rect.y() + point_size, "bpm");
        }

        point_size /= 2;
        painter.setPen(QColor(215,215,215));
        painter.setFont(QFont("Calibri", point_size, QFont::Normal));
        painter.drawText(rect.x() + rect.width() - 6 * point_size, rect.y() + 2 * point_size, stringPeriod);
        painter.drawText(rect.x() + rect.width() - 6 * point_size, rect.y() + 3 * point_size, stringSNR);
}

//------------------------------------------------------------------------------------


void QCVImageWidget::putTextOnPulse(const QString &input_text, const QColor &input_color)
{
    stringPulse = input_text;
    color = input_color;
}

//------------------------------------------------------------------------------------

void QCVImageWidget::putTextOnSNR(const QString &input_text)
{
    stringSNR = input_text;
}

//------------------------------------------------------------------------------------

void QCVImageWidget::putTextOnPeriod(const QString &input_text)
{
    stringPeriod = input_text;
    currentCount = (++currentCount) % lengthCounts; // looped increment
}

//------------------------------------------------------------------------------------

QSize QCVImageWidget::image_size() const
{
     return qt_image.size();
}

//------------------------------------------------------------------------------------

void QCVImageWidget::setCountsPointer(double *pointer_to_data, unsigned int length_of_data, unsigned int data_counts_per_screen, fftwf_complex *pointer_to_spectrum, unsigned int length_of_spectrum)
{
    ptCounts = pointer_to_data;
    ptSPCounts = pointer_to_spectrum;
    lengthCounts = length_of_data;
    lengthSPCounts = length_of_spectrum / 2 + 1 ; // see Algorithmic unit, FFT from real data
    perScreenCounts = data_counts_per_screen;
    currentCount = 0;
}

//------------------------------------------------------------------------------------

void QCVImageWidget::drawSignalTrace(QPainter &painter, const QRect &rect)
{

    unsigned int stepX = rect.width()/perScreenCounts;
    unsigned int startX = rect.x() + rect.width() - (rect.width() - stepX * perScreenCounts)/2;
    unsigned int startY = 7*rect.height()/8;
    float scale = rect.height()/25;

    painter.drawLine(startX, startY - scale * ptCounts[currentCount] - scale/4, startX, startY - scale * ptCounts[currentCount] + scale/4);
    painter.drawLine(startX - scale/4, startY - scale * ptCounts[currentCount], startX + scale/4, startY - scale * ptCounts[currentCount]);
    unsigned int Alpha = 255;
    unsigned int stepAlpha = 256/perScreenCounts;
    for (unsigned int i = 0 ; i < (perScreenCounts-1) ; i++)
    {
        painter.setPen(QColor(0,225,0,Alpha));
        painter.drawLine(startX - stepX*i, startY - scale * ptCounts[loop(currentCount-i)], startX - stepX*(i+1), startY - scale * ptCounts[loop(currentCount-(i+1))]);
        Alpha -= stepAlpha;
    }
}

//------------------------------------------------------------------------------------

void QCVImageWidget::unsetCountsPointer()
{
    ptCounts = NULL;
    ptSPCounts = NULL;
}

//------------------------------------------------------------------------------------

void QCVImageWidget::drawSpectrumTrace(QPainter &painter, const QRect &rect)
{
    unsigned int stepX = rect.width()/lengthSPCounts;
    unsigned int startX = rect.x() + (rect.width() - stepX * lengthSPCounts)/2;
    unsigned int startY = rect.height() - stepX;
    float scale = (float)rect.height()/16192; // i take this value by means of selection

    QPainterPath path;
    path.moveTo( startX , startY - scale * (ptSPCounts[0][0]*ptSPCounts[0][0] + ptSPCounts[0][1]*ptSPCounts[0][1]) );
    for (unsigned int i = 1 ; i < lengthSPCounts ; i++)
    {
        path.lineTo( startX + stepX * i, startY - scale * (ptSPCounts[i][0]*ptSPCounts[i][0] + ptSPCounts[i][1]*ptSPCounts[i][1]));
    }
    painter.setPen(QColor(255,255,255,128));
    painter.drawPath(path);
}

//------------------------------------------------------------------------------------

void QCVImageWidget::setrectflag(bool value)
{
    rect_flag = value;
}

//------------------------------------------------------------------------------------

cv::Rect QCVImageWidget::recalculate_rectregion()
{
    QRect workfield = get_proportional_rect(this->rect(), opencv_image.cols, opencv_image.rows);
    QRect region = workfield.intersected( aimrect );
    int output_x = (int)(( (double)(region.x() - workfield.x())/workfield.width() ) * opencv_image.cols);
    int output_y = (int)(( (double)(region.y() - workfield.y())/workfield.height() ) * opencv_image.rows);
    int output_w = (int)(( (double)region.width()/workfield.width() ) * opencv_image.cols);
    int output_h = (int)(( (double)region.height()/workfield.height() ) * opencv_image.rows);
    cv::Rect output_rect(output_x, output_y, output_w, output_h);
    return output_rect;
}
