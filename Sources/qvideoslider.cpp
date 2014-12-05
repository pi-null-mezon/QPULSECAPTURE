#include "qvideoslider.h"

QVideoSlider::QVideoSlider(QWidget *parent) :
    QSlider(parent)
{
    connect(this, SIGNAL(sliderReleased()), this, SLOT(valueOnRelease()));
}

void QVideoSlider::valueOnRelease()
{
    emit sliderReleased(value());
}

