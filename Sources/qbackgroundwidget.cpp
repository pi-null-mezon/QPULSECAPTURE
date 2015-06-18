#include <QPainter>
#include "qbackgroundwidget.h"

QBackgroundWidget::QBackgroundWidget(QWidget *parent) :
    WIDGET_CLASS(parent)
{
    //setSamplesNumber(SAMPLES_FOR_OPENGLWIDGET);
}

QBackgroundWidget::QBackgroundWidget(QWidget *parent, QColor backgroundColor, QColor foregroundColor, Qt::BrushStyle foregroundStyle):
    WIDGET_CLASS(parent)
{
    //setSamplesNumber(SAMPLES_FOR_OPENGLWIDGET);
    m_foregroundBrush.setStyle(foregroundStyle);
    m_foregroundBrush.setColor(foregroundColor);

    m_backgroundBrush.setColor(backgroundColor);
    m_backgroundBrush.setStyle(Qt::SolidPattern);
}

QBackgroundWidget::QBackgroundWidget(QWidget *parent, const QString &fileName, Qt::ImageConversionFlags flags):
    WIDGET_CLASS(parent)
{
   //setSamplesNumber(SAMPLES_FOR_OPENGLWIDGET);
    m_texturePixmap.load(fileName, 0, flags);
    m_backgroundBrush.setTexture(m_texturePixmap);
    m_backgroundBrush.setStyle(Qt::TexturePattern);
}

void QBackgroundWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    //-------------------------------------------
    painter.fillRect(rect(), m_backgroundBrush);
    //-------------------------------------------
    if(m_texturePixmap.isNull())
    {
        //Reimplement this section as you like
        painter.fillRect(rect(), m_foregroundBrush);
        //-----------------------------------------
    }
}

void QBackgroundWidget::setSamplesNumber(int value)
{
    #ifdef REPLACE_WIDGET_TO_OPENGLWIDGET
        qWarning("QOpenGLWidget default samples: %d", this->format().samples());
        this->setAutoFillBackground(true);
        QSurfaceFormat format;
        format.setSamples(value);   //antialiasing becomes better for high values, but performance becomes too slow (for the instance, 3 is good enought for Pentium IV)
        this->setFormat(format);
        qWarning("QOpenGLWidget manual samples: %d", this->format().samples());
    #endif
}

