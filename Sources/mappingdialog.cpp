#include "mappingdialog.h"
#include "ui_mappingdialog.h"

mappingdialog::mappingdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mappingdialog)
{
    ui->setupUi(this);
}

mappingdialog::~mappingdialog()
{
    delete ui;
}

void mappingdialog::on_buttonAccept_clicked()
{
    this->accept();
}

void mappingdialog::on_buttonReject_clicked()
{
    this->reject();
}
