# ifndef   BITLOADING_H
#   define BITLOADING_H

#include "commonfunc.h"

/////////////////////////////// New Bitloading prototypes

// Fixed point bitloading precision constants

// Defines fixed point variables width for tables and operations
// Also sets size of invLog table (2^BL_INV_LOG_DIGITS elements, BL_INV_LOG_DIGITS bits each)
#define BL_INV_LOG_DIGITS 10        

// Defines the precision of all fixed point operation in bitloading
#define BL_EXP_POSITION 4
#define BL_PRECISION 10

// Defines maximum signal constellation to use in bitloading (16QAM=4)
// Do not change unless P_SIG_ERR_DIF_LOG contents is properly modified too.
#define BL_MAX_CONSTELLATION 4

typedef struct sort_str_tag
{
  float value;
  int index;
}sort_str;

int init_NEW_bitloading_constants();

int NEW_bitloading_FXP
(
    const int*   channel_sq_amplitudes, 
    unsigned        required_bits,         
    double          max_power,             // unused
    unsigned*       bitsPerSymbol,     
    int*         signal_gains,          // unused, set to 1;
    unsigned        min_bitsPerSymbol    // unused
);

int BPLA_scheme_v3_1( int* modulation,
                      float* power,
                      sort_str* sorted_ENP, 
                      float* ENP,
                      float power_limit, 
                      float thresholdFCC, 
                      int N_subcarriers );

int Wraper_for_BPLA( const fxp_complex* freq_tr_syms,
                     const fxp_complex* additional_freq_tr_syms,
                     unsigned* bits_per_symbol,
                     float* data_profile );

int BPLA_fxp (const fxp_complex* freq_tr_syms,
			  unsigned* bits_per_symbol,
			  unsigned data_precision,
			  unsigned thresh_precision);

int BPLA_flp( const fxp_complex* freq_tr_syms,
                     unsigned* bits_per_symbol );

void setUpABLThresholds( float* ABL_thresholds );


#endif // BITLOADING_H