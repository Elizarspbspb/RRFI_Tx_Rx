#ifndef FFT_H
#define FFT_H

#include "commonfunc.h"

void fft_with_mapping_11a( fxp_complex* x, 
                           int T,
                           char complement,
                           unsigned precision, 
                           unsigned exp_position,
                           unsigned shift_control );

void fft_11a( fxp_complex* x, 
              int T,
              char complement,
              unsigned precision, 
              unsigned exp_position,
              unsigned shift_control );



#ifdef TESTING_ROUTINES

void test_fft();

#endif



#endif

