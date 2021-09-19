#include "pilots_insertion.h"
#include "model.h"
#include "sim_consts.h"
#include "sim_params.h"
#include <memory.h>

/**
////////////////////////////////////////////////////////////////////////////////
// Name: insert_pilot_symbols_11a
//
// Purpose: Add pilots symbols to the modulated symbols
//
// Author: Alexandr Kozlov
//
// Parameters: ofdm_symbols - output, OFDM symbols in frequency domain
// 
//						 mod_symbols - input, Modulated symbols
//
//             num_of_OFDM_symbols - input, number of OFDM symbols
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void insert_pilot_symbols_11a( fxp_complex* ofdm_symbols,
											         fxp_complex* mod_symbols, 
                               unsigned num_of_OFDM_symbols )
{
  unsigned i,j;
	unsigned symbolIx;

  #ifdef LOGGING
  logging( "insert_pilot_symbols_11a...\n", 1 );
  #endif  
  
  memset( ofdm_symbols,
          0, 
          num_of_OFDM_symbols * NumOfSubcarriers * sizeof( fxp_complex ) );

  for( i = 0; i < num_of_OFDM_symbols; i++ )
  {
    for( j = 0; j < NumOfDataSubcarriers; j++ )
    {
      symbolIx = i * NumOfSubcarriers + sim_consts.DataSubcIdx[j] - 1;         

      ofdm_symbols[symbolIx] = mod_symbols[i * NumOfDataSubcarriers + j];
    }
    
		for( j = 0; j < NumOfPilotSubcarriers; j++ )
		{
      symbolIx = i * NumOfSubcarriers + sim_consts.PilotSubcIdx[j] - 1;
			
  		ofdm_symbols[symbolIx].re = 
        find_closest_fxp_value( sim_consts.PilotScramble[i%127] * sim_consts.PilotSubcSymbols[j],
                                fxp_params.QAM_precision,
                                2.16 / ( 1 << fxp_params.QAM_precision ) );

      ofdm_symbols[symbolIx].im = 0;
		}
  }

  #ifdef LOGGING
  logging( "insert_pilot_symbols_11a finished\n", -1 );
  #endif  
}
