#ifndef QEASYPLOT_H
#define QEASYPLOT_H

#include <QWidget>
#include <QPen>
#include <QStaticText>

class QEasyPlot : public QWidget
{
    Q_OBJECT
public:
    explicit QEasyPlot(QWidget *parent = 0);
    explicit QEasyPlot(QWidget *parent, QString nameOfXaxis, QString nameOfYaxis);
    enum DrawRegime {TraceRegime, FilledTraceRegime, PhaseRegime};

protected:
    enum VisualEntity  {Background, Coordinates, Trace};
    void paintEvent(QPaintEvent *);

public slots:
    //data interchange section
    void set_externalArray(const qreal *pointer, quint16 length);
    bool set_horizontal_Borders(qreal left_value, qreal right_value);
    bool set_X_Ticks(quint16 value);   // here, horizontal means a set of ticks on X axis, another words they located in such way: |
    bool set_vertical_Borders(qreal bottom_value, qreal top_value);
    bool set_Y_Ticks(quint16 value); // here, vertical means a set of ticks on Y axis, another words they located in such way: --
    void set_coordinatesPrecision(quint8 value_for_x, quint8 value_for_y); // it is only draw precision (2:3.14 or 1:3.1), not actual, that always qreal
    //visual section
    void set_defaultValues();
    void set_coordinatePen(const QPen &pen, const QColor &color);
    void set_tracePen(const QPen &pen, const QColor &color);
    bool open_traceColorDialog();
    bool open_backgroundColorDialog();
    bool open_coordinatesystemColorDialog();
    bool open_fontSelectDialog();
    void set_axis_names(const QString& name_for_x, const QString & name_for_y);
    void set_DrawRegime(DrawRegime value);

private:  
    bool open_colorSelectDialog_for(VisualEntity value);
    void draw_coordinateSystem(QPainter &painter) const;
    void draw_externalArray(QPainter &painter);

    void update_X_TicksStep();
    void update_Y_TicksStep();

    QPointF convertXY(qreal x, qreal y) const; // use this function everytime you want to draw on Widget correspond to coordinate system
    //variable list
        //data to draw
    const qreal *pt_Array;
    quint32 m_ArrayLength;
        //visual appearance
    QColor m_backgroundColor;
    QPen m_tracePen;
    QPen m_coordinatePen;
    QFont m_coordinateSystemFont;
    quint16 m_XTicks;  // Number of horizontal ticks
    quint16 m_YTicks;    // Number of vertical ticks
    quint8 m_coordinatesXPrecision;    // Precision for coordinate ticks draw
    quint8 m_coordinatesYPrecision;    // Precision for coordinate ticks draw
    quint16 m_textMargin;
    QStaticText m_X_axisName;
    QStaticText m_Y_axisName;
    qreal m_topBorder;
    qreal m_bottomBorder;
    qreal m_leftBorder;
    qreal m_rightBorder;
    qreal m_xStep;  // a qreal step between vertical ticks
    qreal m_yStep;  // a qreal step between horizontal ticks
    DrawRegime m_DrawRegime;
};

#endif // QEASYPLOT_H
