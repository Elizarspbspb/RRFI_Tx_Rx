/* File History
 * $History: equalizer.c $
 * 
 * *****************  Version 4  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:49
 * Updated in $/WLAN
 * Demodulator input convertion added
 * 
 */


#include "equalizer.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: equalizer_11a
//
// Purpose: Equalize OFDM symbols
//
// Author: Alexandr Kozlov
//
// Parameters: freq_OFDM_syms - input/output, OFDM symbols in frequency domain
//
//             channel_estimate - input, Estimated transfer factors of channels	
//																
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void equalizer_11a( fxp_complex* freq_OFDM_syms,
		                fxp_complex* channel_estimate,
                    unsigned num_of_OFDM_syms )
{
	unsigned i,j;

	char overflow_flag;

	fxp_complex conj_channel_estimate;

  #ifdef LOGGING
  logging( "equalizer_11a...\n", 1 );
  #endif

	//[ Convert precision freq_OFDM_syms to the fxp_params.Demodulator_precision
	for ( i = 0; i < num_of_OFDM_syms; i++ )
	{
		for ( j = 0; j < NumOfSubcarriers; j++ )
		{

      #ifdef LOGGING
      sprintf( overflow_comment, 
			"Overflow: freq_OFDM_syms[ %d * NumOfSubcarriers + %d ].re\n", i, j );
      #endif
   
      freq_OFDM_syms[ i * NumOfSubcarriers + j ].re =
         get_bits_from_fxp_number( freq_OFDM_syms[ i * NumOfSubcarriers + j ].re,
                                   fxp_params.first_bit_for_demodulator, 
                                   fxp_params.last_bit_for_demodulator );

      #ifdef LOGGING
      sprintf( overflow_comment, 
			"Overflow: freq_OFDM_syms[ %d * NumOfSubcarriers + %d ].im\n", i, j );
      #endif

      freq_OFDM_syms[ i * NumOfSubcarriers + j ].im =
         get_bits_from_fxp_number( freq_OFDM_syms[ i * NumOfSubcarriers + j ].im,
                                   fxp_params.first_bit_for_demodulator, 
                                   fxp_params.last_bit_for_demodulator );
		}
	}
	//]

  for ( i = 0; i < num_of_OFDM_syms; i++ )
	{
		for ( j = 0; j < NumOfUsedSubcarriers; j++ )
		{
      #ifdef LOGGING
      sprintf( overflow_comment, 
			"Overflow: conj_channel_estimate = fxp_complex_conjugate( channel_estimate[%d]\n", j );
      #endif

			conj_channel_estimate = fxp_complex_conjugate( channel_estimate[j],
																										 fxp_params.Demodulator_precision,
																										 &overflow_flag,
																										 overflow_comment );

      #ifdef LOGGING
      sprintf( overflow_comment, 
			"Overflow: freq_OFDM_syms[ %d * NumOfSubcarriers +	sim_consts.UsedSubcIdx[%d] - 1 ]\n", i, j );
      #endif

			//[ Equalize
			freq_OFDM_syms[ i * NumOfSubcarriers + sim_consts.UsedSubcIdx[j] - 1 ] =
				fxp_complex_mul(
					freq_OFDM_syms[ i * NumOfSubcarriers + sim_consts.UsedSubcIdx[j] - 1 ],
					conj_channel_estimate,
          fxp_params.Demodulator_precision,
          fxp_params.Demodulator_exp_position,
          &overflow_flag,
          overflow_comment );
			//]
		}
	}

  #ifdef LOGGING
  logging( "equalizer_11a finished\n", -1 );
  #endif
}


