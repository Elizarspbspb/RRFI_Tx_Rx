#include "modulatorABL.h"
#include "sim_params.h"
#include "sim_consts.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: modulator_for_ABL
//
// Purpose: Implements BPSK/QPSK/QAM - modulation for ABL
//
// Author: Alexandr Kozlov
//
// Parameters: mod_symbols - output, Modulated symbols from data bits
// 
//						 in_bits - input, Data bits to be modulated
//
//						 data_size - input, Number of data bits  	
//
//             bits_per_QAM_symbol - input, bits per symbol for data subcarriers
//
//             num_of_OFDM_symbols - input, Number of OFDM symbols
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void modulator_for_ABL( fxp_complex* mod_symbols, 
								        unsigned char* in_bits, 
                        unsigned data_size,
                        unsigned* bits_per_QAM_symbol,
                        unsigned num_of_OFDM_symbols )
{
  unsigned i,j,k,point_ID1,point_ID2;
  unsigned char* cur_data;

  unsigned num_of_turn_off_subc = 0;
  
  #ifdef LOGGING
  logging( "modulator_for_ABL...\n", 1 );
  #endif

  //[ Calculate number of turn off subcarriers
  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
    if ( bits_per_QAM_symbol[i] == 0 )
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
  
  cur_data = in_bits;

  for ( i = 0; i < num_of_OFDM_symbols; i++ )
  {
    for ( j = 0; j < NumOfDataSubcarriers; j++ )
    {
      if ( bits_per_QAM_symbol[j] == 1 )
      {
        if ( *cur_data > 0 )
        {
          mod_symbols[i * NumOfDataSubcarriers + j].re =
            sim_consts.QAMSignalLevels[num_of_turn_off_subc][0][1];
          mod_symbols[i * NumOfDataSubcarriers + j].im = 0;

          cur_data++;
        }
        else
        {
          mod_symbols[i * NumOfDataSubcarriers + j].re =
            sim_consts.QAMSignalLevels[num_of_turn_off_subc][0][0];
          mod_symbols[i * NumOfDataSubcarriers + j].im = 0;

          cur_data++;
        }        
      }
      else
      if ( bits_per_QAM_symbol[j] != 0 )
      {
        point_ID1 = 0;
        point_ID2 = 0;

        /*for ( k = 0; k < bits_per_QAM_symbol[j] / 2; k++ )
        {
          point_ID1 <<= 1;
          point_ID1 |= (*cur_data > 0) ? 1 : 0;
          cur_data++;

          point_ID2 <<= 1;
          point_ID2 |= (*cur_data > 0) ? 1 : 0;
          cur_data++;
        }*/

        for(  k = 0; k < bits_per_QAM_symbol[j] / 2; k++ )
        {
          point_ID1 |= ( (unsigned)(*cur_data) ) << (bits_per_QAM_symbol[j] / 2 - 1 - k);
          cur_data ++;
        }

        for( k = 0; k < bits_per_QAM_symbol[j] / 2; k++ )
        {
          point_ID2 |= ( (unsigned)(*cur_data) ) << (bits_per_QAM_symbol[j] / 2 - 1 - k);
          cur_data ++;
        }

        mod_symbols[ i * NumOfDataSubcarriers + j ].re = 
          sim_consts.QAMSignalLevels[num_of_turn_off_subc][bits_per_QAM_symbol[j] / 2][point_ID1];
        mod_symbols[ i * NumOfDataSubcarriers + j ].im =  
          sim_consts.QAMSignalLevels[num_of_turn_off_subc][bits_per_QAM_symbol[j] / 2][point_ID2];
      }
      else
      {
        mod_symbols[ i * NumOfDataSubcarriers + j ].re = 0;
        mod_symbols[ i * NumOfDataSubcarriers + j ].im = 0; 
      }
    }
  }

  #ifdef LOGGING
  logging( "modulator_for_ABL finished\n", -1 );
  #endif
}

