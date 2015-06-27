#include "qharmonicprocessor.h"
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QFile>

//----------------------------------------------------------------------------------------------------------
QHarmonicProcessor::QHarmonicProcessor(QObject *parent, quint16 length_of_data, quint16 length_of_buffer) :
    QObject(parent),
    m_DataLength(length_of_data),
    m_BufferLength(length_of_buffer),
    curpos(0),
    m_HeartSNR(-5.0),
    m_HeartRate(0.0),
    m_BreathRate(0.0),
    m_BreathSNR(-5.0),
    f_PCA(false),
    m_ColorChannel(Green),
    m_zerocrossing(0),
    m_PulseCounter(4),
    m_leftThreshold(60),
    m_rightTreshold(85),
    m_output(1.0),
    m_ID(0),
    m_estimationInterval(DEFAULT_NORMALIZATION_INTERVAL),
    m_HeartSNRControlFlag(false),
    m_BreathStrobe(DEFAULT_BREATH_STROBE),
    m_BreathStrobeCounter(0),
    m_BreathCurpos(0),
    m_BreathAverageInterval(DEFAULT_BREATH_AVERAGE),
    m_BreathCNInterval(DEFAULT_BREATH_NORMALIZATION_INTERVAL),
    m_pruningFlag(false)
{
    // Memory allocation
    v_RawCh1 = new qreal[m_DataLength];
    v_RawCh2 = new qreal[m_DataLength];
    v_HeartSignal = new qreal[m_DataLength];
    v_HeartTime = new qreal[m_DataLength];
    v_HeartCNSignal = new qreal[DIGITAL_FILTER_LENGTH];
    v_HeartForFFT = new qreal[m_BufferLength];
    v_HeartSpectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (m_BufferLength/2 + 1));
    v_HeartAmplitude = new qreal[m_BufferLength/2 + 1];
    m_HeartPlan = fftw_plan_dft_r2c_1d(m_BufferLength, v_HeartForFFT, v_HeartSpectrum, FFTW_ESTIMATE);
    v_BinaryOutput = new qreal[m_DataLength];
    v_SmoothedSignal = new qreal[DIGITAL_FILTER_LENGTH];

    v_RawBreathSignal = new qreal[m_DataLength];
    v_BreathSignal = new qreal[m_DataLength];
    v_BreathTime = new qreal[m_DataLength];
    v_BreathForFFT = new qreal[m_BufferLength];
    v_BreathAmplitude = new qreal[m_BufferLength/2 + 1];
    v_BreathSpectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (m_BufferLength/2 + 1));
    m_BreathPlan = fftw_plan_dft_r2c_1d(m_BufferLength, v_BreathForFFT, v_BreathSpectrum, FFTW_ESTIMATE);;

    // Vectors initialization
    for (quint16 i = 0; i < m_DataLength; i++)
    {
        v_RawCh1[i] = 0.0; // it should be equal to zero at start
        v_RawCh2[i] = 0.0; // it should be equal to zero at start
        v_HeartTime[i] = 35.0; // just for ensure that at the begining there is not any "division by zero"
        v_BreathTime[i] = 35.0;
        v_RawBreathSignal[i]= 0.0;
        v_BreathSignal[i] = 0.0;
        v_HeartSignal[i] = 0.0;
        if(i % 4)
        {
            v_BinaryOutput[i] = 1.0;
        }
        else
        {
            v_BinaryOutput[i] = -1.0;
        }
    }

    for(quint16 i = 0; i < DIGITAL_FILTER_LENGTH; i++)
    {
        v_HeartCNSignal[i] = 0.0;
    }

    // Memory allocation block for ALGLIB arrays
    PCA_RAW_RGB.setlength(m_BufferLength, 3); // 3 because RED, GREEN and BLUE colors represent 3 independent variables
    PCA_Variance.setlength(3);
    PCA_Basis.setlength(3, 3);
    PCA_Info = 0;
}

//----------------------------------------------------------------------------------------------------------

