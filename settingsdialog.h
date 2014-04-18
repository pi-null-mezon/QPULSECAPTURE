#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class Settingsdialog;
}

class Settingsdialog : public QDialog
{
    Q_OBJECT

public:
    explicit Settingsdialog(QWidget *parent = 0);
    ~Settingsdialog();

public:
     bool isCBrecord_checked();
     bool isCBvideo_checked();
     bool isCBpca_checked();
     bool isCBcolor_checked();
     bool isCBtrigger_checked();
     bool isCBoutputsignal_checked();
     bool isCBcascade_checked();
     unsigned int getSrecord_position();
     unsigned int get_bufferlength();
     unsigned int get_datalength();
     unsigned int get_countsperscreen();
     QString getEvideo_text();
     QString getErecord_text();
     QString getEsignal_text();
     QString getEcascade_text();

private slots:

    void on_Ddatalength_valueChanged(int value);

    void on_Dbufferlength_valueChanged(int value);

    void on_Dcounts_valueChanged(int value);

    void on_TBcascade_clicked();

    void on_TBvideo_clicked();

    void on_CBtrigger_stateChanged(int arg1);

    void on_CBpca_stateChanged(int arg1);

    void on_TBrecord_clicked();

    void on_GBvideo_clicked(bool checked);

    void on_TBsignal_clicked();

    void on_Dtimer_valueChanged(int value);

    void on_GBsignal_clicked(bool checked);

    void on_GBrecord_clicked(bool checked);

    void on_GBcascade_clicked(bool checked);

    void on_buttonBox_accepted();

private:
    Ui::Settingsdialog *ui;
};

extern Settingsdialog *ptdialog;

#endif // SETTINGSDIALOG_H
