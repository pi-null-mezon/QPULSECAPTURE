#ifndef QVIDEOSLIDER_H
#define QVIDEOSLIDER_H

#include <QSlider>
#include <QMouseEvent>

class QVideoSlider : public QSlider
{
    Q_OBJECT
public:
    explicit QVideoSlider(QWidget *parent = 0);

signals:
    void sliderReleased(int position);

private slots:
    void valueOnRelease();
};

#endif // QVIDEOSLIDER_H