QHarmonicProcessor::~QHarmonicProcessor()
{
    fftw_destroy_plan(m_HeartPlan);
    delete[] v_RawCh1;
    delete[] v_RawCh2;
    delete[] v_HeartSignal;
    delete[] v_HeartTime;
    delete[] v_HeartCNSignal;
    delete[] v_HeartForFFT;
    fftw_free(v_HeartSpectrum);
    delete[] v_HeartAmplitude;
    delete[] v_BinaryOutput;
    delete[] v_SmoothedSignal;  

    fftw_destroy_plan(m_BreathPlan);
    delete[] v_RawBreathSignal;
    delete[] v_BreathSignal;
    delete[] v_BreathTime;
    delete[] v_BreathForFFT;
    delete[] v_BreathAmplitude;
    fftw_free(v_BreathSpectrum);
}

//----------------------------------------------------------------------------------------------------------

void QHarmonicProcessor::EnrollData(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double time)
{

    const quint16 pos = loopBuffer(curpos);   //a variable for position storing

    qreal m_MeanCh1 = 0.0;    //a variable for mean value in channel1 storing
    qreal m_MeanCh2 = 0.0;    //a variable for mean value in channel2 storing
    qreal m_MeanCh3 = 0.0;
    quint16 temp_pos;

    PCA_RAW_RGB(pos, 0) = (qreal)red / area;
    PCA_RAW_RGB(pos, 1) = (qreal)green / area;
    PCA_RAW_RGB(pos, 2) = (qreal)blue / area;

    //color pruning block, based on statistics
    if(m_pruningFlag)
    {
        for(quint16 i = 0; i < m_estimationInterval; i++)
        {
            temp_pos = loopBuffer(curpos - i);
            m_MeanCh1 += PCA_RAW_RGB(temp_pos, 0);
            m_MeanCh2 += PCA_RAW_RGB(temp_pos, 1);
            m_MeanCh3 += PCA_RAW_RGB(temp_pos, 2);
        }
        m_MeanCh1 /= m_estimationInterval;
        m_MeanCh2 /= m_estimationInterval;
        m_MeanCh3 /= m_estimationInterval;
        qreal sko1 = 0.0;
        qreal sko2 = 0.0;
        qreal sko3 = 0.0;
        for(quint16 i = 0; i < m_estimationInterval; i++)
        {
            temp_pos = loopBuffer(curpos - i);
            sko1 += (PCA_RAW_RGB(temp_pos, 0) - m_MeanCh1)*(PCA_RAW_RGB(temp_pos, 0) - m_MeanCh1);
            sko2 += (PCA_RAW_RGB(temp_pos, 1) - m_MeanCh2)*(PCA_RAW_RGB(temp_pos, 1) - m_MeanCh2);
            sko3 += (PCA_RAW_RGB(temp_pos, 2) - m_MeanCh3)*(PCA_RAW_RGB(temp_pos, 2) - m_MeanCh3);
        }
        sko1 = sqrt(sko1 / (m_estimationInterval - 1));
        sko2 = sqrt(sko2 / (m_estimationInterval - 1));
        sko3 = sqrt(sko3 / (m_estimationInterval - 1));
        if( ((PCA_RAW_RGB(pos, 0) - m_MeanCh1) < -PRUNING_SKO_COEFF*sko1) || ((PCA_RAW_RGB(pos, 0) - m_MeanCh1) > PRUNING_SKO_COEFF*sko1) )
            PCA_RAW_RGB(pos, 0) = m_MeanCh1;
        if( ((PCA_RAW_RGB(pos, 1) - m_MeanCh2) < -PRUNING_SKO_COEFF*sko2) || ((PCA_RAW_RGB(pos, 1) - m_MeanCh2) > PRUNING_SKO_COEFF*sko2) )
            PCA_RAW_RGB(pos, 1) = m_MeanCh2;
        if( ((PCA_RAW_RGB(pos, 2) - m_MeanCh3) < -PRUNING_SKO_COEFF*sko3) || ((PCA_RAW_RGB(pos, 2) - m_MeanCh3) > PRUNING_SKO_COEFF*sko3) )
            PCA_RAW_RGB(pos, 2) = m_MeanCh3;
    }


    if(m_ColorChannel == RGB) {

        v_RawCh1[curpos] = PCA_RAW_RGB(pos, 0) - PCA_RAW_RGB(pos, 1);
        v_RawCh2[curpos] = PCA_RAW_RGB(pos, 0) + PCA_RAW_RGB(pos, 1) - 2 * PCA_RAW_RGB(pos, 2);

        m_MeanCh1 = 0.0;
        m_MeanCh2 = 0.0;
        for(quint16 i = 0; i < m_estimationInterval; i++)
        {
            temp_pos = loop(curpos - i);
            m_MeanCh1 += v_RawCh1[temp_pos];
            m_MeanCh2 += v_RawCh2[temp_pos];
        }
        m_MeanCh1 /= m_estimationInterval;
        m_MeanCh2 /= m_estimationInterval;

        qreal ch1_sko = 0.0;
        qreal ch2_sko = 0.0;
        for (unsigned int i = 0; i < m_estimationInterval; i++)
        {
            temp_pos = loop(curpos - i);
            ch1_sko += (v_RawCh1[temp_pos] - m_MeanCh1)*(v_RawCh1[temp_pos] - m_MeanCh1);
            ch2_sko += (v_RawCh2[temp_pos] - m_MeanCh2)*(v_RawCh2[temp_pos] - m_MeanCh2);
        }
        ch1_sko = sqrt(ch1_sko / (m_estimationInterval - 1));
        if(ch1_sko < 0.01)
            ch1_sko = 1.0;
        ch2_sko = sqrt(ch2_sko / (m_estimationInterval - 1));
        if(ch2_sko < 0.01)
            ch2_sko = 1.0;
        v_HeartCNSignal[loopInput(curpos)] = (v_RawCh1[curpos] - m_MeanCh1) / ch1_sko  - (v_RawCh2[curpos] - m_MeanCh2) / ch2_sko;

    } else if(m_ColorChannel == Experimental) {

        v_RawCh1[curpos] = PCA_RAW_RGB(pos, 1);

        m_MeanCh1 = 0.0;
        for(quint16 i = 0; i < m_estimationInterval; i++)
        {
            m_MeanCh1 += v_RawCh1[loop(curpos - i)];
        }
        m_MeanCh1 /= m_estimationInterval;

        v_HeartCNSignal[loopInput(curpos)] = (v_RawCh1[curpos] - m_MeanCh1);

    } else {

        switch(m_ColorChannel) {
            case Red:
                v_RawCh1[curpos] = PCA_RAW_RGB(pos, 0);
                break;
            case Green:
                v_RawCh1[curpos] = PCA_RAW_RGB(pos, 1);
                break;
            case Blue:
                v_RawCh1[curpos] = PCA_RAW_RGB(pos, 2);
                break;
        }

        m_MeanCh1 = 0.0;
        for(quint16 i = 0; i < m_estimationInterval; i++)
        {
            m_MeanCh1 += v_RawCh1[loop(curpos - i)];
        }
        m_MeanCh1 /= m_estimationInterval;

        qreal ch1_sko = 0.0;
        for (quint16 i = 0; i < m_estimationInterval; i++)
        {
            temp_pos = loop(curpos - i);
            ch1_sko += (v_RawCh1[temp_pos] - m_MeanCh1)*(v_RawCh1[temp_pos] - m_MeanCh1);
        }
        ch1_sko = sqrt(ch1_sko / (m_estimationInterval - 1));
        if(ch1_sko < 0.01)
            ch1_sko = 1.0;
        v_HeartCNSignal[loopInput(curpos)] = (v_RawCh1[curpos] - m_MeanCh1)/ ch1_sko;
    }

    v_HeartTime[curpos] = time;
    emit TimeUpdated(v_HeartTime, m_DataLength);
    //v_HeartSignal[curpos] = ( v_HeartCNSignal[loopInput(curpos)] + v_HeartSignal[loop(curpos - 1)] ) / 2.0;
    v_HeartSignal[curpos] = ( v_HeartCNSignal[loopInput(curpos)] + v_HeartCNSignal[loopInput(curpos - 1)] + v_HeartCNSignal[loopInput(curpos - 2)] + v_HeartSignal[loop(curpos - 1)] ) / 4.0;
    emit heartSignalUpdated(v_HeartSignal, m_DataLength);

    ///------------------------------------------Breath signal part-------------------------------------------
    v_BreathTime[m_BreathCurpos] += time;
    m_BreathStrobeCounter =  (++m_BreathStrobeCounter) % m_BreathStrobe;
    if(m_BreathStrobeCounter ==  0)
    {
        ///Averaging from VPG
        m_MeanCh1 = 0.0;
        for(quint16 i = 0; i < m_BreathAverageInterval; i++)
        {
            m_MeanCh1 += v_RawCh1[loop(curpos - i)];
        }
        v_RawBreathSignal[m_BreathCurpos] = m_MeanCh1 / m_BreathAverageInterval;

        ///Centering and normalization
        m_MeanCh1 = 0.0;
        for(quint16 i = 0; i < m_BreathCNInterval; i++)
        {
            m_MeanCh1 += v_RawBreathSignal[loop(m_BreathCurpos - i)];
        }
        m_MeanCh1 /= m_BreathCNInterval;
        qreal temp_sko = 0.0;
        for(quint16 i = 0; i < m_BreathCNInterval; i++)
        {
            temp_sko += (v_RawBreathSignal[loop(m_BreathCurpos - i)] - m_MeanCh1)*(v_RawBreathSignal[loop(m_BreathCurpos - i)] - m_MeanCh1);
        }
        temp_sko = sqrt(temp_sko / (m_BreathCNInterval - 1 ) );
        if(temp_sko < 0.01)
            temp_sko = 1.0;
        v_BreathSignal[m_BreathCurpos] = ((( v_RawBreathSignal[m_BreathCurpos] - m_MeanCh1 ) / temp_sko) + v_BreathSignal[loop(m_BreathCurpos - 1)] ) / 2.0;
        //v_BreathSignal[m_BreathCurpos] = (v_RawBreathSignal[m_BreathCurpos] - m_MeanCh1 ) / temp_sko;
        emit breathSignalUpdated(v_BreathSignal, m_DataLength);
        m_BreathCurpos = (++m_BreathCurpos) % m_DataLength;
        v_BreathTime[m_BreathCurpos] = 0.0;
    }
    ///--------------------------------------------End of breath signal part-------------------------------------------------

    qreal outputValue = 0.0;
    for(quint16 i = 0; i < DIGITAL_FILTER_LENGTH ; i++)
    {
        outputValue += v_HeartCNSignal[i];
    }
    v_SmoothedSignal[loopInput(curpos)] = outputValue / DIGITAL_FILTER_LENGTH;  
    v_Derivative[loopOnTwo(curpos)] = v_SmoothedSignal[loopInput(curpos)] - v_SmoothedSignal[loopInput(curpos - 1)];
    if( (v_Derivative[0]*v_Derivative[1]) < 0.0 )
    {
        m_zerocrossing = (++m_zerocrossing) % 2;
        if(m_zerocrossing == 0)
        {
            m_output *= -1.0;
        }
    }
    v_BinaryOutput[curpos] = m_output; // note, however, that v_BinaryOutput accumulates phase delay about DIGITAL_FILTER_LENGTH
    emit BinaryOutputUpdated(v_BinaryOutput, m_DataLength);
    //----------------------------------------------------------------------------

    if(m_HeartSNRControlFlag)
    {
        if(m_HeartSNR > SNR_TRESHOLD)
        {
            emit vpgUpdated(m_ID, v_HeartSignal[curpos]);
            emit svpgUpdated(m_ID, v_SmoothedSignal[loopInput(curpos)]);
        }
        else
        {
            emit vpgUpdated(m_ID, 0.0);
            emit svpgUpdated(m_ID, 0.0);
        }
    }
    else
    {
        emit vpgUpdated(m_ID, v_HeartSignal[curpos]);
        emit svpgUpdated(m_ID, v_SmoothedSignal[loopInput(curpos)]);
    }

    //----------------------------------------------------------------------------

    emit CurrentValues(v_HeartSignal[curpos], PCA_RAW_RGB(pos, 0), PCA_RAW_RGB(pos, 1), PCA_RAW_RGB(pos, 2));
    curpos = (++curpos) % m_DataLength; // for loop-like usage of ptData and the other arrays in this class
}

