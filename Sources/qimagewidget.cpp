/*--------------------------------------------------------------------------------------------
Taranov Alex, 2014                              		               CPP SOURCE FILE
The Develnoter [internet], Marcelo Mottalli, 2014
This class is a descendant of QWidget with QImageWidget::updateImage(...) slot that constructs
QImage instance from cv::Mat image. The QImageWidget should be used as widget for video display  
 * ------------------------------------------------------------------------------------------*/

#include "qimagewidget.h"

#define DEFAULT_OPACITY 72 //8-bit value

//-----------------------------------------------------------------------------------

QImageWidget::QImageWidget(QWidget *parent): QWidget(parent)
{
    pt_data = NULL;
    m_margin = 10;
    m_informationColor = QColor(Qt::black);
    m_contourColor = QColor(Qt::black);
    m_fillColor = QColor(Qt::white);
    m_advancedvisualizationFlag = true;
    m_drawDataFlag = false;
    v_map = NULL;
    m_imageFlag = true;
    m_opacity = DEFAULT_OPACITY;
    computeColorTable();
}

//-----------------------------------------------------------------------------------
void QImageWidget::computeColorTable()
{
    quint16 r = 0;
    quint16 b = 255;
    quint16 g = 0;
    for (quint16 i = 0 ; i < 256; i++)
    {
        if((i < 64) && (g <= 251))
        {
            g += 4;
        }
        else if((i < 128) && (b >= 4))
        {
            b -= 4;
        }
        else if( (i < 192) && (r <= 251))
        {
            r += 4;
        }
        else if((i < 256) && (g >= 4))
        {
            g -= 4;
        }
        v_colors[i] = QColor( r, g, b, m_opacity);
    }
}

//-----------------------------------------------------------------------------------
QImageWidget::~QImageWidget()
{
}

//-----------------------------------------------------------------------------------

void QImageWidget::updateImage(const cv::Mat& image, qreal frame_period, quint32 pixels_enrolled)
{
    m_informationString = QString::number(frame_period, 'f', 1) + tr(" ms, ")
                          + QString::number(image.cols) + "x" + QString::number(image.rows) + " / "
                          + QString::number(pixels_enrolled);

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

    if(m_imageFlag)
        painter.drawImage( temp_rect, qt_image); // Draw inside widget, the image is scaled to fit the rectangle
    else
        painter.fillRect( temp_rect, m_contourColor);

    drawMap(painter, temp_rect);
    drawStrings(painter, temp_rect);          // Will draw m_informationString on the widget

    /*if(m_drawDataFlag)
        drawData(painter, temp_rect);
    */
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
                    painter.drawText(startX, startY, tr("Unreliable"));
                }
                else
                {
                    painter.drawText(startX + m_frequencyString.size() * pointsize * 2.25, startY, tr("bpm"));
                }
                painter.drawText(startX, startY + pointsize*1.5, m_snrString);

                startY +=  pointsize * 5.0 ;
                font.setPointSizeF( pointsize * 3 );
                painter.setFont( font );
                painter.setPen(QColor(0,145,215));
                painter.drawText(startX, startY, m_frequencyString);
                font.setPointSizeF( pointsize );
                painter.setFont( font );
                painter.setPen( m_informationColor );

                if(m_breathRateString.isEmpty())
                {
                    painter.drawText(startX, startY , tr("Unreliable"));
                }
                else
                {
                    painter.drawText(startX + m_breathRateString.size() * pointsize * 2.25, startY, tr("rpm"));
                }
                painter.drawText(startX, startY + pointsize*1.5, m_breathSNRString);
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
                font.setPointSizeF( pointsize * 1.5);
                qreal pX = input_rect.x() + ((qreal)input_rect.width() - m_warningString.size()*pointsize) / 2.0;
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
                    path.addText(startX, startY, font , tr("Unreliable"));
                }
                else
                {
                    path.addText(startX + m_frequencyString.size() * pointsize * 2.25, startY, font , tr("bpm"));
                }
                path.addText(startX, startY + pointsize*1.5, font, m_snrString);

                startY += pointsize * 5.0;
                QPainterPath breathPath;
                font.setPointSizeF(pointsize * 3);
                painter.setFont(font);
                breathPath.addText(startX, startY, font, m_breathRateString);
                painter.setBrush(QColor(0,145,215));
                painter.drawPath(breathPath);
                painter.setBrush(m_fillColor);
                font.setPointSizeF( pointsize );

                if(m_breathRateString.isEmpty())
                {
                    path.addText(startX, startY, font , tr("Unreliable"));
                }
                else
                {
                    path.addText(startX + m_breathRateString.size() * pointsize * 2.25, startY, font , tr("rpm"));
                }
                path.addText(startX, startY + pointsize*1.5, font, m_breathSNRString);
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
    m_snrString = "SNR: " +QString::number(value2,'f',2) + tr(" dB");
}

