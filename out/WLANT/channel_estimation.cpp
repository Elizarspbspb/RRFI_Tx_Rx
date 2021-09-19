/* File History
 * $History: channel_estimation.c $
 * 
 * *****************  Version 4  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:49
 * Updated in $/WLAN
 * Demodulator input convertion added
 * 
 */

#include "channel_estimation.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: channel_estimation_11a
//
// Purpose: Channel estimation
//
// Author: Alexandr Kozlov
//
// Parameters: channel_estimate - output, Estimated transfer factors of channels	
//																
//						 freq_tr_syms - input, Received training symbols 
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void channel_estimation_11a( fxp_complex* channel_estimate,
		                         fxp_complex* freq_tr_syms
                             )
{
  unsigned i;
  char overflow_flag;

  #ifdef LOGGING
  logging( "channel_estimation_11a...\n", 1 );
  #endif

  //[ Convert precision for freq_tr_syms to the fxp_params.Demodulator_precision
  for ( i = 0; i < 2 * NumOfUsedSubcarriers; i++ )
  {
    #ifdef LOGGING
    sprintf( overflow_comment, "Overflow: freq_tr_syms[%d].re\n", i );
    #endif

    freq_tr_syms[i].re = get_bits_from_fxp_number(  freq_tr_syms[i].re,
                                                    fxp_params.first_bit_for_demodulator, 
                                                    fxp_params.last_bit_for_demodulator );

    #ifdef LOGGING
    sprintf( overflow_comment, "Overflow: freq_tr_syms[%d].im\n", i );
    #endif

    freq_tr_syms[i].im = get_bits_from_fxp_number(  freq_tr_syms[i].im,
                                                    fxp_params.first_bit_for_demodulator, 
                                                    fxp_params.last_bit_for_demodulator );
        
  }
  //]

  for ( i = 0; i < NumOfUsedSubcarriers; i++ )
  {
    #ifdef LOGGING
    sprintf( overflow_comment, 
            "Overflow: channel_estimate[%d] = fxp_complex_add\n", i );
    #endif

    channel_estimate[i] =
      fxp_complex_add( freq_tr_syms[i],
                       freq_tr_syms[ NumOfUsedSubcarriers + i ],
                       fxp_params.Demodulator_precision,
                       &overflow_flag,
                       overflow_comment );

    channel_estimate[i].re >>= 1;
    channel_estimate[i].im >>= 1;

    if ( sim_consts.LongTrainingSymbols[i] < 0 )
    {
      #ifdef LOGGING
      sprintf( overflow_comment, 
               "Overflow: channel_estimate[%d].re = fxp_not\n", i );
      #endif

      channel_estimate[i].re = fxp_not( channel_estimate[i].re,
                                        fxp_params.Demodulator_precision,
                                        &overflow_flag,
                                        overflow_comment );

      #ifdef LOGGING
      sprintf( overflow_comment, 
               "Overflow: channel_estimate[%d].im = fxp_not\n", i );
      #endif

      channel_estimate[i].im = fxp_not( channel_estimate[i].im,
                                        fxp_params.Demodulator_precision,
                                        &overflow_flag,
                                        overflow_comment );
    }
  }

  #ifdef LOGGING
  logging( "channel_estimation_11a finished\n", -1 );
  #endif
}
