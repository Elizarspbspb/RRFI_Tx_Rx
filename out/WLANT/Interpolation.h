#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_

#include <gsl/gsl_complex_math.h>

#define INTERPOLATION_FILTER_TYPE 2  // 1=len20   2=len30


int Interpolation( gsl_complex* input_signal,	      // The pointer to input vector
					          gsl_complex* interpolated_signal,   // The pointer to output vector
  				          int input_length,            			  // Length of the input sequence
                    int times     
                  );


int Decimation(  gsl_complex* input_signal,	      // The pointer to input signal vector
			            gsl_complex* decimated_signal,   // The poinetr to output signal vector
				          int input_length,            			  // Length of the input sequence
                  int times     
                );


int apply_decimation_filter(
                  gsl_complex* input_signal,	      // The pointer to input signal vector
				          int input_length            			  // Length of the input sequence
                  );

int do_decimate(
                  gsl_complex* input_signal,	      // The pointer to input signal vector
				          int input_length            			  // Length of the input sequence
                  );




int Decimation2(  gsl_complex* input_signal,	      // The pointer to input signal vector
			            gsl_complex* decimated_signal,   // The poinetr to output signal vector
				          int input_length,            			  // Length of the input sequence
                  int times     
                );


//#define TESTING_ROUTINES

#ifdef TESTING_ROUTINES

void test_interpolation();

#endif // TESTING_ROUTINES


#endif // _INTERPOLATION_H_