/*--------------------------------------------------------------------------------------------
Taranov Alex, 2014                              		                   HEADER FILE
The Develnoter [internet], Marcelo Mottalli, 2014
This class is a descendant of QWidget with QImageWidget::updateImage(...) slot that constructs
QImage instance from cv::Mat image. The QImageWidget should be used as widget for video display  
 * ------------------------------------------------------------------------------------------*/

#ifndef QIMAGEWIDGET_H
#define QIMAGEWIDGET_H
//------------------------------------------------------------------------------------------------------

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>

#include <opencv2/opencv.hpp>

//------------------------------------------------------------------------------------------------------

class QImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QImageWidget(QWidget *parent = 0);
    ~QImageWidget();

signals:
    void rect_was_entered(const cv::Rect &value);

public slots:
    void updateImage(const cv::Mat &image, qreal frame_period, quint32 pixels_enrolled); // takes cv::Mat image and converts it to the appropriate Qt QImage format
    void updatePointer(const qreal *pointer, quint16 length);  // updates pointer to data;
    void updateValues(qreal value1, qreal value2, bool flag); // use to update strings
    void updateBreathStrings(qreal breath_rate, qreal snr_value);
    void switchColorScheme(); // use to switch between black and white color of text on the image
    void set_warning_status(const char *input_string);
    void toggle_advancedvisualization(bool value); // interface to switch m_advancedvisualizationFlag
    void set_drawDataFlag(bool value); // read m_drawDataFlag comment
    void clearFrequencyString(qreal value);
    void clearBreathRateString(qreal value);
    void updadeMapRegion(const cv::Rect& input_rect);
    void updateMap(const qreal *pointer, quint32 width, quint32 height, qreal max, qreal min);
    void selectWholeImage();
    void clearMap();
    void setImageFlag(bool value);

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QImage qt_image;        // stores current QImage instance
    cv::Mat opencv_image;   // stores current cv::Mat instance
    QRect m_aimrect;        // stores current rectangle region on widget
    QString m_informationString;    // stores text information that will be drawn on the widget in paintEvent
    QString m_frequencyString;  // stores frequency
    QString m_snrString;    // stores SNR string value
    QString m_breathRateString;  // stores frequency
    QString m_breathSNRString;    // stores SNR string value
    quint16 x0;             // stores coordinate of mousePressEvenr
    quint16 y0;             // stores coordinate of mousePressEvent
    const qreal *pt_data;         // stores pointer to external data, wich is used to draw on this widget, point it to external data vector by menas of updatePointer(...) slot
    quint16 m_datalength;   // should be used ti store length of external vector
    QColor m_frequencyColor;     // stores the color of the m_frequencyString
    QColor m_informationColor;     // stores the color of all textual information on widget except the m_frequencyString
    QString m_warningString; // stores warning about event, when no regions are selected or no objects are detected
    quint16 m_margin;       // stores margin from left side of centered image
    QColor m_contourColor;   // stores color of text countour
    QColor m_fillColor;      // stores color of text background
    bool m_advancedvisualizationFlag; // turn it off by means of toggle_advancedvisualization(true) to make visualization quality higher, higher visualization quality costs about +20 ms/frame on Intel Pentium IV 3.0 on Debug mode
    bool m_drawDataFlag; // a flag, that manages if user want to draw or not dome data as trace on this widget, is controlled by set_drawDataFlag(bool ...)
    bool m_imageFlag; // controls qt_image draw function
    quint8 m_opacity;

    cv::Rect m_mapRect;
    quint16 m_mapCols;
    quint16 m_mapRows;
    qreal m_mapMax;
    qreal m_mapMin;
    qreal m_slope;
    qreal m_intercept;
    const qreal *v_map;
    QColor v_colors[256];

private slots:
    void computeColorTable(); // call in constructor to calculate appropriate colors and write them in v_colors[]
    inline QRect make_proportional_rect(QRect rect, int width, int height) const; // returns QRect inside input rect with the same center point, but with proportional sizes corresponding to width and height
    inline cv::Rect crop_aimrect() const;    // should be used for m_aimrect cropping
    inline QRectF findMapRegion(const QRect &viewRect) const;
    void drawStrings(QPainter &painter, const QRect &input_rect); // use this eunction inside paintEvent(...) handler to draw string on the image
    void drawData(QPainter &painter, const QRect &input_rect);   // draws pt_Data[] if ptData != NULL and drops pt_Data to NULL on every function call
    void drawMap(QPainter &painter, const QRect &input_rect);
};

//------------------------------------------------------------------------------------------------------

inline QRect QImageWidget::make_proportional_rect(QRect rect, int width, int height) const // returns QRect inside input rect with the same center point, but with proportional sizes corresponding to width and height
{
    if( (width > 0) && (height > 0) )
    {
        int old_width = rect.width();
        int old_height = rect.height();
        if( ((qreal)width/height) > ((qreal)old_width/old_height) )
        {
            rect.setHeight( old_width * height/width );
            rect.moveTop( (old_height - rect.height())/2 );
        }
        else
        {
            rect.setWidth( old_height * width/height );
            rect.moveLeft( (old_width - rect.width())/2 );
        }
    }
    return rect;
}

//------------------------------------------------------------------------------------------------------

inline cv::Rect QImageWidget::crop_aimrect() const
{
    QRect workfield = make_proportional_rect(this->rect(), opencv_image.cols, opencv_image.rows);
    QRect region = workfield.intersected( m_aimrect );
    quint16 output_x = ( (qreal)(region.x() - workfield.x())/workfield.width() ) * opencv_image.cols;
    quint16 output_y = ( (qreal)(region.y() - workfield.y())/workfield.height() ) * opencv_image.rows;
    quint16 output_w = ( (qreal)region.width()/workfield.width() ) * opencv_image.cols;
    quint16 output_h = ( (qreal)region.height()/workfield.height() ) * opencv_image.rows;
    cv::Rect output_rect(output_x, output_y, output_w, output_h);
    return output_rect;
}

//------------------------------------------------------------------------------------------------------

inline QRectF QImageWidget::findMapRegion(const QRect& viewRect) const
{
    qreal x = viewRect.x() + ((qreal)m_mapRect.x / opencv_image.cols) * viewRect.width();
    qreal y = viewRect.y() + ((qreal)m_mapRect.y / opencv_image.rows) * viewRect.height();
    qreal w = ((qreal)m_mapRect.width / opencv_image.cols) * viewRect.width();
    qreal h = ((qreal)m_mapRect.height / opencv_image.rows) * viewRect.height();
    return QRectF(x,y,w,h);
}

//------------------------------------------------------------------------------------------------------
#endif // QIMAGEWIDGET_H
