#ifndef DEMODULATORABL_H
#define DEMODULATORABL_H

#include "commonfunc.h"

//#define AMP_TABLE_PRECISION 10

void demodulator_for_ABL( int* soft_bits,
  	              		    fxp_complex* freq_OFDM_syms,
                          fxp_complex* channel_estimate,
                          unsigned num_of_OFDM_syms,
                          unsigned* ABL_bits_per_QAM_symbol,
                          int* channel_sq_amplitudies );

/*void init_inv_channel_amps_table();
int fxp_div( int a, int b, unsigned precision );*/

#endif // DEMODULATORABL_H
