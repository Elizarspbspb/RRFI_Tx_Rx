#include "extract_LTS.h"
#include "commonfunc.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "fft.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: extract_LTS_11a
//
// Purpose: Extract long training symbols
//
// Author: Alexandr Kozlov
//
// Parameters: freq_tr_syms - output, Long training symbols
//
//						 time_signal - input, Received time domain signal 
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void extract_LTS_11a( fxp_complex* freq_tr_syms,
  		                fxp_complex* time_signal )
{

  unsigned i, j;
  fxp_complex long_tr_syms[2 * NumOfSubcarriers];

  #ifdef LOGGING
  logging( "extract_LTS_11a...\n", 1 );
  #endif

  //[ Extract long training sequences
  for ( i = 0; i < 2 * NumOfSubcarriers; i++ )
  {
    long_tr_syms[i] = time_signal[i];
  }
  //]

  //[ Convert to frequency domain
  for ( i = 0; i < 2; i++ )
  {
    fft_with_mapping_11a( long_tr_syms + i * NumOfSubcarriers, 6, 0, 
                          fxp_params.FFT_precision,
                          fxp_params.FFT_exp_position, 
                          fxp_params.FFT_shift_control );
  }
  //]

  //[ Select training carriers and normalize
  for ( i = 0; i < 2; i++ )
  {
    for ( j = 0; j < NumOfUsedSubcarriers; j++ )
    {
      freq_tr_syms[i * NumOfUsedSubcarriers + j] = 
        long_tr_syms[ i * NumOfSubcarriers + sim_consts.UsedSubcIdx[j] - 1 ];
    }
  }
  //]

  #ifdef LOGGING
  logging( "extract_LTS_11a finished\n", -1 );
  #endif
}
