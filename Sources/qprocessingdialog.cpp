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

void QProcessingDialog::setInterval(int value)
{
    ui->SInterval->setValue(value);
    ui->EInterval->setText(QString::number(value));
}

void QProcessingDialog::setMaximumInterval(int value)
{
    ui->SInterval->setMaximum(value);
}

void QProcessingDialog::on_BDefault_clicked()
{
    ui->STimer->setValue(1000);
    ui->SInterval->setValue(16);
}
