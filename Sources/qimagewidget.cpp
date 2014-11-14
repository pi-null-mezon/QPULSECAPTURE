/*--------------------------------------------------------------------------------------------
Taranov Alex, 2014                              		               CPP SOURCE FILE
The Develnoter [internet], Marcelo Mottalli, 2014
This class is a descendant of QWidget with QImageWidget::updateImage(...) slot that constructs
QImage instance from cv::Mat image. The QImageWidget should be used as widget for video display  
 * ------------------------------------------------------------------------------------------*/

#include "qimagewidget.h"

//-----------------------------------------------------------------------------------

QImageWidget::QImageWidget(QWidget *parent): QWidget(parent)
{
    pt_data = NULL;
    m_margin = 10;
    m_informationColor = QColor(Qt::black);
    m_contourColor = QColor(Qt::black);
    m_fillColor = QColor(Qt::white);
    m_advancedvisualizationFlag = false;
    m_drawDataFlag = false;
    v_map = NULL;
}

//-----------------------------------------------------------------------------------

void QImageWidget::updateImage(const cv::Mat& image, qreal frame_period)
{
    m_informationString = QString::number(frame_period, 'f', 1) + " ms, " + QString::number(image.cols) + "x" + QString::number(image.rows);

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
    update();
}

//------------------------------------------------------------------------------------

void QImageWidget::paintEvent(QPaintEvent* )
{
    QPainter painter( this );
    QRect temp_rect = make_proportional_rect(this->rect(), opencv_image.cols, opencv_image.rows);
    painter.drawImage( temp_rect, qt_image); // Draw inside widget, the image is scaled to fit the rectangle
    drawStrings(painter, temp_rect);          // Will draw m_informationString on the widget
    drawMap(painter, temp_rect);


    /*if(m_drawDataFlag)
    {
        drawData(painter, temp_rect);
    }*/
}

//------------------------------------------------------------------------------------

void QImageWidget::mousePressEvent(QMouseEvent *event)
{
    x0 = event->x();
    y0 = event->y();
    m_aimrect.setX( x0 );
    m_aimrect.setY( y0 );
}

void QImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if( event->x() > x0)
    {
        m_aimrect.setWidth(event->x() - x0);
    }
    else
    {
        m_aimrect.setX( event->x() );
        m_aimrect.setWidth( x0 - event->x() );
    }
    if( event->y() > y0)
    {
        m_aimrect.setHeight(event->y() - y0);
    }
    else
    {
        m_aimrect.setY( event->y() );
        m_aimrect.setHeight( y0 - event->y() );
    }
    emit rect_was_entered( crop_aimrect() );
}

//------------------------------------------------------------------------------------

void QImageWidget::drawStrings(QPainter &painter, const QRect &input_rect)
{
    if(!m_advancedvisualizationFlag)
    {        
        if(!m_informationString.isEmpty())
        {
            qreal startX = input_rect.x() + m_margin;
            qreal startY = input_rect.y();
            qreal pointsize = (qreal)input_rect.height()/32;
            painter.setPen( m_informationColor );
            QFont font("Calibri", pointsize, QFont::DemiBold);
            painter.setFont( font );

            painter.drawText( startX, startY + input_rect.height() - m_margin, m_informationString); // Draws resolution and frametime

            if(!m_warningString.isEmpty())
            {
                font.setPointSizeF( pointsize * 2);
                painter.setFont( font );
                painter.setPen( Qt::red );
                painter.drawText(input_rect, Qt::AlignCenter, m_warningString);
                m_warningString.clear();
            }
            else
            {
                font.setPointSizeF( pointsize * 3 );
                painter.setFont( font );
                painter.setPen( m_frequencyColor );

                startY += font.pointSize() + m_margin;
                painter.drawText(startX, startY, m_frequencyString); // Draws frequency value

                font.setPointSizeF( pointsize );
                painter.setFont( font );
                painter.setPen( m_informationColor );

                if(m_frequencyString.isEmpty())
                {
                    painter.drawText(startX, startY, "Unreliable");
                }
                else
                {
                    painter.drawText(startX + m_frequencyString.size() * pointsize * 2.25, startY, "bpm");                   
                }
                painter.drawText(startX, startY + pointsize*1.5, m_snrString);
            }
        }
    }
    else
    {
        if(!m_informationString.isEmpty())
        {
            painter.setRenderHint(QPainter::Antialiasing); // Turn antialiasing on

            qreal startX = input_rect.x() + m_margin;
            qreal startY = input_rect.y();
            qreal pointsize = (qreal)input_rect.height()/32;

            QPen pen(Qt::NoBrush, 1.0, Qt::SolidLine);
            pen.setColor(m_contourColor);
            painter.setPen( pen );
            painter.setBrush(m_fillColor);
            QFont font("Calibri", pointsize, QFont::Black);
            font.setItalic(true);
            QPainterPath path;
            path.addText( startX, startY + input_rect.height() - m_margin, font, m_informationString); // Draws resolution and frametime

            if(!m_warningString.isEmpty())
            {
                font.setPointSizeF( pointsize * 2);
                qreal pX = input_rect.x() + (input_rect.width() - m_warningString.size()*pointsize - 2*m_margin) / 2.0;
                qreal pY = input_rect.y() + font.pointSizeF() + (input_rect.height() - font.pointSizeF() ) / 2.0;

                QPainterPath path_warning;
                path_warning.addText(pX, pY, font, m_warningString);
                painter.setBrush(Qt::red);
                painter.drawPath(path_warning);
                painter.setBrush(m_fillColor);
                m_warningString.clear();
            }
            else
            {
                QPainterPath path_freq;
                font.setPointSizeF( pointsize * 3 );                          
                startY += font.pointSize() + m_margin;
                path_freq.addText(startX, startY, font, m_frequencyString); // Draws frequency value
                painter.setBrush(m_frequencyColor);
                painter.drawPath(path_freq);

                painter.setBrush(m_fillColor);
                font.setPointSizeF( pointsize );
                if(m_frequencyString.isEmpty())
                {
                    path.addText(startX, startY, font ,"Unreliable");
                }
                else
                {
                    path.addText(startX + m_frequencyString.size() * pointsize * 2.25, startY, font ,"bpm");
                }
                path.addText(startX, startY + pointsize*1.5, font, m_snrString);
            }
            painter.drawPath(path);
        }
    }


}

