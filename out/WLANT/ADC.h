#ifndef ADC_H
#define ADC_H

#include <gsl/gsl_complex.h>
#include "commonfunc.h"


void init_ADC_table();

void ADC_11a( gsl_complex* time_signal,
              fxp_complex* fxp_time_signal,
              unsigned num_of_samples );

#endif // ADC_H