//----------------------------------------------------------------------------------------------------------

void QHarmonicProcessor::computeHeartRate()
{
    qint16 temp_position = curpos - 1;
    qreal buffer_duration = 0.0; // for buffer duration accumulation without first time interval
    if(f_PCA)
    {
        alglib::pcabuildbasis(PCA_RAW_RGB, m_BufferLength, 3, PCA_Info, PCA_Variance, PCA_Basis);
        if (PCA_Info == 1)
        {
            qreal mean0 = 0.0;
            qreal mean1 = 0.0;
            qreal mean2 = 0.0;
            for (unsigned int i = 0; i < m_BufferLength; i++)
            {
                mean0 += PCA_RAW_RGB(i,0);
                mean1 += PCA_RAW_RGB(i,1);
                mean2 += PCA_RAW_RGB(i,2);
            }
            mean0 /= m_BufferLength;
            mean1 /= m_BufferLength;
            mean2 /= m_BufferLength;

            qreal temp_sko = sqrt(PCA_Variance(0));
            qint16 start = loopBuffer(temp_position) - (m_BufferLength - 1); // has to be signed
            for (quint16 i = 0; i < m_BufferLength; i++)
            {
                quint16 pos = loopBuffer(start + i);
                v_HeartForFFT[i] = ((PCA_RAW_RGB(pos,0) - mean0)*PCA_Basis(0,0) + (PCA_RAW_RGB(pos,1) - mean1)*PCA_Basis(1,0) + (PCA_RAW_RGB(pos,2) - mean2)*PCA_Basis(2,0)) / temp_sko;
                buffer_duration += v_HeartTime[loop(temp_position - (m_BufferLength - 1) + i)];
            }
        }
        emit PCAProjectionUpdated(v_HeartForFFT, m_BufferLength);
    }
    else
    {
        quint16 pos;
        for (unsigned int i = 0; i < m_BufferLength; i++)
        {
            pos = loop(temp_position - (m_BufferLength - 1) + i);
            v_HeartForFFT[i] = v_HeartSignal[pos];
            buffer_duration += v_HeartTime[pos];
        }
    }

    fftw_execute(m_HeartPlan); // Datas were prepared, now execute fftw_plan

    qreal totalPower = 0.0;
    for (quint16 i = 0; i < (m_BufferLength/2 + 1); i++)
    {
        v_HeartAmplitude[i] = v_HeartSpectrum[i][0]*v_HeartSpectrum[i][0] + v_HeartSpectrum[i][1]*v_HeartSpectrum[i][1];
        totalPower += v_HeartAmplitude[i];
    }
    for (quint16 i = 0; i < (m_BufferLength/2 + 1); i++) // normalization
    {
        v_HeartAmplitude[i] /= totalPower;
    }
    emit heartSpectrumUpdated(v_HeartAmplitude, m_BufferLength/2 + 1);

    quint16 bottom_bound = (quint16)(BOTTOM_LIMIT * buffer_duration / 1000.0);   // You should ensure that ( LOW_HR_LIMIT < discretization frequency / 2 )
    quint16 top_bound = (quint16)(TOP_LIMIT * buffer_duration / 1000.0);
    if(top_bound > (m_BufferLength / 2 + 1))
    {
        top_bound = m_BufferLength / 2 + 1;
    }
    quint16 index_of_maxpower = 0;
    qreal maxpower = 0.0;
    for (quint16 i = ( bottom_bound + HALF_INTERVAL ); i < ( top_bound - HALF_INTERVAL ); i++)
    {
        if ( maxpower < v_HeartAmplitude[i] )
        {
            maxpower = v_HeartAmplitude[i];
            index_of_maxpower = i;
        }
    }
    /*-------------------------SNR estimation evaluation-----------------------*/
    qreal noise_power = 0.0;
    qreal signal_power = 0.0;
    qreal power_multiplyed_by_index = 0.0;
    for (quint16 i = bottom_bound; i < top_bound; i++)
    {
        if ( (i >= (index_of_maxpower - HALF_INTERVAL )) && (i <= (index_of_maxpower + HALF_INTERVAL)) )
        {
            signal_power += v_HeartAmplitude[i];
            power_multiplyed_by_index += i * v_HeartAmplitude[i];
        }
        else
        {
            noise_power += v_HeartAmplitude[i];
        }
    }    
    if(signal_power < 0.01)
        m_HeartSNR = -13.0;
    else
    {
        m_HeartSNR = 10 * log10( signal_power / noise_power ); // this string may cause problem in msvc11, future issue to handle exeption
        qreal bias = (qreal)index_of_maxpower - ( power_multiplyed_by_index / signal_power );
        m_HeartSNR *= (1 / (1 + bias*bias));
    }
    emit snrUpdated(m_ID, m_HeartSNR); // signal for mapper

    if(m_HeartSNR > SNR_TRESHOLD)
    {
        m_HeartRate = (power_multiplyed_by_index / signal_power) * 60000.0 / buffer_duration;
        if((m_HeartRate <= m_rightTreshold) && (m_HeartRate >= m_leftThreshold))
            emit heartRateUpdated(m_HeartRate, m_HeartSNR, true);
        else
            emit heartRateUpdated(m_HeartRate, m_HeartSNR, false);
    }
    else
       emit heartTooNoisy(m_HeartSNR);

    if(m_HeartSNRControlFlag)
    {
        if(m_HeartSNR > SNR_TRESHOLD)
           emit amplitudeUpdated(m_ID, 10*signal_power);
        else
            emit amplitudeUpdated(m_ID, 0.0);
    }
    else
        emit amplitudeUpdated(m_ID, 10*signal_power);

}

