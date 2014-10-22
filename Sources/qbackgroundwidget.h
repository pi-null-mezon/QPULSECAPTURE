#ifndef QBACKGROUNDWIDGET_H
#define QBACKGROUNDWIDGET_H

#include <QWidget>

class QBackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QBackgroundWidget(QWidget *parent = 0);
    explicit QBackgroundWidget(QWidget *parent, QColor backgroundColor = QColor(75,75,75), QColor foregroundColor = QColor(175,175,175), Qt::BrushStyle foregroundStyle = Qt::Dense7Pattern);
    explicit QBackgroundWidget(QWidget *parent, const QString & fileName, Qt::ImageConversionFlags flags = Qt::AutoColor);

protected:
    void paintEvent(QPaintEvent *);

private:
    QBrush m_backgroundBrush;
    QBrush m_foregroundBrush;
    QPixmap m_texturePixmap;
};

#endif // QBACKGROUNDWIDGET_H
