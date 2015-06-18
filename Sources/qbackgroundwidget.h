#ifndef QBACKGROUNDWIDGET_H
#define QBACKGROUNDWIDGET_H

#ifdef REPLACE_WIDGET_TO_OPENGLWIDGET
    #include <QOpenGLWidget>
    #include <QSurfaceFormat>
    #define WIDGET_CLASS QOpenGLWidget
#else
    #include <QWidget>
    #define WIDGET_CLASS QWidget
#endif

class QBackgroundWidget : public WIDGET_CLASS
{
    Q_OBJECT
public:
    explicit QBackgroundWidget(QWidget *parent = 0);
    explicit QBackgroundWidget(QWidget *parent, QColor backgroundColor = QColor(75,75,75), QColor foregroundColor = QColor(175,175,175), Qt::BrushStyle foregroundStyle = Qt::Dense6Pattern);
    explicit QBackgroundWidget(QWidget *parent, const QString & fileName, Qt::ImageConversionFlags flags = Qt::AutoColor);

protected:
    void paintEvent(QPaintEvent *);
    void setSamplesNumber(int value);

private:
    QBrush m_backgroundBrush;
    QBrush m_foregroundBrush;
    QPixmap m_texturePixmap;
};

#endif // QBACKGROUNDWIDGET_H
