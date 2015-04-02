#ifndef QHARMONICPROCESSOR_H
#define QHARMONICPROCESSOR_H

#include <QObject>
#include "fftw3.h"
#include "ap.h" // ALGLIB types
#include "dataanalysis.h" // ALGLIB functions

#define BOTTOM_LIMIT 0.7 // in s^-1, it is 42 bpm
#define TOP_LIMIT 4.5 // in s^-1, it is 270 bpm
#define SNR_TRESHOLD 2.0 // in most cases this value is suitable when (m_BufferLength == 256)
#define HALF_INTERVAL 2 // defines the number of averaging indexes when frequency is evaluated, this value should be >= 1
#define DIGITAL_FILTER_LENGTH 5 // in counts
#define MEAN_INTERVAL 16 // should be greater than one, but less than m_datalength


class QHarmonicProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QHarmonicProcessor(QObject *parent = NULL, quint16 length_of_data = 256, quint16 length_of_buffer = 256 );
    ~QHarmonicProcessor();
    enum ColorChannel { Red, Green, Blue, RGB, Experimental };
    enum XMLparserError { NoError, FileOpenError, FileExistanceError, ReadError, ParseFailure };
    enum SexID { Male, Female };
    enum TwoSideAlpha { FiftyPercents, TwentyPercents, TenPercents, FivePercents, TwoPercents };

signals:
    void SignalUpdated(const qreal * pointer_to_vector, quint16 length_of_vector);
    void SpectrumUpdated(const qreal * pointer_to_vector, quint16 length_of_vector);
    void TimeUpdated(const qreal * pointer_to_vector, quint16 length_of_vector);
    void HeartRateUpdated(qreal freq_value, qreal snr_value, bool reliable_data_flag);
    void PCAProjectionUpdated(const qreal * ppointer_to_vector, quint16 length_of_vector);
    void BinaryOutputUpdated(const qreal *pointer_to_vector, quint16 length_of_vector);
    void CurrentValues(qreal signalValue, qreal meanRed, qreal meanGreen, qreal meanBlue, qreal freqValue, qreal snrValue);
    void TooNoisy(qreal snr_value);

    void snrUpdated(quint32 id, qreal value);   // signal for mapping
    void vpgUpdated(quint32 id, qreal value);       // signal for mapping
    void svpgUpdated(quint32 id, qreal value);      // signal for mapping
    void bvpgUpdated(quint32 id, qreal value);      // signal for mapping

public slots:
    void EnrollData(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double time);
    void ComputeFrequency(); // use FFT algorithm for HeartRate evaluation
    void CountFrequency(); // use simple count algorithm on v_BinaryOutput for HeartRate evaluation
    void setPCAMode(bool value); // controls PCA alignment
    void switchColorMode(int value); // controls colors enrollment
    int  loadWarningRates(const char *fileName, SexID sex, int age, TwoSideAlpha alpha);
    void setID(quint32 value); // use it to set ID, it is used for QHarmonicMapper internal logic management
    void setEstiamtionInterval(int value); // use it to set m_estimationInterval property value
    unsigned int getDataLength() const;
    unsigned int getBufferLength() const;
    unsigned int getEstimationInterval() const;
    void setSnrControl(bool value);

private:
    qreal *v_Signal;  //a pointer to centered and normalized data (typedefinition from fftw3.h, a single precision complex float number type)
    qreal *v_Input; // a pointer to input counts history, for digital filtration
    fftw_complex *v_Spectrum;  // a pointer to an array for FFT-spectrum
    qreal m_SNR; // a variable for signal-to-noise ratio estimation storing
    qreal *v_RawCh1; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    qreal *v_RawCh2; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    qreal *v_ForFFT; //a pointer to data prepared for FFT
    qreal *v_Amplitude; // stores amplitude spectrum
    qreal m_MeanCh1;   //a variable for mean value in channel1 storing
    qreal m_MeanCh2;   //a variable for mean value in channel2 storing
    qreal *v_Time; //a pointer to an array for frame periods storing (values in milliseconds thus unsigned int)
    qreal m_HeartRate; //a variable for storing a last evaluated frequency of the 'strongest' harmonic
    unsigned int curpos; //a current position I meant
    unsigned int m_DataLength; //a length of data array
    unsigned int m_BufferLength; //a lenght of sub data array for FFT (m_BufferLength should be <= m_DataLength)
    bool f_PCA; // this flag controls whether ComputeFrequency use ordinary computation or PCA alignment, value is controlled by set_f_PCA(...)
    fftw_plan m_plan; // a plan for FFT evaluation
    ColorChannel m_channel; // determines which color channel is enrolled by WriteToDataOneColor(...) method
    qreal *v_BinaryOutput; // a pointer to a vector of digital filter output
    qreal *v_SmoothedSignal; // for intermediate result storage
    qreal v_Derivative[2]; // to store two close counts from digital derivative
    quint8 m_zerocrossing; // controls zero crossings of the first derivative
    qint16 m_PulseCounter; // will store the number of pulse waves for averaging m_HeartRate estimation
    double m_leftThreshold; // a bottom threshold for warning about high pulse value
    double m_rightTreshold; // a top threshold for warning aboul low pulse value
    qreal m_output; // a variable for v_BinaryOutput control, it should take values 1.0 or -1.0

    alglib::real_2d_array PCA_RAW_RGB; // a container for PCA analysis
    alglib::real_1d_array PCA_Variance; // array[0..2] - variance values corresponding to basis vectors
    alglib::real_2d_array PCA_Basis; // array[0..2,0..2], whose columns will store basis vectors
    alglib::ae_int_t PCA_Info; // PCA result code

    quint16 loop(qint16) const; //a function that return a loop-index
    quint16 loopInput(qint16) const; //a function that return a loop-index
    quint16 loopBuffer(qint16) const; //a function that return a loop-index
    quint8 loopOnTwo(qint16 difference) const;

    quint32 m_ID;
    quint16 m_estimationInterval; // stores the number of counts that will be used to evaluate mean and sko estimations
    bool m_snrControlFlag;
};

// inline, for speed, must therefore reside in header file
inline quint16 QHarmonicProcessor::loop(qint16 difference) const
{
    return ((m_DataLength + (difference % m_DataLength)) % m_DataLength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
inline quint16 QHarmonicProcessor::loopInput(qint16 difference) const
{
    return ((DIGITAL_FILTER_LENGTH + (difference % DIGITAL_FILTER_LENGTH)) % DIGITAL_FILTER_LENGTH);
}
//---------------------------------------------------------------------------
inline quint16 QHarmonicProcessor::loopBuffer(qint16 difference) const
{
    return ((m_BufferLength + (difference % m_BufferLength)) % m_BufferLength);
}
//---------------------------------------------------------------------------
inline quint8 QHarmonicProcessor::loopOnTwo(qint16 difference) const
{
    return ((2 + (difference % 2)) % 2);
}

//---------------------------------------------------------------------------
#endif // QHARMONICPROCESSOR_H
