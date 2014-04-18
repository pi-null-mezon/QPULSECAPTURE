//---------------------------------------------------------------------------

#ifndef AlgorithmicH
#define AlgorithmicH

#include "fftw3.h" // FFT library, courtesy from MIT
#include "ap.h" // ALGLIB TYPES
#include "dataanalysis.h" // ALGLIB header for PCA

#define LOWER_HR_LIMIT 0.5 // in s^-1, it is 30 bpm
#define SNR_TRESHOLD -1.5 // in most cases this value is suitable when (bufferlength == 256)
#define HALF_INTERVAL 5 // defines the number of averaging indexes when frequency is evaluated, this value should be >= 1
#define UNIT_SKO_PORTION 0.5 //defines threshold of the trigger, which determines time_interval values
#define DIGITAL_FILTER_LENGTH 3 // in counts

typedef double MyFloatType; // data type for arrays

class TRealTimeProcessing
{
	public:
         TRealTimeProcessing(unsigned int length_of_data, unsigned int length_of_buffer);
         ~TRealTimeProcessing();
        void  WriteToDataColor(unsigned long int red, unsigned long int green, unsigned long int blue, unsigned long int area, double time);
        void  WriteToDataBW(unsigned long int intensity, unsigned long int area, double time);
        MyFloatType ComputeFrequency(bool PCA_Enable);
        MyFloatType EvaluateFrequency();
        MyFloatType getCurentSignalCount();

		MyFloatType *ptCNSignal;  //a pointer to centered and normalized data (typedefinition from fftw3.h, a single precision complex float number type)
		fftwf_complex *ptSpectrum;  // a pointer to an array for FFT-spectrum
		MyFloatType SNRE; // a variable for signal-to-noise ratio estimation storing

	private:
		MyFloatType *ptData_ch1; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
		MyFloatType *ptData_ch2; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
		float *ptDataForFFT; //a pointer to data prepared for FFT, explicit float because fftwf (single precision) is used
		MyFloatType ch1_mean;   //a variable for mean value in channel1 storing
		MyFloatType ch2_mean;   //a variable for mean value in channel2 storing
        double *ptTime; //a pointer to an array for frame periods storing (values in milliseconds thus unsigned int)
        MyFloatType current_freq_estimation; //a variable for storing a last evaluated frequency of the 'strongest' harmonic
		unsigned int curpos; //a current position I meant
		unsigned int datalength; //a length of data array
		unsigned int bufferlength; //a lenght of sub data array for FFT (bufferlength should be <= datalength)

        MyFloatType time_interval; // this variable will store the value of time between rect-pulses in triggered version of ptCNSignal[]
		unsigned int count_interval; // this variable will store numbers of counts in ptTime[]
		bool after_sign_change;

		alglib::real_2d_array PCA_RAW_RGB; // a container for PCA analysis
		alglib::real_1d_array PCA_Variance; // array[0..2] - variance values corresponding to basis vectors
		alglib::real_2d_array PCA_Basis; // array[0..2,0..2], whose columns will store basis vectors
		alglib::ae_int_t PCA_Info; // PCA result code

        MyFloatType *ptX; // a pointer to input counts history, for digital filtration

        unsigned int loop(int); //a function that return a loop-index (not '' because 'inline' )
        unsigned int loop_for_ptX(int); //a function that return a loop-index (not '' because 'inline' )
        unsigned int loop_for_PCA(int); //a function that return a loop-index (not '' because 'inline' )
};

// inline for speed, must therefore reside in header file
inline unsigned int TRealTimeProcessing::loop(int difference)
{
	return ((datalength + (difference % datalength)) % datalength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
inline unsigned int TRealTimeProcessing::loop_for_ptX(int difference)
{
    return ((DIGITAL_FILTER_LENGTH + (difference % DIGITAL_FILTER_LENGTH)) % DIGITAL_FILTER_LENGTH); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
inline unsigned int TRealTimeProcessing::loop_for_PCA(int difference)
{
	return ((bufferlength + (difference % bufferlength)) % bufferlength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
#endif
