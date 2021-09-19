#include "add_cyclic_prefix.h"
#include "sim_consts.h"
#include "model.h"

/*
////////////////////////////////////////////////////////////////////////////////
// Name: add_cyclic_prefix_11a
//
// Purpose: Add cyclic prefix to each time domain OFDM symbol
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - output, Time domain symbols with cyclic prefix
// 
//						 time_sym - input, Time domain symbols
//
//             num_of_OFDM_symbols - input, number of OFDM symbols
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void add_cyclic_prefix_11a( fxp_complex* time_signal,
   												  fxp_complex* time_sym,
                            unsigned num_of_OFDM_symbols )
{
  int i, j;
  unsigned  cur_index; 
  fxp_complex* dest = time_signal;

  #ifdef LOGGING
  logging( "add_cyclic_prefix_11a...\n", 1 );
  #endif

  for ( i = 0; i < num_of_OFDM_symbols; i++ )
  {
    for( j = -CyclicPrefixLength; j < NumOfSubcarriers; j++)
    {
       cur_index = (j + NumOfSubcarriers) % (NumOfSubcarriers);

       *dest = time_sym[ i * NumOfSubcarriers + cur_index ];
       dest++;
    }
  }

  #ifdef LOGGING
  logging( "add_cyclic_prefix_11a finished\n", -1 );
  #endif
}
  