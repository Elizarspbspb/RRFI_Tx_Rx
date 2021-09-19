#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include "commonfunc.h"

void demodulator_11a( int* soft_bits,
  	              		fxp_complex* freq_OFDM_syms,
                      fxp_complex* channel_estimate,
                      unsigned num_of_OFDM_syms,
                      unsigned bits_per_QAM_symbol,
                      int* channel_sq_amplitudies );

#endif // DEMODULATOR_H
