#ifndef MAPPINGDIALOG_H
#define MAPPINGDIALOG_H

#include <QDialog>

namespace Ui {
class mappingdialog;
}

class mappingdialog : public QDialog
{
    Q_OBJECT

public:
    explicit mappingdialog(QWidget *parent = 0);
    ~mappingdialog();

private slots:
    void on_buttonAccept_clicked();

    void on_buttonReject_clicked();

private:
    Ui::mappingdialog *ui;
};

#endif // MAPPINGDIALOG_H
