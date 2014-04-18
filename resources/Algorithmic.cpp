//---------------------------------------------------------------------------
#include "Algorithmic.h"
//--------------------------------------------------------------------------
 TRealTimeProcessing::TRealTimeProcessing(unsigned int length_of_data, unsigned int length_of_buffer)
{
	datalength = length_of_data;
	bufferlength = length_of_buffer;
	curpos = 0;
    time_interval = 600.0; // in ms
	count_interval = 0;
	after_sign_change = false;
	current_freq_estimation = 0;
    ch1_mean = 0.0;
    ch2_mean = 0.0;
    SNRE = 0.0;

	ptData_ch1 = new MyFloatType[datalength];
	ptData_ch2 = new MyFloatType[datalength];
	ptCNSignal = new MyFloatType[datalength];
    ptTime = new double[datalength];

    ptX = new MyFloatType[DIGITAL_FILTER_LENGTH];

	ptDataForFFT = new float[bufferlength];
	ptSpectrum = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * (bufferlength/2 + 1));

	for (unsigned int i = 0; i < datalength; i++)
	{
		ptData_ch1[i] = 0.0;
		ptData_ch2[i] = 0.0;
        ptTime[i] = 55.0; // just for ensure that at the begining there is not any "division by zero"
		ptCNSignal[i] = 0.0;
	}

	// Memory allocation block for ALGLIB arrays
	PCA_RAW_RGB.setlength(bufferlength, 3); // 3 because RED, GREEN and BLUE colors represent 3 independent variables
	PCA_Variance.setlength(3);
	PCA_Basis.setlength(3, 3);
	PCA_Info = 0;

	for (unsigned int i = 0; i < bufferlength; i++)
		for (int j = 0; j < 3; j++)
			{
				PCA_RAW_RGB(i,j) =  i + j; // just for good beginning, a random numbers generator could be here
			}
}
//--------------------------------------------------------------------------
 TRealTimeProcessing::~TRealTimeProcessing()
{
	delete[] ptData_ch1;
	delete[] ptData_ch2;
	delete[] ptCNSignal;
	delete[] ptTime;
    delete[] ptX;
	delete[] ptDataForFFT;
	fftwf_free(ptSpectrum);
}
//--------------------------------------------------------------------------
void  TRealTimeProcessing::WriteToDataColor(unsigned long int red, unsigned long int green, unsigned long int blue, unsigned long int area, double time)
{

    PCA_RAW_RGB(loop_for_PCA(curpos), 0) = (MyFloatType)red / area;
    PCA_RAW_RGB(loop_for_PCA(curpos), 1) = (MyFloatType)green / area;
    PCA_RAW_RGB(loop_for_PCA(curpos), 2) = (MyFloatType)blue / area;

    MyFloatType ch1_temp = (MyFloatType)(red - green) / area;
    MyFloatType ch2_temp = (MyFloatType)(red + green - 2 * blue) / area;

    ch1_mean += (ch1_temp - ptData_ch1[curpos]) / datalength;
    ch2_mean += (ch2_temp - ptData_ch2[curpos]) / datalength;

	ptData_ch1[curpos] = ch1_temp;
	ptData_ch2[curpos] = ch2_temp;
	ptTime[curpos] = time;

    MyFloatType ch1_sko = 0.0;
    MyFloatType ch2_sko = 0.0;
	for (unsigned int i = 0; i < datalength; i++)
	{
		ch1_sko += (ptData_ch1[i] - ch1_mean)*(ptData_ch1[i] - ch1_mean);
		ch2_sko += (ptData_ch2[i] - ch2_mean)*(ptData_ch2[i] - ch2_mean);
	}
	ch1_sko = sqrt(ch1_sko / (datalength - 1));
	ch2_sko = sqrt(ch2_sko / (datalength - 1));

    ptX[loop_for_ptX(curpos)] = (ptData_ch1[curpos] - ch1_mean) / ch1_sko  - (ptData_ch2[curpos] - ch2_mean) / ch2_sko;
    ptCNSignal[curpos] = ( ptX[loop_for_ptX(curpos)] + ptCNSignal[loop(curpos - 1)] ) / 2;

    //This part make something like Shmit-trigger work and in that way works well, but frequency approximation is very weak
	count_interval++; // incremet this on each call of the WriteToData(...)
    if ( (!after_sign_change) && (ptCNSignal[curpos] < -UNIT_SKO_PORTION ) )
	{
		after_sign_change = true;
	}
	else if ( after_sign_change && ( ptCNSignal[curpos] > UNIT_SKO_PORTION ) )
	{
        for (unsigned int i = 0; i < count_interval; i++)
		{
			time_interval += ptTime[loop(curpos-i)];//
		}
        time_interval /= 2; // digital filtration - mean between two closest period estimations
		count_interval = 0;
		after_sign_change = false;
	}
	//-------------------------------------------------------------------------//

	curpos = (++curpos) % datalength; // for loop-like usage of ptData and the other arrays...
}
//---------------------------------------------------------------------------
void  TRealTimeProcessing::WriteToDataBW(unsigned long int intensity, unsigned long int area, double time)
{
    MyFloatType ch1_temp = (MyFloatType)intensity / area;

    ch1_mean += (ch1_temp - ptData_ch1[curpos]) / datalength;

	ptData_ch1[curpos] = ch1_temp;
	ptTime[curpos] = time;

    MyFloatType ch1_sko = 0.0;
	for (unsigned int i = 0; i < datalength; i++)
	{
		ch1_sko += (ptData_ch1[i] - ch1_mean)*(ptData_ch1[i] - ch1_mean);
	}
	ch1_sko = sqrt(ch1_sko / (datalength - 1));

    ptCNSignal[curpos] = ( ( (ptData_ch1[curpos] - ch1_mean) / ch1_sko ) + ptCNSignal[loop(curpos-1)] ) / 2;

    //This part make something like Shmit-trigger work and in that way works well, but frequency approximation is very weak
	count_interval++; // incremet this on each call of the WriteToData(...)
	if ( (!after_sign_change) && (ptCNSignal[curpos] < ( -1 * UNIT_SKO_PORTION )) )
	{
		after_sign_change = true;
	}
	else if ( after_sign_change && ( ptCNSignal[curpos] > UNIT_SKO_PORTION ) )
	{
        for (unsigned int i = 0; i < count_interval; i++)
		{
			time_interval += ptTime[loop(curpos-i)];//
		}
        time_interval /= 2; // digital filtration - mean between two closest period estimations
		count_interval = 0;
		after_sign_change = false;
	}
	//-------------------------------------------------------------------------//

	curpos = (++curpos) % datalength; // for loop-like usage of ptData and the other arrays...
}