//-----------------------------------------------------------------------------------

void QImageWidget::updateBreathStrings(qreal breath_rate, qreal snr_value)
{
    m_breathRateString = QString::number(breath_rate, 'f', 0);
    m_breathSNRString = "SNR: " + QString::number(snr_value,'f',2) + tr(" dB");
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
    m_warningString = tr(input_string);
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
    m_snrString = "SNR: " + QString::number(value,'f',2) + tr(" dB");
}

//----------------------------------------------------------------------------------

void QImageWidget::clearBreathRateString(qreal value)
{
    m_breathRateString.clear();
    m_breathSNRString = "SNR: " + QString::number(value,'f',2) + tr(" dB");
}

//----------------------------------------------------------------------------------

void QImageWidget::updadeMapRegion(const cv::Rect &input_rect)
{
    m_mapRect = input_rect;
}

//----------------------------------------------------------------------------------

 void QImageWidget::updateMap(const qreal *pointer, quint32 width, quint32 height, qreal max, qreal min)
{
    v_map = pointer;
    m_mapCols = width;
    m_mapRows = height;
    m_mapMin = min;
    m_mapMax = max;
    m_slope = 1 / (max - min);
    m_intercept = - min * m_slope;
}

//----------------------------------------------------------------------------------

 void QImageWidget::drawMap(QPainter &painter, const QRect &input_rect)
{
    if(v_map)
    {
        QRectF mapRect = findMapRegion(input_rect);
        painter.setPen(QColor(0,0,0,m_opacity));
        int fontSize = mapRect.width() / (m_mapCols * 3);
        if(fontSize > 8)
        {
            painter.setFont(QFont("Calibri", fontSize));
            int alignmentFlag = Qt::AlignHCenter | Qt::AlignVCenter;
            QRectF temp = QRectF(mapRect.x(), mapRect.y(), mapRect.width()/m_mapCols, mapRect.height()/m_mapRows);
            painter.setBrush(v_colors[(int)((v_map[0] * m_slope + m_intercept) * 255)]);
            painter.drawRect(temp);
            painter.drawText(temp, alignmentFlag, QString::number(v_map[0],'f',1));
            for(quint16 i = 1; i < m_mapCols*m_mapRows; i++)
            {
                if((i % m_mapCols) == 0)
                {
                    temp.moveLeft(mapRect.x());
                    temp.moveTop(temp.y()+temp.height());
                }
                else
                {
                    temp.moveLeft(temp.x()+temp.width());
                }
                painter.setBrush(v_colors[(int)((v_map[i] * m_slope + m_intercept) * 255)]);
                painter.drawRect(temp);
                painter.drawText(temp, alignmentFlag, QString::number(v_map[i],'f',1));
            }
        }
        else
        {
            QRectF temp = QRectF(mapRect.x(), mapRect.y(), mapRect.width()/m_mapCols, mapRect.height()/m_mapRows);
            painter.setBrush(v_colors[(int)((v_map[0] * m_slope + m_intercept)*255)]);
            painter.drawRect(temp);
            for(quint16 i = 1; i < m_mapCols*m_mapRows; i++)
            {
                if((i % m_mapCols) == 0)
                {
                    temp.moveLeft(mapRect.x());
                    temp.moveTop(temp.y()+temp.height());
                }
                else
                {
                    temp.moveLeft(temp.x()+temp.width());
                }
                painter.setBrush(v_colors[(int)((v_map[i] * m_slope + m_intercept)*255)]);
                painter.drawRect(temp);
            }
        }

    }
}

//----------------------------------------------------------------------------------
void QImageWidget::selectWholeImage()
{
    emit rect_was_entered( cv::Rect(0,0,opencv_image.cols, opencv_image.rows) );
}
//----------------------------------------------------------------------------------
void QImageWidget::clearMap()
{
    v_map = NULL;
}
//----------------------------------------------------------------------------------
void QImageWidget::setImageFlag(bool value)
{
    m_imageFlag = value;
    if(m_imageFlag)
        m_opacity = DEFAULT_OPACITY;
    else
        m_opacity = 156;
    computeColorTable();
}
