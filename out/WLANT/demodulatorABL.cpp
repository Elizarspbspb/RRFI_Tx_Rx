/* File History
 * $History: demodulatorABL.c $
 * 
 * *****************  Version 7  *****************
 * User: Akozlov      Date: 16.12.04   Time: 16:12
 * Updated in $/WLAN
 * Calculating of channel_sq_amplitudies changed
 * 
 * *****************  Version 6  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:47
 * Updated in $/WLAN
 * Multiply for LLR added
 * 
 */

#include "demodulatorABL.h"
#include "sim_params.h"
#include "sim_consts.h"
#include "model.h"

//int ABL_inv_amps_table[1<<AMP_TABLE_PRECISION];

/**
////////////////////////////////////////////////////////////////////////////////
// Name: demodulator_for_ABL
//
// Purpose: Implements QAM - demodulation for ABL case
//
// Author: Alexandr Kozlov
//
// Parameters: soft_bits - output, Soft-output demodulated data bits
// 
//						 freq_OFDM_syms - input, Demodulated symbols
//
//             channel_estimate - input, Estimated transfer factors of channels	
//
//             ABL_bits_per_QAM_symbol - input, Bits per QAM symbol for data subcarriers
//
//             num_of_OFDM_syms - input, Number of OFDM symbols
//
//             channel_sq_amplitudies - output, Channel square amplitudies
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void demodulator_for_ABL( int* soft_bits,
  	              		    fxp_complex* freq_OFDM_syms,
                          fxp_complex* channel_estimate,
                          unsigned num_of_OFDM_syms,
                          unsigned* ABL_bits_per_QAM_symbol,
                          int* channel_sq_amplitudies )
{
  unsigned i,j,k;
  int* cur_dest;
  int threshold;

  int multiplier;
  int last_soft_metric1;
  int last_soft_metric2;
  
  char overflow_flag;

  unsigned num_of_turn_off_subc = 0;

  #ifdef LOGGING
  logging( "demodulator_11a...\n", 1 );
  #endif

  //[ Calculate number of turn off subcarriers
  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
    if ( ABL_bits_per_QAM_symbol[i] == 0 )
    {
      num_of_turn_off_subc ++;
    }
  }

  if ( num_of_turn_off_subc > 36 )
  {
    num_of_turn_off_subc = 36;
  }

  if ( !sim_params.subcarriersPower )
  {
    num_of_turn_off_subc = 0;
  }
  //]

  //[ Calculate channel amplitidies
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
      if ( ( ABL_bits_per_QAM_symbol[k] != 1 ) && ( ABL_bits_per_QAM_symbol[k] != 0 ) )
      {
        #ifdef LOGGING
        sprintf( overflow_comment, 
		    "Overflow: threshold = fxp_mul, i = %d\n", i );
        #endif

        threshold = fxp_mul( sim_consts.QAMThresholds[num_of_turn_off_subc][ABL_bits_per_QAM_symbol[k] / 2],
                             channel_sq_amplitudies[k],
                             fxp_params.Demodulator_precision,
                             fxp_params.Demodulator_exp_position,
                             &overflow_flag,
                             overflow_comment );

        //LLR multiplier
		    multiplier = sim_consts.QAMThresholds[num_of_turn_off_subc][ABL_bits_per_QAM_symbol[k] / 2] >> (ABL_bits_per_QAM_symbol[k]/2-1);

        last_soft_metric1 = *cur_dest = 
          freq_OFDM_syms[i * NumOfSubcarriers + sim_consts.DataSubcIdx[k] - 1].re;

		    *cur_dest=fxp_mul(*cur_dest,
							    multiplier,
							    fxp_params.Demodulator_precision,
							    fxp_params.Demodulator_exp_position,
							    &overflow_flag,
							    overflow_comment);

        cur_dest++;

        for( j = 1; j < ABL_bits_per_QAM_symbol[k] / 2; j++ )
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


		      *cur_dest=fxp_mul(*cur_dest,
							    multiplier,
							    fxp_params.Demodulator_precision,
							    fxp_params.Demodulator_exp_position,
							    &overflow_flag,
							    overflow_comment);

          cur_dest++;

          threshold >>= 1;
        }

        threshold = fxp_mul( sim_consts.QAMThresholds[num_of_turn_off_subc][ABL_bits_per_QAM_symbol[k] / 2],
                             channel_sq_amplitudies[k],
                             fxp_params.Demodulator_precision,
                             fxp_params.Demodulator_exp_position,
                             &overflow_flag,
                             overflow_comment );

        last_soft_metric2 = *cur_dest = 
          freq_OFDM_syms[i * NumOfSubcarriers + sim_consts.DataSubcIdx[k] - 1].im;

		    *cur_dest = fxp_mul(*cur_dest,
							    multiplier,
							    fxp_params.Demodulator_precision,
							    fxp_params.Demodulator_exp_position,
							    &overflow_flag,
							    overflow_comment);

        cur_dest++;

        for( j = 1; j < ABL_bits_per_QAM_symbol[k] / 2; j++ )
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

		      *cur_dest=fxp_mul(*cur_dest,
							    multiplier,
							    fxp_params.Demodulator_precision,
							    fxp_params.Demodulator_exp_position,
							    &overflow_flag,
							    overflow_comment);

          cur_dest++;

          threshold >>= 1;
        }
      }
      else
      {
        (*cur_dest) = 
          freq_OFDM_syms[ i * NumOfSubcarriers + sim_consts.DataSubcIdx[k] - 1 ].re;

        cur_dest += ABL_bits_per_QAM_symbol[k];
      }
    }
  }
  //]

  #ifdef LOGGING
  logging( "demodulator_11a finished\n", -1 );
  #endif
}

/*void init_inv_channel_amps_table()
{
  int i;
  double d;
  char overflow_flag;

  for( i = 0; i < (1 << AMP_TABLE_PRECISION); i++ )
  {
    if ( i ) 
    {
      //d = convert_fxp_to_flp( i, 4 );

      d = i;
    }
    else
    {
      //d = convert_fxp_to_flp( 1, 4 );

      d = 1;
    }

    ABL_inv_amps_table[i] =
      convert_flp_to_fxp( 1.0 / d,
                          AMP_TABLE_PRECISION + 1,
                          10,
                          &overflow_flag,
                          overflow_comment );
    if ( overflow_flag )
    {
    //  printf("OF in initing A_SQ_INV_LOG\n");
    }
  }
}

int fxp_div( int a, int b, unsigned precision )
{
	int i;
	int subres;
	int result = 0;
	int sign = ( ( ( a >= 0 ) && ( b >= 0 ) ) || ( ( a <= 0 ) && ( b <= 0 ) ) );

	if ( a < 0 ) a = -a;
	if ( b < 0 ) b = -b;

	for ( i = 0; i < precision; i++ )
	{
		result <<= 1;
		a <<= 1;
		
		subres = a - b;

		if ( subres > 0 )
		{
			result++;
			a = subres;
		}
	}

	if ( sign )
	{
		return result;
	}
	else
	{
		return -result;
	}
}*/
