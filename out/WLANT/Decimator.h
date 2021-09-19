#ifndef _DECIMATOR_H_
#define _DECIMATOR_H_

#include <gsl/gsl_complex_math.h>


int Decimator( gsl_complex* time_signal, 
	            unsigned num_of_samples );


int Drift_And_Jitter( gsl_complex* time_signal, 
	            unsigned num_of_samples );


#endif // _DECIMATOR_H_