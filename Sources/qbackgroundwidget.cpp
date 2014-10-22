#include <QPainter>
#include "qbackgroundwidget.h"

QBackgroundWidget::QBackgroundWidget(QWidget *parent) :
    QWidget(parent)
{
}

QBackgroundWidget::QBackgroundWidget(QWidget *parent, QColor backgroundColor, QColor foregroundColor, Qt::BrushStyle foregroundStyle):
    QWidget(parent)
{
    m_foregroundBrush.setStyle(foregroundStyle);
    m_foregroundBrush.setColor(foregroundColor);

    m_backgroundBrush.setColor(backgroundColor);
    m_backgroundBrush.setStyle(Qt::SolidPattern);
}

QBackgroundWidget::QBackgroundWidget(QWidget *parent, const QString &fileName, Qt::ImageConversionFlags flags):
    QWidget(parent)
{
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

