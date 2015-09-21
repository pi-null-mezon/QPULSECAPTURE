#ifndef QPROCESSINGDIALOG_H
#define QPROCESSINGDIALOG_H

#include <QDialog>
#include "qharmonicprocessor.h"

namespace Ui {
class QProcessingDialog;
}

class QProcessingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QProcessingDialog(QWidget *parent = 0);
    ~QProcessingDialog();

signals:
    void timerValueUpdated(int value);
    void intervalValueUpdated(int value);
    void breathAverageUpdated(int value);
    void breathStrobeUpdated(int value);
    void breathCNIntervalUpdated(int value);

public slots:
    void setTimer(int value);
    void setValues(int heartEstimation, int breathStrobe, int breathAverage, int breathCNInterval);
    void setLimits(int dataLength);

private slots:
    void on_STimer_valueChanged(int value);

    void on_SInterval_valueChanged(int value);

    void on_BDefault_clicked();

    void on_SbreathStrobe_valueChanged(int value);

    void on_SbreathAverage_valueChanged(int value);

    void on_SbreathCNInterval_valueChanged(int value);

private:
    Ui::QProcessingDialog *ui;
};

#endif // QPROCESSINGDIALOG_H
