#include "commonfunc.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fine_frequency_sync_11a
//
// Purpose: Fine frequency error estimation
//
// Author: Alexandr Kozlov
//
// Parameters: time_tr_syms - input, Long training symbols
//
// Returning value: Radians per sample rotation
//////////////////////////////////////////////////////////////////////////////// */
int fine_frequency_sync_11a( fxp_complex* time_tr_syms )
{
  unsigned i;

	fxp_complex phase;
	int arg;
	char overflow_flag;

  #ifdef LOGGING
  logging( "fine_frequency_sync_11a...\n", 1 );
  #endif

  //[ Calculate phase
  phase.re = 0;
  phase.im = 0;
  for ( i = 0; i < NumOfSubcarriers; i++ )
  {
	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: phase = fxp_complex_add( phase, i=%d\n", i );
	  #endif

    phase = fxp_complex_add( phase,
          fxp_complex_mul(
            time_tr_syms[i],
            fxp_complex_conjugate( time_tr_syms[NumOfSubcarriers + i],
                                   fxp_params.Freq_Sync_precision,
                                   &overflow_flag,
                                   overflow_comment ),
            fxp_params.Freq_Sync_precision,
            fxp_params.Freq_Sync_exp_position,
            &overflow_flag,
            overflow_comment
          ),
          fxp_params.Freq_Sync_precision,
          &overflow_flag,
          overflow_comment
        );

    if ( ( phase.re > 1 << ( fxp_params.Freq_Sync_precision - 2 ) ) ||
         ( phase.im > 1 << ( fxp_params.Freq_Sync_precision - 2 ) ) )
    {
      phase.re >>= 1;
      phase.im >>= 1;
    }
  }
  //]

  //[ Correct long training symbols
  arg = fxp_complex_arg( phase,
                         fxp_params.Arctan_precision,
                         &overflow_flag,
                         overflow_comment );

  #ifdef LOGGING
  logging( "fine_frequency_sync_11a finished\n", -1 );
  #endif

	return arg >> 6;
} 