//----------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setPCAMode(bool value)
{
    f_PCA = value;
}

//----------------------------------------------------------------------------------------------------

void QHarmonicProcessor::switchColorMode(int value)
{
    m_ColorChannel = (ColorChannel)value;
}

//----------------------------------------------------------------------------------------------------

void QHarmonicProcessor::CountFrequency()
{
    qint16 position = curpos - 1; // delay on 1 count is critical valuable here
    quint16 watchDogCounter = 0;
    quint16 sign_changes = m_PulseCounter;
    qreal temp_time = 0.0;

    while((v_BinaryOutput[loop(position)]*v_BinaryOutput[loop(position-1)] > 0.0) && (watchDogCounter < m_DataLength))
    {
        position--;
        watchDogCounter++;
    }

    while((sign_changes > 0) && (watchDogCounter < m_DataLength))
    {
        if(v_BinaryOutput[loop(position)]*v_BinaryOutput[loop(position-1)] < 0.0)
        {
            sign_changes--;
        }
        position--;
        watchDogCounter++;
        temp_time += v_HeartTime[loop(position)];
    }

    m_HeartRate = 60.0 * (m_PulseCounter - 1) / ((temp_time - v_HeartTime[loop(position)])/1000.0);
    emit heartRateUpdated(m_HeartRate,0.0,true);
}

