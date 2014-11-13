/*------------------------------------------------------------------------------------------------------
Taranov Alex, 2014									     SOURCE FILE
Class that wraps opencv functions into Qt SIGNAL/SLOT interface
The simplest way to use it - rewrite appropriate section in QOpencvProcessor::customProcess(...) slot
------------------------------------------------------------------------------------------------------*/

#include "qopencvprocessor.h"

#define OBJECT_MINSIZE 150
//------------------------------------------------------------------------------------------------------

QOpencvProcessor::QOpencvProcessor(QObject *parent):
    QObject(parent)
{
    //Initialization
    m_cvRect.width = 0;
    m_cvRect.height = 0;
    m_framePeriod = 0.0;
    m_fullFaceFlag = false;

    pt_map = NULL;
}

//-----------------------------------------------------------------------------------------------------

cv::Rect QOpencvProcessor::getRect()
{
    return m_cvRect;
}

//-----------------------------------------------------------------------------------------------------

void QOpencvProcessor::setMapRegion(const cv::Rect &input_rect)
{
    m_mapRect = input_rect;
}

//-----------------------------------------------------------------------------------------------------

void QOpencvProcessor::updateTime()
{
    m_timeCounter = cv::getTickCount();
}

//------------------------------------------------------------------------------------------------------

void QOpencvProcessor::customProcess(const cv::Mat &input)
{
    cv::Mat output(input); // Copy the header and pointer to data of input object
    cv::Mat temp; // temporary object

    //-------------CUSTOM ALGORITHM--------------

    //You can do here almost whatever you wnat...
    cv::cvtColor(output,temp,CV_BGR2GRAY);
    cv::equalizeHist(temp,temp);
    cv::Canny(temp,output,10,100);

    //----------END OF CUSTOM ALGORITHM----------

    //---------Drawing of rectangle--------------
    if( (m_cvRect.width > 0) && (m_cvRect.height > 0) )
    {
        cv::rectangle( output , m_cvRect, cv::Scalar(255,255,255)); // white color
    }

    //-------------Time measurement--------------
    m_framePeriod = (cv::getTickCount() -  m_timeCounter) * 1000.0 / cv::getTickFrequency(); // result is calculated in milliseconds
    m_timeCounter = cv::getTickCount();

    emit frameProcessed(output, m_framePeriod);
}

//------------------------------------------------------------------------------------------------------

void QOpencvProcessor::setRect(const cv::Rect &input_rect)
{
     m_cvRect = input_rect;
}

//------------------------------------------------------------------------------------------------------

bool QOpencvProcessor::loadClassifier(const std::string &filename)
{
    return m_classifier.load( filename );
}

