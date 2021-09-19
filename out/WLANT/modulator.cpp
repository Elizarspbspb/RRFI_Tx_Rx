#include "modulator.h"
#include "sim_params.h"
#include "sim_consts.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: modulator_11a
//
// Purpose: Implements BPSK/QPSK/QAM - modulation
//
// Author: Alexandr Kozlov
//
// Parameters: mod_symbols - output, Modulated symbols from data bits
// 
//						 in_bits - input, Data bits to be modulated
//
//						 data_size - input, Number of data bits  	
//
//             bits_per_symbol - input, Bits per one modulated symbol
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void modulator_11a( fxp_complex* mod_symbols, 
								    unsigned char* in_bits,
                    unsigned data_size,
                    unsigned bits_per_symbol )
{
  unsigned i,j,point_ID1,point_ID2;
  unsigned char* cur_data;

  #ifdef LOGGING
  logging( "modulator_11a...\n", 1 );
  #endif

  cur_data = in_bits;
  for ( i = 0; i < data_size / bits_per_symbol; i++ ) // errors
  {
    point_ID1 = 0;
    point_ID2 = 0;

    if ( bits_per_symbol != 1 )
    {
      //for( j = 0; j < bits_per_symbol / 2; j++ )
      //{
      //   point_ID1 <<= 1;
      //   point_ID1 |= (cur_data[2 * j] > 0) ? 1 : 0;
      //   point_ID2 <<= 1;
      //   point_ID2 |= (cur_data[2 * j + 1] > 0) ? 1 : 0;
      //}

      for(  j = 0; j < bits_per_symbol / 2; j++ )
      {
         point_ID1 |= ( (unsigned)(*cur_data) ) << (bits_per_symbol / 2 - 1 - j);
         cur_data ++;
      }

      for( j = 0; j < bits_per_symbol / 2; j++ )
      {
         point_ID2 |= ( (unsigned)(*cur_data) ) << (bits_per_symbol / 2 - 1 - j);
         cur_data ++;
      }

      
      mod_symbols[i].re = sim_consts.QAMSignalLevels[0][bits_per_symbol/2][point_ID1];

      mod_symbols[i].im = sim_consts.QAMSignalLevels[0][bits_per_symbol/2][point_ID2];
    }
    else
    {
      if ( *cur_data > 0 )
      {
        mod_symbols[i].re = sim_consts.QAMSignalLevels[0][0][1];
        mod_symbols[i].im = 0;
        cur_data ++;
      }
      else
      {
        mod_symbols[i].re = sim_consts.QAMSignalLevels[0][0][0];
        mod_symbols[i].im = 0;
        cur_data ++;
      }
    }
      
   // cur_data += bits_per_symbol;
  }

  #ifdef LOGGING
  logging( "modulator_11a finished\n", -1 );
  #endif
}

