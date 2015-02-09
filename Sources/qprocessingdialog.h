#ifndef QPROCESSINGDIALOG_H
#define QPROCESSINGDIALOG_H

#include <QDialog>

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

public slots:
    void setTimer(int value);
    void setInterval(int value);
    void setMaximumInterval(int value);

private slots:
    void on_STimer_valueChanged(int value);

    void on_SInterval_valueChanged(int value);

    void on_BDefault_clicked();

private:
    Ui::QProcessingDialog *ui;
};

#endif // QPROCESSINGDIALOG_H
