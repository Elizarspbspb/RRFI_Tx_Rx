#include "add_preamble.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "fft.h"
#include "model.h"
#include <memory.h>
#include "commonfunc.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: add_preamble_11a
//
// Purpose: Add short and long training symbols to time domain signal
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - output, Time domain signal
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void add_preamble_11a( fxp_complex* time_signal )
{
  int i,j;
  fxp_complex* cur_dest;
  unsigned cur_index;

  fxp_complex short_training_seq_time[NumOfSubcarriers];
  fxp_complex long_training_seq_time[NumOfSubcarriers];

  #ifdef LOGGING
  logging( "add_preamble_11a...\n", 1 );
  #endif

  //[ Prepare training sequences in time domain
  memset( short_training_seq_time, 0, NumOfSubcarriers * sizeof( fxp_complex ) );
	memset( long_training_seq_time, 0, NumOfSubcarriers * sizeof( fxp_complex ) );

	for ( i = 0; i < NumOfUsedSubcarriers; i++ )
	{
	  short_training_seq_time[sim_consts.UsedSubcIdx[i] - 1].re = 
      find_closest_fxp_value( sim_consts.ShortTrainingSymbols[i].dat[0],
                              fxp_params.QAM_precision,
                              2.16 / ( 1 << fxp_params.QAM_precision ) );

    short_training_seq_time[sim_consts.UsedSubcIdx[i] - 1].im = 
      find_closest_fxp_value( sim_consts.ShortTrainingSymbols[i].dat[1],
                              fxp_params.QAM_precision,
                              2.16 / ( 1 << fxp_params.QAM_precision ) );

    if ( sim_consts.LongTrainingSymbols[i] > 0 )
	    long_training_seq_time[sim_consts.UsedSubcIdx[i] - 1].re =
        sim_consts.QAMSignalLevels[0][0][1];
    else
	    long_training_seq_time[sim_consts.UsedSubcIdx[i] - 1].re =
        sim_consts.QAMSignalLevels[0][0][0];

    long_training_seq_time[sim_consts.UsedSubcIdx[i] - 1].im = 0;
	}

  #ifdef TEST_SEQ
  dump_fxp_complex_array( short_training_seq_time, 
                          NumOfSubcarriers,
                          "STS_freq.log",
                          "Short training symbols in frequency domain" );
  #endif

  #ifdef TEST_SEQ
  dump_fxp_complex_array( long_training_seq_time, 
                          NumOfSubcarriers,
                          "LTS_freq.log",
                          "Long training symbols in frequency domain" );
  #endif

  fft_with_mapping_11a( short_training_seq_time, 6, 1,
                        fxp_params.FFT_precision,
                        fxp_params.FFT_exp_position, 
                        fxp_params.IFFT_shift_control );

  fft_with_mapping_11a( long_training_seq_time, 6, 1,
                        fxp_params.FFT_precision,
                        fxp_params.FFT_exp_position,
                        fxp_params.IFFT_shift_control );

  #ifdef TEST_SEQ
  dump_fxp_complex_array( short_training_seq_time, 
                          NumOfSubcarriers,
                          "STS_time.log",
                          "Short training symbols in time domain" );
  #endif

  #ifdef TEST_SEQ
  dump_fxp_complex_array( long_training_seq_time, 
                          NumOfSubcarriers,
                          "LTS_time.log",
                          "Long training symbols in time domain" );
  #endif
  //]

  //[ Add short training symbols
	cur_dest = time_signal;
	
	for ( i = 0; i < 10; i++ )
	{
		for ( j = 0; j < 16; j++ )
		{
			*cur_dest = short_training_seq_time[j];
			cur_dest++;
		}
	}
  //]

	//[ Add long training symbols
	for( i = - 2 * CyclicPrefixLength; i < 0; i++ )
	{
		 cur_index  = (i + NumOfSubcarriers) % (NumOfSubcarriers);
		 *cur_dest = long_training_seq_time[ cur_index ];
  	 cur_dest++;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < NumOfSubcarriers; j++ )
		{
			*cur_dest = long_training_seq_time[j];
			cur_dest++;
		}
	}
  //]

  #ifdef LOGGING
  logging( "add_preamble_11a finished\n", -1 );
  #endif
}
