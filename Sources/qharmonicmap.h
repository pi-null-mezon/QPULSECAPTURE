#ifndef QHARMONICMAP_H
#define QHARMONICMAP_H

#include <QObject>
#include <QThread>

#include "qharmonicprocessor.h"

class QHarmonicProcessorMap: public QObject
{
    Q_OBJECT

public:
    QHarmonicProcessorMap(QObject* parent = NULL, quint32 width = 32, quint32 height = 32);
    ~QHarmonicProcessorMap();
    enum MapType {VPGMap, SVPGMap, SNRMap, AmpMap};

signals:
    void updateMap();
    void mapUpdated(const qreal *pointer, quint32 width, quint32 height, qreal max, qreal min);
    void dataArrived(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double period);
    void changeColorChannel(int value);
    void updatePCAMode(bool value);
    void setEstimationInterval(int value);

public slots:
    void updateHarmonicProcessor(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double period);
    void setMapType(MapType type_id, bool snrControl);

private:
    quint32 m_cellNum;
    quint32 m_width;
    quint32 m_height;
    quint32 m_length;
    qreal *v_map;
    qreal *v_outputmap;
    QHarmonicProcessor *v_processors;
    QThread *v_threads;
    quint32 m_updations;
    qreal m_min;
    qreal m_max;
    quint32 m_cell;
    quint16 m_threadCount;
    MapType m_type;

private slots:
    void updateCell(quint32 id, qreal value);
};
#endif
