#include "frequency_sync.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "model.h"
#include <math.h>

#define AveragingLength (16)

/**
////////////////////////////////////////////////////////////////////////////////
// Name: coarse_frequency_sync_11a
//
// Purpose: Frequency error estimation
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input, Received time domain
//
// Returning value: Radians per sample rotation
//////////////////////////////////////////////////////////////////////////////// */
int coarse_frequency_sync_11a( fxp_complex* time_signal )
{
	unsigned i;
	fxp_complex phase;
  fxp_complex conj_time_sample;

  char overflow_flag;

	int radians_per_sample;

  #ifdef LOGGING
  logging( "coarse_frequency_sync_11a...\n", 1 );
  #endif

	//[ Estimate frequency error
  phase.re = 0;
  phase.im = 0;
	for ( i = 0; i < AveragingLength; i++ )
	{
	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: conj_time_sample = fxp_complex_conjugate, i=%d\n", i );
	  #endif

    conj_time_sample = fxp_complex_conjugate( time_signal[Delay + i],
                                              fxp_params.Freq_Sync_precision,
                                              &overflow_flag,
                                              overflow_comment );
   
	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: phase = fxp_complex_add( phase, i=%d\n", i );
	  #endif    

		phase = fxp_complex_add( phase,
			                       fxp_complex_mul( time_signal[i],
			                                        conj_time_sample,
                                              fxp_params.Freq_Sync_precision,
                                              fxp_params.Freq_Sync_exp_position,
                                              &overflow_flag,
                                              overflow_comment ),
                             fxp_params.Freq_Sync_precision,
                             &overflow_flag,
                             overflow_comment );
	}
  //]

	radians_per_sample = fxp_complex_arg( phase, 
                                        fxp_params.Arctan_precision,
                                        &overflow_flag,
                                        overflow_comment ) >> 4;


  #ifdef LOGGING
  logging( "coarse_frequency_sync_11a finished\n", -1 );
  #endif

  return radians_per_sample;
}