//-----------------------------------------------------------------------------------

void QImageWidget::updatePointer(const qreal *pointer, quint16 length)
{
    pt_data = pointer;
    m_datalength = length;
    update();
}

//-----------------------------------------------------------------------------------

void QImageWidget::updateValues(qreal value1, qreal value2, bool flag) // value1 for HRfrequency and value2 for SNR
{
    if(flag)
    {
        m_frequencyColor = QColor(Qt::green);
    }
    else
    {
        m_frequencyColor = QColor(Qt::red);
    }
    m_frequencyString = QString::number(value1, 'f', 0);
    m_snrString = "SNR: " +QString::number(value2,'f',2) + " dB";
}

//-----------------------------------------------------------------------------------

void QImageWidget::drawData(QPainter &painter, const QRect &input_rect)
{
    if((pt_data != NULL) && (m_datalength != 0))
    {
        QPen pen(QBrush(Qt::NoBrush),1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setColor(Qt::yellow);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(pen);

        QPainterPath path;

        qreal startY = input_rect.y() + (qreal)input_rect.height()*7/8;
        qreal startX = input_rect.x() + m_margin;
        qreal stepX = (qreal)(input_rect.width() - 2*m_margin)/m_datalength;
        qreal stepY = (qreal)input_rect.height()/24;

        path.moveTo(startX, startY - pt_data[0]*stepY);

        for(quint16 i = 1; i < m_datalength; i++)
        {
            startX += stepX;
            path.lineTo(startX, startY - pt_data[i]*stepY);
        }
        painter.drawPath( path );
        pt_data = NULL; // drop to NULL for external objects protection after drawing operation
    }

}

//-----------------------------------------------------------------------------------

void QImageWidget::switchColorScheme()
{
    if(!m_advancedvisualizationFlag)
    {
        if( m_informationColor == QColor(Qt::black) )
        {
            m_informationColor = QColor(Qt::white);
        }
        else
        {
            m_informationColor = QColor(Qt::black);
        }
    }
    else
    {
        if( m_contourColor == QColor(Qt::black))
        {
            m_contourColor = QColor(Qt::white);
            m_fillColor = QColor(Qt::black);
        }
        else
        {
            m_fillColor = QColor(Qt::white);
            m_contourColor = QColor(Qt::black);
        }
    }
}

//----------------------------------------------------------------------------------

void QImageWidget::set_warning_status(const char * input_string)
{
    m_frequencyString.clear();
    m_warningString = QString( input_string );
}

//----------------------------------------------------------------------------------

void QImageWidget::toggle_advancedvisualization(bool value)
{
    m_advancedvisualizationFlag = value;
}

//----------------------------------------------------------------------------------

void QImageWidget::set_drawDataFlag(bool value)
{
    m_drawDataFlag = value;
}

//----------------------------------------------------------------------------------

void QImageWidget::clearFrequencyString(qreal value)
{
    m_frequencyString.clear();
    m_snrString = "SNR: " + QString::number(value,'f',2) + " dB";
}

//----------------------------------------------------------------------------------

void QImageWidget::updadeMapRegion(const cv::Rect &input_rect)
{
    m_mapRect = input_rect;
}

//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
 void QImageWidget::updateMap(const qreal *pointer, quint32 width, quint32 height, qreal max, qreal min)
{
    v_map = pointer;
    m_mapCols = width;
    m_mapRows = height;
    m_mapMin = min;
    m_mapMax = max;
}

//----------------------------------------------------------------------------------
void QImageWidget::drawMap(QPainter &painter, const QRect &input_rect)
{
    if(v_map)
    {
        QRect rect = findMapRegion(input_rect);
        qreal stepX = (qreal)rect.width() / m_mapCols;
        qreal stepY = (qreal)rect.height() / m_mapRows;

        for(quint16 i = 0; i < m_mapCols*m_mapRows; i++)
        {
            painter.fill
        }


        //painter.setBrush(QColor(255,0,0,125));
        painter.fillRect(rect, QColor(255,0,0,0));
    }



}
