#include "settingsdialog.h"
#include "ui_settingsdialog.h"


unsigned long int get_power_of_two(int power)
{
    unsigned long int temp = 1;
    while( power-- )
    {
        temp *= 2;
    }
    return temp;
}

Settingsdialog *ptdialog;

Settingsdialog::Settingsdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settingsdialog)
{
    ui->setupUi(this);
}

Settingsdialog::~Settingsdialog()
{
    delete ui;
}

void Settingsdialog::on_Ddatalength_valueChanged(int value)
{
   ui->Edatalength->setText(QString::number(get_power_of_two(value)));
   ui->Dbufferlength->setMaximum(value);
   ui->Dcounts->setMaximum(value);
}

void Settingsdialog::on_Dbufferlength_valueChanged(int value)
{
    ui->Ebufferlength->setText(QString::number(get_power_of_two(value)));
}

void Settingsdialog::on_Dcounts_valueChanged(int value)
{
    ui->Ecounts->setText(QString::number(get_power_of_two(value)));
}

void Settingsdialog::on_TBcascade_clicked()
{
     QString filename = QFileDialog::getOpenFileName(this, tr("Choose classifier cascade file"), "", tr("Files (*.xml)"));
     if(filename.length() != 0)
     {
         ui->Ecascade->setText(filename);
     }
}

void Settingsdialog::on_TBvideo_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose video file"), "", tr("Files (*.avi;*.mp4;*.wmv)"));
    if(filename.length() != 0)
    {
        ui->Evideo->setText(filename);
    }
}

void Settingsdialog::on_CBtrigger_stateChanged(int arg1)
{
    switch (arg1) {
    case Qt::Checked:
    {
        ui->CBpca->setChecked(false);
        ui->CBpca->setEnabled(false);
        break;
    }
    case Qt::Unchecked:
    {
        ui->CBpca->setEnabled(true);
        break;
    }
        }
}

void Settingsdialog::on_CBpca_stateChanged(int arg1)
{
    switch (arg1) {
    case Qt::Checked:
    {
        ui->CBtrigger->setChecked(false);
        ui->CBtrigger->setEnabled(false);
        break;
    }
    case Qt::Unchecked:
    {
        ui->CBtrigger->setEnabled(true);
        break;
    }
        }
}

void Settingsdialog::on_TBrecord_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "/Test_record.txt", tr("Text (*.txt)"));
    if(fileName.length() != 0)
    {
        ui->Erecord->setText( fileName );
    }
}

bool Settingsdialog::isCBrecord_checked()
{
    return ui->GBrecord->isChecked();
}

bool Settingsdialog::isCBvideo_checked()
{
    return ui->GBvideo->isChecked();
}

bool Settingsdialog::isCBpca_checked()
{
    return ui->CBpca->isChecked();
}

bool Settingsdialog::isCBcolor_checked()
{
    return ui->CBcolor->isChecked();
}

bool Settingsdialog::isCBtrigger_checked()
{
    return ui->CBtrigger->isChecked();
}

unsigned int Settingsdialog::getSrecord_position()
{
    return ui->Dtimer->value();
}

unsigned int Settingsdialog::get_bufferlength()
{
    return ui->Ebufferlength->text().toUInt();
}

unsigned int Settingsdialog::get_datalength()
{
    return ui->Edatalength->text().toUInt();
}

unsigned int Settingsdialog::get_countsperscreen()
{
    return ui->Ecounts->text().toUInt();
}

QString Settingsdialog::getEvideo_text()
{
    return ui->Evideo->text();
}

QString Settingsdialog::getErecord_text()
{
    return ui->Erecord->text();
}

QString Settingsdialog::getEcascade_text()
{
    return ui->Ecascade->text();
}

void Settingsdialog::on_GBvideo_clicked(bool checked)
{
    if(checked)
    {
        ui->Evideo->setEnabled(true);
        ui->TBvideo->setEnabled(true);
    }
}

bool Settingsdialog::isCBoutputsignal_checked()
{
    return ui->GBsignal->isChecked();
}

void Settingsdialog::on_TBsignal_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "/Test_signal.txt", tr("Text (*.txt)"));
    if(fileName.length() != 0)
    {
        ui->Esignal->setText( fileName );
    }
}

void Settingsdialog::on_Dtimer_valueChanged(int value)
{
    ui->Etimer->setText( QString::number( value ) + " sec" );
}

QString Settingsdialog::getEsignal_text()
{
    return ui->Esignal->text();
}

void Settingsdialog::on_GBsignal_clicked(bool checked)
{
   ui->Esignal->setEnabled(checked);
}

void Settingsdialog::on_GBrecord_clicked(bool checked)
{
    ui->Erecord->setEnabled(checked);
}

void Settingsdialog::on_GBcascade_clicked(bool checked)
{
    ui->Ecascade->setEnabled(checked);
}

bool Settingsdialog::isCBcascade_checked()
{
    return ui->GBcascade->isChecked();
}

void Settingsdialog::on_buttonBox_accepted()
{

}
