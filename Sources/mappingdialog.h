#ifndef MAPPINGDIALOG_H
#define MAPPINGDIALOG_H

#include <QDialog>
#include "qharmonicmap.h"

namespace Ui {
class mappingdialog;
}

class mappingdialog : public QDialog
{
    Q_OBJECT

public:
    explicit mappingdialog(QWidget *parent = 0);
    ~mappingdialog();

public slots:
    void setImageWidth(int value);
    void setImageHeight(int value);
    quint16 getMapWidth() const;
    quint16 getMapHeight() const;
    quint16 getCellSize() const;
    QHarmonicProcessorMap::MapType getMapType() const;

private slots:
    void on_buttonAccept_clicked();
    void on_buttonReject_clicked();
    void on_sliderCell_valueChanged(int value);

private:
    Ui::mappingdialog *ui;
    quint16 m_cellSize;
    quint16 m_width;
    quint16 m_height;
};

#endif // MAPPINGDIALOG_H
