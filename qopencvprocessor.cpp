#include "qopencvprocessor.h"

//------------------------------------------------------------------------------------------------------

QOpencvprocessor::QOpencvprocessor(QObject *parent) : QObject(parent)
{
    rect.width = 0; // explicit zero initialization
}

//-----------------------------------------------------------------------------------------------------

QOpencvprocessor::~QOpencvprocessor()
{

}

//------------------------------------------------------------------------------------------------------

bool QOpencvprocessor::loadclassifier(const std::string& filename)
{
    if(classifier.load(filename))
    {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------

void QOpencvprocessor::actualizetimecounter()
{
    timecounter = cv::getTickCount();
}

//------------------------------------------------------------------------------------------------------

void QOpencvprocessor::process_custom_rule(const cv::Mat &input)
{
    //maybe Mat::clone(input) should be here...
    cv::Mat temp;
    cv::Mat output;
    cv::cvtColor(input,temp,CV_BGR2GRAY);
    cv::equalizeHist(temp,temp);
    cv::Canny(temp,output,10,100);
    emit frame_was_processed(output);
}

//------------------------------------------------------------------------------------------------------

void QOpencvprocessor::process_face_pulse(const cv::Mat& input_image)
{
        cv::Mat output_image = input_image.clone(); //Explicitly clone an input data matrix, if undo that there is memory collisions may ocur between current thread and mainwindow
        cv::Mat gray_image;
        cv::cvtColor(output_image, gray_image, CV_BGR2GRAY);
        cv::equalizeHist(gray_image, gray_image);

        std::vector<cv::Rect> faces_vector;
        //-- Detect faces (list of flags CASCADE_DO_CANNY_PRUNING, CASCADE_DO_ROUGH_SEARCH, CASCADE_FIND_BIGGEST_OBJECT, CASCADE_SCALE_IMAGE )
        classifier.detectMultiScale(gray_image ,faces_vector ,1.1 ,7 , cv::CASCADE_DO_ROUGH_SEARCH|cv::CASCADE_FIND_BIGGEST_OBJECT ,cv::Size(MIN_WIDTH_FOR_FACE_PROJECTION,MIN_WIDTH_FOR_FACE_PROJECTION));

        //-------------------------------------------------------------------------

        unsigned int channels = output_image.channels();
        unsigned int X = 0;
        unsigned int Y = 0;
        unsigned int rectwidth = 0;
        unsigned int rectheight = 0;
        unsigned long red = 0;
        unsigned long green = 0;
        unsigned long blue = 0;
        unsigned int dX = 0;
        unsigned int dY = 0;

       if(faces_vector.size() != 0)
        {
            unsigned char *p;

            X = faces_vector[0].x;  //the top-left corner, as well as width and height of the rectangle
            Y = faces_vector[0].y;
            rectwidth = faces_vector[0].width;
            rectheight = faces_vector[0].height;
            dX = (int)rectwidth/4;
            dY = (int)rectheight/13;

            if(channels == 3)
            {
                for(unsigned int j = (Y + dY); j < (Y + 3*dY); j++)
                {
                    p = output_image.ptr(j);//point to beginning of data on rows
                    for(unsigned int i = X + dX; i < X + rectwidth - dX; i++)
                    {
                        blue += p[3*i];
                        green += p[3*i+1];
                        red += p[3*i+2];
                        //p[3*i] = 0;
                        //p[3*i+1] = 0;
                        //p[3*i+2] = 0;
                    }
                }
                for(unsigned int j = (Y + 6*dY); j < (Y + 9*dY); j++)
                {
                    p = output_image.ptr<unsigned char>(j);//pointer to beginning of data on rows
                    for(unsigned int i = (X + dX); i < (X + rectwidth - dX); i++)
                    {
                        blue += p[3*i];
                        green += p[3*i+1];
                        red += p[3*i+2];
                        /*p[3*i] = 0;
                        p[3*i+1] = 0;
                        p[3*i+2] = 0;*/
                    }
                }
            }
            else
            {
                for(unsigned int j = (Y + dY); j < (Y + 3*dY); j++)
                {
                    p = output_image.ptr<unsigned char>(j);//pointer to beginning of data on rows
                    for(unsigned int i = (X + dX); i < (X + rectwidth - dX); i++)
                    {
                        green += p[i];
                        //p[i] = 0;
                    }
                }
                for(unsigned int j = (Y + 6*dY); j < (Y + 9*dY); j++)
                {
                    p = output_image.ptr<unsigned char>(j);//pointer to beginning of data on rows
                    for(unsigned int i = (X + dX); i < (X + rectwidth - dX); i++)
                    {
                        green += p[i];
                        //p[i] = 0;
                    }
                }
            }
        } // end of if(faces_vector.length() != 0)

        //-------------------------------------------------------------------------

        for (unsigned int i = 0; i < faces_vector.size(); i++ )
        {
            cv::rectangle( output_image , faces_vector[i] , cv::Scalar(0,0,255));
        }

        frametime = ((double)cv::getTickCount() -  timecounter)*1000/cv::getTickFrequency();
        timecounter = cv::getTickCount();
        cv::putText(output_image, ( std::to_string((long double)frametime) + " ms" ), cv::Point(output_image.cols/2,25), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar::all(0));
        cv::putText(output_image, ( std::to_string((long double)output_image.cols) + " x " + std::to_string((long double)output_image.rows)), cv::Point(output_image.cols/2,50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar::all(0));

        if(faces_vector.size() != 0)
        {
            emit values_were_evaluated(channels, red , green, blue, (rectwidth - 2*dX)*(5*dY), frametime);
        }
        emit frame_was_processed(output_image);
}

//------------------------------------------------------------------------------------------------------

void QOpencvprocessor::process_rectregion_pulse(const cv::Mat& input_image)
{
        cv::Mat output_image = input_image.clone(); //Explicitly clone an input data matrix, if undo that there is memory collisions may ocur between current thread and mainwindow

        unsigned int channels = output_image.channels();
        unsigned int X = rect.x;
        unsigned int Y = rect.y;
        unsigned int rectwidth = rect.width;
        unsigned int rectheight = rect.height;
        unsigned long red = 0;
        unsigned long green = 0;
        unsigned long blue = 0;

        //-------------------------------------------------------------------------
        if((rectheight > 0) && (rectwidth > 0))
        {
            unsigned char *p;
            if(channels == 3)
            {
                for(unsigned int j = Y; j < Y + rectheight; j++)
                {
                    p = output_image.ptr(j);//point to beginning of data on rows
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
                    p = output_image.ptr<unsigned char>(j);//pointer to beginning of data on rows
                    for(unsigned int i = X; i < X + rectwidth; i++)
                    {
                        green += p[i];
                        //p[i] = 0;
                    }
                }
            }
        } //end of if((rectheight > 0) && (rectwidth > 0))
        //-------------------------------------------------------------------------

        cv::rectangle( output_image , rect, cv::Scalar(125,255,125));

        frametime = ((double)cv::getTickCount() -  timecounter)*1000/cv::getTickFrequency();
        timecounter = cv::getTickCount();
        cv::putText(output_image, ( std::to_string((long double)frametime) + " ms" ), cv::Point(25, output_image.rows/2), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar::all(50));
        cv::putText(output_image, ( std::to_string((long double)rectwidth) + " x " + std::to_string((long double)rectheight)), cv::Point(25, output_image.rows/2 - 25), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar::all(50));

        if((rectheight > 0) && (rectwidth > 0))
        {
            emit values_were_evaluated(channels, red , green, blue, rectwidth*rectheight, frametime);
        }
        emit frame_was_processed(output_image);
}

//------------------------------------------------------------------------------------------------------

void QOpencvprocessor::actualizerect(const cv::Rect &input_rect)
{
     rect = input_rect;
}

//------------------------------------------------------------------------------------------------------
