#include <QDate>

#include "qsettingsdialog.h"
#include "ui_qsettingsdialog.h"

#define TIMER_INTERVAL 500

QSettingsDialog::QSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QSettingsDialog)
{
    ui->setupUi(this);
}

QSettingsDialog::~QSettingsDialog()
{
    delete ui;
}

//--------------------------------------------
quint32 get_power_of_two(quint16 power)
{
    return (0x00000001 << power);
}
//--------------------------------------------


void QSettingsDialog::on_ButtonAccept_clicked()
{
    this->accept();
}

void QSettingsDialog::on_ButtonCancel_clicked()
{
    this->reject();
}

void QSettingsDialog::on_dialDatalength_valueChanged(int value)
{
    ui->lineEditDatalength->setText(QString::number(get_power_of_two(value)));
    ui->dialBufferlength->setMaximum(value);
}

void QSettingsDialog::on_dialBufferlength_valueChanged(int value)
{
    ui->lineEditBufferlength->setText(QString::number(get_power_of_two(value)));
}

void QSettingsDialog::on_ButtonCascade_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, tr("Open File"), "haarcascades/haarcascade_frontalface_alt.xml", tr("Cascade (*.xml)"));
    if(str.length() != 0)
    {
        ui->lineEditCascade->setText(str);
    }
}

void QSettingsDialog::on_ButtonDefault_clicked()
{
    ui->dialDatalength->setValue(8);
    ui->dialBufferlength->setValue(8);
    ui->checkBoxCascade->setChecked(false);
    ui->lineEditCascade->setText("haarcascades/haarcascade_frontalface_alt.xml");
    ui->checkBoxVideoFile->setChecked(false);
    ui->checkBoxFFT->setChecked(true);
    ui->horizontalSliderTimer->setValue(2); //
    ui->checkBoxPatient->setChecked(true);
    ui->comboBoxPatient->setCurrentIndex(0);
    ui->lineEditPatient->setText("normal_heart_rate_at_rest.xml");
}

bool QSettingsDialog::get_flagCascade() const
{
    return ui->checkBoxCascade->isChecked();
}

bool QSettingsDialog::get_flagVideoFile() const
{
    return ui->checkBoxVideoFile->isChecked();
}

QString QSettingsDialog::get_stringCascade() const
{
    return ui->lineEditCascade->text();
}

quint32 QSettingsDialog::get_datalength() const
{
    return get_power_of_two(ui->dialDatalength->value());
}

quint32 QSettingsDialog::get_bufferlength() const
{
    return get_power_of_two(ui->dialBufferlength->value());
}

void QSettingsDialog::on_checkBoxCascade_stateChanged(int arg1)
{
    switch(arg1)
    {
        case Qt::Checked:
            ui->groupBoxCascade->setEnabled(true);
            break;
        case Qt::Unchecked:
            ui->groupBoxCascade->setEnabled(false);
            break;
    }
}

void QSettingsDialog::on_horizontalSliderTimer_valueChanged(int value)
{
    ui->lineEditTimer->setText(QString::number(value * TIMER_INTERVAL));
}

int QSettingsDialog::get_timerValue() const
{
    return (ui->horizontalSliderTimer->value() * TIMER_INTERVAL);
}

bool QSettingsDialog::get_FFTflag() const
{
    return ui->checkBoxFFT->isChecked();
}

void QSettingsDialog::on_checkBoxPatient_stateChanged(int arg1)
{
    switch(arg1) {
        case Qt::Checked:
            ui->groupBoxPatient->setEnabled(true);
            break;
        case Qt::Unchecked:
            ui->groupBoxPatient->setEnabled(false);
            break;
    }
}

void QSettingsDialog::on_pushButtonPatient_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("distribution (*.xml)"));
    if(!str.isEmpty())
    {
        ui->lineEditPatient->setText(str);
    }
}

int QSettingsDialog::get_patientPercentile() const
{
    return ui->comboBoxPatient->currentIndex();
}

QString QSettingsDialog::get_stringDistribution() const
{
    return ui->lineEditPatient->text();
}

int QSettingsDialog::get_patientAge() const
{
    int age = QDate::currentDate().year() - ui->dateEditPatient->date().year();
    if(QDate::currentDate().month() <= ui->dateEditPatient->date().month())
        age--;
    return age;
}

void QSettingsDialog::on_radioButtonMale_clicked(bool checked)
{
    ui->radioButtonMale->setChecked(checked);
    ui->radioButtonFemale->setChecked(!checked);
}


void QSettingsDialog::on_radioButtonFemale_clicked(bool checked)
{
    ui->radioButtonFemale->setChecked(checked);
    ui->radioButtonMale->setChecked(!checked);
}

bool QSettingsDialog::get_customPatientFlag() const
{
    return ui->checkBoxPatient->isChecked();
}

int QSettingsDialog::get_patientSex() const
{
    if(ui->radioButtonFemale->isChecked())
        return 1; // because harmonicprocessor defines enum SexID { Male, Female };
    else
        return 0;
}


