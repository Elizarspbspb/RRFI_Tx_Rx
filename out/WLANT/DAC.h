#ifndef DAC_H
#define DAC_H

#include <gsl/gsl_complex.h>
#include "commonfunc.h"

unsigned DAC_11a( gsl_complex* time_signal,
                  fxp_complex* fxp_time_signal,
                  unsigned precision, 
                  unsigned exp_position,
                  unsigned num_of_samples );

#endif // DAC_H
 