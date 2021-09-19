#include "remove_cyclic_prefix.h"
#include "sim_consts.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: remove_cyclic_prefix_11a
//
// Purpose: Remove cyclic prefix
//
// Author: Alexandr Kozlov
//
// Parameters: time_OFDM_symbols - output, Time domain OFDM symbols
//
//						 time_signal - input, Received time domain signal
//
//             num_of_symbols - input, Number of OFDM symbols
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void remove_cyclic_prefix_11a( fxp_complex* time_OFDM_symbols,
		                           fxp_complex* time_signal,
                               unsigned num_of_symbols )
{
  unsigned i, j;
  unsigned cur_index;

  #ifdef LOGGING
  logging( "remove_cyclic_prefix_11a...\n", 1 );
  #endif

  for ( i = 0; i < num_of_symbols; i++ )
  {
    for( j = 0; j < NumOfSubcarriers ; j++)
    {
		  cur_index = 
        i * (NumOfSubcarriers + CyclicPrefixLength) + j + CyclicPrefixLength;
      
      time_OFDM_symbols[i * NumOfSubcarriers + j] = time_signal[ cur_index ];
    }
  }

  #ifdef LOGGING
  logging( "remove_cyclic_prefix_11a finished\n", -1 );
  #endif
}
