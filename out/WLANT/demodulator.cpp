/* File History
 * $History: demodulator.c $
 * 
 * *****************  Version 5  *****************
 * User: Akozlov      Date: 16.12.04   Time: 16:12
 * Updated in $/WLAN
 * Calculating of channel_sq_amplitudies changed
 * 
 */

#include "demodulator.h"
#include "sim_params.h"
#include "sim_consts.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: demodulator_11a
//
// Purpose: Implements QAM - demodulation
//
// Author: Alexandr Kozlov
//
// Parameters: soft_bits - output, Soft-output demodulated data bits
// 
//						 freq_OFDM_syms - input, Demodulated symbols
//
//             channel_estimate - input, Estimated transfer factors of channels	
//
//             bits_per_QAM_symbol - Bits per QAM symbol
//
//             num_of_OFDM_syms - input, Number of OFDM symbols
//
//             channel_sq_amplitudies - output, Channel square amplitudies
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void demodulator_11a( int* soft_bits,
  	              		fxp_complex* freq_OFDM_syms,
                      fxp_complex* channel_estimate,
                      unsigned num_of_OFDM_syms,
                      unsigned bits_per_QAM_symbol,
                      int* channel_sq_amplitudies )
{
  unsigned i,j,k;
  int* cur_dest;
  int threshold;
  int last_soft_metric1;
  int last_soft_metric2;
  
  char overflow_flag;

  #ifdef LOGGING
  logging( "demodulator_11a...\n", 1 );
  #endif

  //[ Calculate channel amplitudes
  for ( j = 0; j < NumOfDataSubcarriers; j++ )
  {
    #ifdef LOGGING
    sprintf( overflow_comment, 
		"Overflow: channel_sq_amplitudies[%d] = \n", j );
    #endif
    
    channel_sq_amplitudies[j] =
      fxp_complex_abs2( channel_estimate[ sim_consts.DataSubcPatt[j] - 1 ],
        fxp_params.Demodulator_precision,
        fxp_params.Demodulator_exp_position,
        &overflow_flag,
        overflow_comment );
                        

    /*channel_sq_amplitudies[j] = 
      fxp_complex_mul(
        channel_estimate[ sim_consts.DataSubcPatt[j] - 1 ],
        fxp_complex_conjugate( channel_estimate[ sim_consts.DataSubcPatt[j] - 1 ],
                               fxp_params.Demodulator_precision,
                               &overflow_flag,
                               overflow_comment ),
        fxp_params.Demodulator_precision,
        fxp_params.Demodulator_exp_position,
        &overflow_flag,
        overflow_comment ).re;*/
  } 
  //]

  //[ Demodulate
  cur_dest = soft_bits;

  for ( i = 0; i < num_of_OFDM_syms; i++ )
  {
    for ( k = 0; k < NumOfDataSubcarriers; k++ )
    {
      if ( bits_per_QAM_symbol != 1 )
      {
        #ifdef LOGGING
        sprintf( overflow_comment, 
		    "Overflow: threshold = fxp_mul, i = %d\n", i );
        #endif

        threshold = fxp_mul( sim_consts.QAMThresholds[0][bits_per_QAM_symbol / 2],
                             channel_sq_amplitudies[k],
                             fxp_params.Demodulator_precision,
                             fxp_params.Demodulator_exp_position,
                             &overflow_flag,
                             overflow_comment );

        last_soft_metric1 = *cur_dest = 
          freq_OFDM_syms[i * NumOfSubcarriers + sim_consts.DataSubcIdx[k] - 1].re;

        cur_dest ++;

        for( j = 1; j < bits_per_QAM_symbol / 2; j++ )
        {
          #ifdef LOGGING
          sprintf( overflow_comment, 
		      "Overflow: last_soft_metric1, i = %d, j = %d\n", i, j );
          #endif

          last_soft_metric1 = *cur_dest = 
            fxp_sub( threshold,
                     last_soft_metric1 > 0 ? last_soft_metric1 : -last_soft_metric1,
                     fxp_params.Demodulator_precision,
                     &overflow_flag,
                     overflow_comment );

          cur_dest++;

          threshold >>= 1;
        }

        threshold = fxp_mul( sim_consts.QAMThresholds[0][bits_per_QAM_symbol / 2],
                             channel_sq_amplitudies[k],
                             fxp_params.Demodulator_precision,
                             fxp_params.Demodulator_exp_position,
                             &overflow_flag,
                             overflow_comment );

        last_soft_metric2 = *cur_dest = 
          freq_OFDM_syms[i * NumOfSubcarriers + sim_consts.DataSubcIdx[k] - 1].im;

        cur_dest++;

        for( j = 1; j < bits_per_QAM_symbol / 2; j++ )
        {
          #ifdef LOGGING
          sprintf( overflow_comment, 
		      "Overflow: last_soft_metric2, i = %d, j = %d\n", i, j );
          #endif

          last_soft_metric2 = *cur_dest = 
            fxp_sub( threshold,
                     last_soft_metric2 > 0 ? last_soft_metric2 : -last_soft_metric2,
                     fxp_params.Demodulator_precision,
                     &overflow_flag,
                     overflow_comment );

          cur_dest++;
                    
          threshold >>= 1;
        }


//        cur_dest += bits_per_QAM_symbol;
      }
      else
      {
        (*cur_dest) = 
          freq_OFDM_syms[ i * NumOfSubcarriers + sim_consts.DataSubcIdx[k] - 1 ].re;

        cur_dest ++;
      }
    }
  }
  //]

  #ifdef LOGGING
  logging( "demodulator_11a finished\n", -1 );
  #endif
}