//---------------------------------------------------------------------------
MyFloatType  TRealTimeProcessing::ComputeFrequency(bool PCA_Enable)
{
	unsigned int temp_curpos = curpos; // here i tried to figure out - maybe curpos read (at write time) causes the system to hang 
    double buffer_duration = 0.0; // for buffer duration accumulation

	fftwf_plan p = fftwf_plan_dft_r2c_1d(bufferlength, ptDataForFFT, ptSpectrum, FFTW_ESTIMATE);
	if (PCA_Enable)
	{
		alglib::pcabuildbasis(PCA_RAW_RGB, bufferlength, 3, PCA_Info, PCA_Variance, PCA_Basis);
		if (PCA_Info == 1)
		{
			MyFloatType mean0 = 0.0;
			MyFloatType mean1 = 0.0;
			MyFloatType mean2 = 0.0;
			for (unsigned int i = 0; i < bufferlength; i++)
			{
				mean0 += PCA_RAW_RGB(i,0);
				mean1 += PCA_RAW_RGB(i,1);
				mean2 += PCA_RAW_RGB(i,2);
			}
			mean0 /= bufferlength;
			mean1 /= bufferlength;
			mean2 /= bufferlength;
			MyFloatType temp_sko0 = sqrt(PCA_Variance(0));

			unsigned int start_position = loop_for_PCA(temp_curpos) - (bufferlength - 1);
			ptDataForFFT[0] = ((PCA_RAW_RGB(loop_for_PCA(start_position),0) - mean0)*PCA_Basis(0,0) + (PCA_RAW_RGB(loop_for_PCA(start_position),1) - mean1)*PCA_Basis(1,0) + (PCA_RAW_RGB(loop_for_PCA(start_position),2) - mean2)*PCA_Basis(2,0))/ temp_sko0;
			buffer_duration += ptTime[loop(temp_curpos - (bufferlength - 1))];
			for (unsigned int i = 1; i < bufferlength; i++)
			{
				unsigned int position = loop_for_PCA(start_position + i);
				ptDataForFFT[i] = (((PCA_RAW_RGB(position,0) - mean0)*PCA_Basis(0,0) + (PCA_RAW_RGB(position,1) - mean1)*PCA_Basis(1,0) + (PCA_RAW_RGB(position,2) - mean2)*PCA_Basis(2,0))/ temp_sko0 + ptDataForFFT[i-1])/2;
				buffer_duration += ptTime[loop(temp_curpos - (bufferlength - 1) + i)];
			}
		}

	}
	else
	{
		for (unsigned int i = 0; i < bufferlength; i++)
		{
			ptDataForFFT[i] = ptCNSignal[loop(temp_curpos - (bufferlength - 1) + i)]; // an real part
			buffer_duration += ptTime[loop(temp_curpos - (bufferlength - 1) + i)];
		}
	}

	fftwf_execute(p);

    unsigned int lower_bound = (unsigned int)(LOWER_HR_LIMIT * buffer_duration / 1000);   //! You should ensure that ( LOW_HR_LIMIT < discretization frequency / 2 )

    unsigned int index_of_maxpower = 0;
	MyFloatType maxpower = 0.0;
	for (unsigned int i = ( lower_bound + HALF_INTERVAL ); i < ( (bufferlength / 2) - HALF_INTERVAL ); i++)
	{
		MyFloatType temp_power = ptSpectrum[i][0]*ptSpectrum[i][0] + ptSpectrum[i][1]*ptSpectrum[i][1];
		if ( maxpower < temp_power )
		{
			maxpower = temp_power;
			index_of_maxpower = i;
		}
	}
/*-------------------------SNR estimation evaluation-----------------------*/
    MyFloatType noise_power = 0.0;
    MyFloatType signal_power = 0.0;
	for (unsigned int i = lower_bound; i < (bufferlength / 2); i++)
	{
		if ( ( (i > (index_of_maxpower - HALF_INTERVAL )) && (i < (index_of_maxpower + HALF_INTERVAL) ) ) || ( (i > (2 * index_of_maxpower - HALF_INTERVAL )) && (i < (2 * index_of_maxpower + HALF_INTERVAL) ) ) )
		{
			signal_power += ptSpectrum[i][0]*ptSpectrum[i][0] + ptSpectrum[i][1]*ptSpectrum[i][1];
		}
		else
		{
			noise_power += ptSpectrum[i][0]*ptSpectrum[i][0] + ptSpectrum[i][1]*ptSpectrum[i][1];
        }
	}
    if ((noise_power > 0.0) && (signal_power > 0.0))
	{
		SNRE = 10 * log10( signal_power / noise_power );

		if ( SNRE > SNR_TRESHOLD )
		{
        MyFloatType power_multiplyed_by_index = 0.0;
        MyFloatType power_of_first_harmonic = 0.0;
        for (unsigned int i = (index_of_maxpower - HALF_INTERVAL + 1); i < (index_of_maxpower + HALF_INTERVAL); i++)
		{
			power_of_first_harmonic += (ptSpectrum[i][0]*ptSpectrum[i][0] + ptSpectrum[i][1]*ptSpectrum[i][1]);
			power_multiplyed_by_index += i * (ptSpectrum[i][0]*ptSpectrum[i][0] + ptSpectrum[i][1]*ptSpectrum[i][1]);
		}
        current_freq_estimation = (power_multiplyed_by_index / power_of_first_harmonic) * 60000 / buffer_duration; // averaging suitable only for short periods of measurement (less than 3 s)
        }
	}

	fftwf_destroy_plan(p);
	return current_freq_estimation;
}
//---------------------------------------------------------------------------
MyFloatType  TRealTimeProcessing::EvaluateFrequency()
{
    return (60000 / time_interval);
}
//---------------------------------------------------------------------------

MyFloatType TRealTimeProcessing::getCurentSignalCount()
{
    return ptCNSignal[curpos];
}
