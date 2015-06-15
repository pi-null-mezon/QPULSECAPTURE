#include "qprocessingdialog.h"
#include "ui_qprocessingdialog.h"

QProcessingDialog::QProcessingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QProcessingDialog)
{
    ui->setupUi(this);
}

QProcessingDialog::~QProcessingDialog()
{
    delete ui;
}

void QProcessingDialog::on_STimer_valueChanged(int value)
{
    ui->ETimer->setText(QString::number(value));
    emit timerValueUpdated(value);
}

void QProcessingDialog::on_SInterval_valueChanged(int value)
{
    ui->EInterval->setText(QString::number(value));
    emit intervalValueUpdated(value);
}

void QProcessingDialog::setTimer(int value)
{
    ui->STimer->setValue(value);
    ui->ETimer->setText(QString::number(value));
}

void QProcessingDialog::setValues(int heartEstimation, int breathStrobe, int breathAverage, int breathCNInterval)
{
    ui->SInterval->setValue(heartEstimation);
    ui->EInterval->setText(QString::number(heartEstimation));

    ui->SbreathStrobe->setValue(breathStrobe);
    ui->EbreathStrobe->setText(QString::number(breathStrobe));

    ui->SbreathAverage->setValue(breathAverage);
    ui->EbreathAverage->setText(QString::number(breathAverage));

    ui->SbreathCNInterval->setValue(breathCNInterval);
    ui->EbreathCNInterval->setText(QString::number(breathCNInterval));
}

void QProcessingDialog::setLimits(int dataLength)
{
    ui->SInterval->setMaximum(dataLength);
    ui->SbreathAverage->setMaximum(dataLength);
    ui->SbreathCNInterval->setMaximum(dataLength);
}

void QProcessingDialog::on_BDefault_clicked()
{
    ui->STimer->setValue(1000);
    ui->SInterval->setValue(DEFAULT_NORMALIZATION_INTERVAL);
    ui->SbreathStrobe->setValue(DEFAULT_BREATH_STROBE);
    ui->SbreathAverage->setValue(DEFAULT_BREATH_AVERAGE);
    ui->SbreathCNInterval->setValue(DEFAULT_BREATH_NORMALIZATION_INTERVAL);
}

void QProcessingDialog::on_SbreathStrobe_valueChanged(int value)
{
    ui->EbreathStrobe->setText(QString::number(value));
    emit breathStrobeUpdated(value);
}

void QProcessingDialog::on_SbreathAverage_valueChanged(int value)
{
    ui->EbreathAverage->setText(QString::number(value));
    emit breathAverageUpdated(value);
}

void QProcessingDialog::on_SbreathCNInterval_valueChanged(int value)
{
    ui->EbreathCNInterval->setText(QString::number(value));
    emit breathCNIntervalUpdated(value);
}