//------------------------------------------------------------------------------------------------------
void QOpencvProcessor::faceProcess(const cv::Mat &input)
{
    cv::Mat output(input);  // Copy the header and pointer to data of input object
    cv::Mat gray; // Create an instance of cv::Mat for temporary image storage
    cv::cvtColor(output, gray, CV_BGR2GRAY);
    cv::equalizeHist(gray, gray);
    std::vector<cv::Rect> faces_vector;
    m_classifier.detectMultiScale(gray, faces_vector, 1.1, 11, cv::CASCADE_DO_ROUGH_SEARCH|cv::CASCADE_FIND_BIGGEST_OBJECT, cv::Size(OBJECT_MINSIZE, OBJECT_MINSIZE)); // Detect faces (list of flags CASCADE_DO_CANNY_PRUNING, CASCADE_DO_ROUGH_SEARCH, CASCADE_FIND_BIGGEST_OBJECT, CASCADE_SCALE_IMAGE )
    unsigned int X = 0; // the top-left corner horizontal coordinate of future rectangle
    unsigned int Y = 0; // the top-left corner vertical coordinate of future rectangle
    unsigned int rectwidth = 0; //...
    unsigned int rectheight = 0; //...
    unsigned long red = 0; // an accumulator for red color channel
    unsigned long green = 0; // an accumulator for green color channel
    unsigned long blue = 0; // an accumulator for blue color channel
    unsigned int dX = 0;
    unsigned int dY = 0;
    unsigned long area = 0;

    if(faces_vector.size() != 0) // if classifier find something, then do...
    {
        X = faces_vector[0].x; // take actual coordinate
        Y = faces_vector[0].y; // take actual coordinate
        rectwidth = faces_vector[0].width; // take actual size
        rectheight = faces_vector[0].height; // take actual size
        dX = (int)rectwidth/4; // the horizontal portion of rect domain that will be enrolled
        dY = (int)rectheight/13; //...
        unsigned char *p; // this pointer will be used to store adresses of the image rows

        if(m_fullFaceFlag == false)
        {
            if(output.channels() == 3)
            {
                for(unsigned int j = (Y + dY); j < (Y + 3*dY); j++)
                {
                    p = output.ptr(j); //takes pointer to beginning of data on row
                    for(unsigned int i = X + dX; i < X + rectwidth - dX; i++)
                    {
                        blue += p[3*i];
                        green += p[3*i+1];
                        red += p[3*i+2];
                        //Uncomment if want to see the enrolled domain on image
                        //p[3*i] = 0;
                        p[3*i+1] = 255;
                        //p[3*i+2] = 0;
                    }
                }
                for(unsigned int j = (Y + 6*dY); j < (Y + 9*dY); j++)
                {
                    p = output.ptr(j);//pointer to beginning of data on rows
                    for(unsigned int i = (X + dX); i < (X + rectwidth - dX); i++)
                    {
                        blue += p[3*i];
                        green += p[3*i+1];
                        red += p[3*i+2];
                        //Uncomment if want to see the enrolled domain on image
                        //p[3*i] = 0;
                        p[3*i+1] = 255;
                        //p[3*i+2] = 0;
                    }
                }
            }
            else
            {
                for(unsigned int j = (Y + dY); j < (Y + 3*dY); j++)
                {
                    p = output.ptr(j);//pointer to beginning of data on rows
                    for(unsigned int i = (X + dX); i < (X + rectwidth - dX); i++)
                    {
                        green += p[i];
                        //Uncomment if want to see the enrolled domain on image
                        //p[i] = 0;
                    }
                }
                for(unsigned int j = (Y + 6*dY); j < (Y + 9*dY); j++)
                {
                    p = output.ptr<unsigned char>(j);//pointer to beginning of data on rows
                    for(unsigned int i = (X + dX); i < (X + rectwidth - dX); i++)
                    {
                        green += p[i];
                        //Uncomment if want to see the enrolled domain on image
                        //p[i] = 0;
                    }
                }
                blue = green;
                red = green;
            }
            area = (rectwidth - 2*dX)*(3*dY);
        }
        else
        {
            if(output.channels() == 3)
            {
                for(unsigned int j = Y; j < Y + rectheight; j++)
                {
                    p = output.ptr(j); //takes pointer to beginning of data on row
                    for(unsigned int i = X; i < X + rectwidth; i++)
                    {
                        blue += p[3*i];
                        green += p[3*i+1];
                        red += p[3*i+2];
                        //Uncomment if want to see the enrolled domain on image
                        //p[3*i] = 0;
                        p[3*i+1] = 255;
                        //p[3*i+2] = 0;
                    }
                }
            }
            else
            {
                for(unsigned int j = Y; j < Y + rectheight; j++)
                {
                    p = output.ptr(j);//pointer to beginning of data on rows
                    for(unsigned int i = X; i < X + rectwidth; i++)
                    {
                        green += p[i];
                        //Uncomment if want to see the enrolled domain on image
                        //p[i] = 0;
                    }
                }
                blue = green;
                red = green;
            }
            area = rectwidth*rectheight;
        }
    }


    //-----end of if(faces_vector.size() != 0)-----
    m_framePeriod = ((double)cv::getTickCount() -  m_timeCounter)*1000.0 / cv::getTickFrequency();
    m_timeCounter = cv::getTickCount();
    if(faces_vector.size() != 0)
    {
        cv::rectangle( output, faces_vector[0] , cv::Scalar(255,25,25));
        emit dataCollected( red , green, blue, area, m_framePeriod);
    }
    else
    {
        if(m_classifier.empty())
        {
            emit selectRegion("Load cascade for detection");
        }
        else
        {
            emit selectRegion("Come closer, please");
        }
    }
    emit frameProcessed(output, m_framePeriod);
}

//------------------------------------------------------------------------------------------------

