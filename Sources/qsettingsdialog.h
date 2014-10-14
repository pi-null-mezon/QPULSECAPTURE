#ifndef QSETTINGSDIALOG_H
#define QSETTINGSDIALOG_H

#include <QDialog>
#include <QString>
#include <QFileDialog>

namespace Ui {
class QSettingsDialog;
}

class QSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QSettingsDialog(QWidget *parent = 0);
    ~QSettingsDialog();


public:
    bool get_flagColor() const;
    bool get_flagPCA() const;
    bool get_FFTflag() const;
    bool get_flagCascade() const;
    bool get_flagRecord() const;
    bool get_flagVideoFile() const;
    QString get_stringCascade() const;
    QString get_stringRecord() const;
    QString get_stringVideoFile() const;
    quint32 get_datalength() const;
    quint32 get_bufferlength() const;
    int get_timerValue() const;

private slots:


    void on_ButtonAccept_clicked();

    void on_ButtonCancel_clicked();

    void on_dialDatalength_valueChanged(int value);

    void on_dialBufferlength_valueChanged(int value);

    void on_ButtonCascade_clicked();

    void on_pushButtonRecord_clicked();

    void on_ButtonDefault_clicked();

    void on_pushButtonVideoFile_clicked();

    void on_checkBoxVideoFile_stateChanged(int arg1);

    void on_checkBoxRecord_stateChanged(int arg1);

    void on_checkBoxCascade_stateChanged(int arg1);

    void on_checkBoxColor_stateChanged(int arg1);

    void on_horizontalSliderTimer_valueChanged(int value);

private:
    Ui::QSettingsDialog *ui;
};

#endif // QSETTINGSDIALOG_H