//----------------------------------------------------------------------------------------------------

int QHarmonicProcessor::loadWarningRates(const char *fileName, SexID sex, int age, TwoSideAlpha alpha)
{
    if( !QFile::exists( fileName ) ) {
        return FileExistanceError;
    }

    QFile file(fileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        return FileOpenError;
    }

    QXmlStreamReader reader(&file);
    QString desiredSex;
    switch(sex)   {
        case Male:
            desiredSex = "male";
            break;
        default:
            desiredSex = "female";
            break;
    }
    QString lowerPercentile;
    QString highestPercentile;
    switch(alpha) {
        case TwoPercents:
            lowerPercentile = "percentile1.0";
            highestPercentile = "percentile99.0";
            break;
        case FivePercents:
            lowerPercentile = "percentile2.5";
            highestPercentile = "percentile97.5";
            break;
        case TenPercents:
            lowerPercentile = "percentile5.0";
            highestPercentile = "percentile95.0";
            break;
        case TwentyPercents:
            lowerPercentile = "percentile10.0";
            highestPercentile = "percentile90.0";
            break;
        default:
            lowerPercentile = "percentile25.0";
            highestPercentile = "percentile75.0";
            break;
    }

    bool FoundSexSection = false;
    bool FoundAgeSection = false;
    bool FoundLowerPercentile = false;
    bool FoundHighestPercentile = false;
    bool ConversionResult1 = false;
    bool ConversionResult2 = false;
    qreal tempLeft = 0.0;
    qreal tempRight = 0.0;

    while(!reader.atEnd())   { // read to the end of xml file
        reader.readNext();
        if(reader.error()) {
            return ParseFailure;
        }
        else {
            if(reader.attributes().hasAttribute("type")) {
                if(reader.attributes().value("type") == desiredSex)
                    FoundSexSection = true;
            }
            if(FoundSexSection && reader.attributes().hasAttribute("agefrom")) {
                if( (age >= reader.attributes().value("agefrom").toInt()) && (age <= reader.attributes().value("ageto").toInt()))
                    FoundAgeSection = true;
            }
            if(FoundSexSection && FoundAgeSection) {
                if(reader.isStartElement() && (reader.name() == lowerPercentile))
                    FoundLowerPercentile = true;
                if(reader.isStartElement() && (reader.name() == highestPercentile))
                    FoundHighestPercentile = true;
            }
            if(FoundLowerPercentile && reader.isCharacters()) {
                tempLeft = reader.text().toDouble(&ConversionResult1);
                qWarning("leftTreshold: %f", tempLeft);
            }
            if(FoundLowerPercentile && reader.isEndElement() && (reader.name() == lowerPercentile))
                FoundLowerPercentile = false;

            if(FoundHighestPercentile && reader.isCharacters()) {
                tempRight = reader.text().toDouble(&ConversionResult2);
                qWarning("RightTreshold: %f", tempRight);
            }
            if(ConversionResult1 && ConversionResult2) {
                m_leftThreshold = tempLeft;
                m_rightTreshold = tempRight;
                return NoError;
            }
        }
    }
    qWarning("Xml parsing:can not find appropriate record in file!");
    return ReadError;
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setID(quint32 value)
{
    m_ID = value;
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setEstiamtionInterval(int value)
{
    if((value > 1) && (value <= m_DataLength))
        m_estimationInterval = value;
}

//------------------------------------------------------------------------------------------------

unsigned int QHarmonicProcessor::getDataLength() const
{
    return m_DataLength;
}

//------------------------------------------------------------------------------------------------

unsigned int QHarmonicProcessor::getBufferLength() const
{
    return m_BufferLength;
}

//------------------------------------------------------------------------------------------------

quint16 QHarmonicProcessor::getEstimationInterval() const
{
    return m_estimationInterval;
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setSnrControl(bool value)
{
    m_HeartSNRControlFlag = value;
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::computeBreathRate()
{
    quint16 position = m_BreathCurpos - 1;
    qreal duration = 0.0;
    quint16 pos;
    for(quint16 i = 0; i < m_BufferLength; i++)
    {
        pos = loop(position - (m_BufferLength - 1) + i);
        v_BreathForFFT[i] = v_BreathSignal[pos];
        duration += v_BreathTime[pos];
    }

    fftw_execute(m_BreathPlan);

    qreal total_power = 0.0;
    for(quint16 i = 0; i < (m_BufferLength/2 + 1) ; i++)
    {
       v_BreathAmplitude[i] = v_BreathSpectrum[i][0]*v_BreathSpectrum[i][0] + v_BreathSpectrum[i][1]*v_BreathSpectrum[i][1];
       total_power += v_BreathAmplitude[i];
    }
    for(quint16 i = 0; i < (m_BufferLength/2 + 1) ; i++)
    {
       v_BreathAmplitude[i] /= total_power;
    }
    emit breathSpectrumUpdated(v_BreathAmplitude, (m_BufferLength/2 + 1));

    quint16 bottom = (quint16)(BREATH_BOTTOM_LIMIT * duration / 1000.0);   // You should ensure that ( LOW_HR_LIMIT < discretization frequency / 2 )
    quint16 top = (quint16)(BREATH_TOP_LIMIT * duration / 1000.0);
    quint16 index_of_maxpower = 0;
    qreal maxpower = 0.0;
    for (quint16 i = ( bottom + BREATH_HALF_INTERVAL ); i < ( top - BREATH_HALF_INTERVAL ); i++)
    {
        if ( maxpower < v_BreathAmplitude[i] )
        {
            maxpower = v_BreathAmplitude[i];
            index_of_maxpower = i;
        }
    }

    qreal noise_power = 0.0;
    qreal signal_power = 0.0;
    qreal power_x_index = 0.0;
    for (quint16 i = bottom; i < top; i++)
    {
        if ( (i >= (index_of_maxpower - BREATH_HALF_INTERVAL )) && (i <= (index_of_maxpower + BREATH_HALF_INTERVAL)) )
        {
            signal_power += v_BreathAmplitude[i];
            power_x_index += i * v_BreathAmplitude[i];
        }
        else
        {
            noise_power += v_BreathAmplitude[i];
        }
    }
    if((signal_power < 0.01) || (noise_power < 0.01))
        m_BreathSNR = -13.0;
    else
    {
        m_BreathSNR = 10 * log10( signal_power / noise_power ); // this string may cause problem in msvc11, future issue to handle exeption
        qreal bias = (qreal)index_of_maxpower - ( power_x_index / signal_power );
        m_BreathSNR *= (1 / (1 + bias*bias));
    }
    emit breathSnrUpdated(m_ID, m_BreathSNR); // signal for mapper

    if(m_BreathSNR > BREATH_SNR_TRESHOLD)
    {
        m_BreathRate = (power_x_index / signal_power) * 60000.0 / duration;
        emit breathRateUpdated(m_BreathRate, m_BreathSNR);
    }
    else
    {
       emit breathTooNoisy(m_BreathSNR);
    }

    emit measurementsUpdated(m_HeartRate, m_HeartSNR, m_BreathRate, m_BreathSNR);
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setBreathStrobe(int value)
{
    if(value > 0)
    {
        m_BreathStrobe = value;
    }
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setBreathAverage(int value)
{
    if((value > 0) && (value <= m_DataLength))
    {
        m_BreathAverageInterval = value;
    }
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setBreathCNInterval(int value)
{
    if((value > 1) && (value <= m_DataLength))
    {
        m_BreathCNInterval = value;
    }
}

//------------------------------------------------------------------------------------------------

quint16 QHarmonicProcessor::getBreathStrobe() const
{
    return m_BreathStrobe;
}

//------------------------------------------------------------------------------------------------

quint16 QHarmonicProcessor::getBreathAverage() const
{
    return m_BreathAverageInterval;
}

//------------------------------------------------------------------------------------------------

quint16 QHarmonicProcessor::getBreathCNInterval() const
{
    return m_BreathCNInterval;
}

//------------------------------------------------------------------------------------------------

void QHarmonicProcessor::setPruning(bool value)
{
    m_pruningFlag = value;
}