void QOpencvProcessor::rectProcess(const cv::Mat &input)
{
    cv::Mat output(input); //Copy constructor
    unsigned int rectwidth = m_cvRect.width;
    unsigned int rectheight = m_cvRect.height;
    unsigned int X = m_cvRect.x;
    unsigned int Y = m_cvRect.y;

    if( (output.rows <= (Y + rectheight)) || (output.cols <= (X + rectwidth)) )
    {
        rectheight = 0;
        rectwidth = 0;
    }

    unsigned long red = 0;
    unsigned long green = 0;
    unsigned long blue = 0;
    //-------------------------------------------------------------------------
    if((rectheight > 0) && (rectwidth > 0))
    {
        unsigned char *p; // a pointer to store the adresses of image rows
        if(output.channels() == 3)
        {
            for(unsigned int j = Y; j < Y + rectheight; j++)
            {
                p = output.ptr(j); //takes pointer to beginning of data on rows
                for(unsigned int i = X; i < X + rectwidth; i++)
                {
                    blue += p[3*i];
                    green += p[3*i+1];
                    red += p[3*i+2];
                            //p[3*i] = 0;
                            //p[3*i+1] = 0;
                            //p[3*i+2] = 0;
                }
            }
        }
        else
        {
            for(unsigned int j = Y; j < Y + rectheight; j++)
            {
                p = output.ptr(j);//pointer to beginning of data on rows
                for(unsigned int i = X; i < X + rectwidth; i++)
                {
                    green += p[i];
                        //p[i] = 0;
                }
            }
        }
    }
    //------end of if((rectheight > 0) && (rectwidth > 0))
    m_framePeriod = ((double)cv::getTickCount() -  m_timeCounter)*1000.0 / cv::getTickFrequency();
    m_timeCounter = cv::getTickCount();
    if((rectheight > 0) && (rectwidth > 0))
    {
        cv::rectangle( output , m_cvRect, cv::Scalar(255,25,25));
        emit dataCollected(red, green, blue, rectwidth*rectheight, m_framePeriod);
    }
    else
    {
        emit selectRegion("Select region on image");
    }
    emit frameProcessed(output, m_framePeriod);
}

//-----------------------------------------------------------------------------------------------

void QOpencvProcessor::setFullFaceFlag(bool value)
{
    m_fullFaceFlag = value;
}

//------------------------------------------------------------------------------------------------


void QOpencvProcessor::mapProcess(const cv::Mat &input)
{
    cv::Mat output(input);
    int X = m_mapRect.x;
    int Y = m_mapRect.y;
    int W = m_mapRect.width;
    int H = m_mapRect.height;

    int stepsY = H / m_mapCellSizeY;
    int stepsX = W / m_mapCellSizeX;
    unsigned char *pixelSet[m_mapCellSizeY];
    int area = m_mapCellSizeY*m_mapCellSizeX;

    unsigned long sumRed = 0;
    unsigned long sumBlue = 0;
    unsigned long sumGreen = 0;

    int performance_pill;

    if(input.channels() == 3)
    {
        for(int i = 0; i < stepsY; i++)
        {
            for(int p = 0; p < m_mapCellSizeY; p++)
            {
                pixelSet[p] = output.ptr(Y + i*m_mapCellSizeY + p);
            }
            for(int j = 0; j < stepsX; j++)
            {
                for(int k = 0; k < m_mapCellSizeX; k++)
                {
                    performance_pill = 3*(X + j*m_mapCellSizeX + k);
                    for(int p = 0; p < m_mapCellSizeY; p++)
                    {
                        sumBlue += pixelSet[p][performance_pill];
                        sumGreen += pixelSet[p][performance_pill + 1];
                        sumRed += pixelSet[p][performance_pill + 2];
                    }
                }
                emit mapCellProcessed(sumRed, sumGreen, sumBlue, area, m_framePeriod);
                sumBlue = 0;
                sumGreen = 0;
                sumRed = 0;
            }
        }
    }
    else
    {
        for(int i = 0; i < stepsY; i++)
        {
            for(int p = 0; p < m_mapCellSizeY; p++)
            {
                pixelSet[p] = output.ptr(Y + i*m_mapCellSizeY + p);
            }
            for(int j = 0; j < stepsX; j++)
            {
                for(int k = 0; k < m_mapCellSizeX; k++)
                {
                    performance_pill = X + j*m_mapCellSizeX + k;
                    for(int p = 0; p < m_mapCellSizeY; p++)
                    {
                        sumBlue += pixelSet[p][performance_pill];
                    }
                }
                emit mapCellProcessed(sumBlue, sumBlue, sumBlue, area, m_framePeriod);
                sumBlue = 0;
            }
        }
    }
}

void QOpencvProcessor::setMapCellSize(quint16 sizeX, quint16 sizeY)
{
    m_mapCellSizeX = sizeX;
    m_mapCellSizeY = sizeY;
}

void QOpencvProcessor::updateMap(const qreal *pointer, quint32 width, quint32 height, qreal max, qreal min)
{
    qWarning("Map was updated!");
}
