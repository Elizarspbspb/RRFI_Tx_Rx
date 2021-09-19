/* File History
 * $History: transmitter.c $
 * 
 * *****************  Version 12  *****************
 * User: Akozlov      Date: 2.09.05    Time: 13:26
 * Updated in $/WLAN
 * 
 * *****************  Version 11  *****************
 * User: Akozlov      Date: 28.07.05   Time: 19:01
 * Updated in $/WLAN
 * Golay code for header added
 * 
 * *****************  Version 10  *****************
 * User: Akozlov      Date: 24.12.04   Time: 19:43
 * Updated in $/WLAN
 * Advanced training added
 * 
 */

#include "model.h"
#include "transmitter.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include <memory.h>
#include <string.h>
#include <math.h>
#include "fft.h"
#include "scrambler.h"
#include "conv_encoder.h"
#include "interleaver.h"
#include "modulator.h"
#include "puncturer.h"
#include "pilots_insertion.h"
#include "add_preamble.h"
#include "add_cyclic_prefix.h"
#include "DAC.h"
#include "ldpc_encode.h"
#include "modulatorABL.h"
#include "golay_wrap.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: transmitter
//
// Purpose: Implements OFDM transmitter scheme: FEC coding, modulation,
//					adding preamble and training symbols, generating time domain signal
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - output, Time domain signal stores 
//
//						 inf_bits - input, data bits to be transmitted	
//           
//             packet_size - input, Packet size in bytes
//
//             ABL_bits_per_QAM_symbol - input, Subcarriers modulation for ABL
//
//             bits_per_OFDM_symbol - input, Coded bits per OFDM symbol
//  
// Returning value: Number of time domain samples
//////////////////////////////////////////////////////////////////////////////// */
unsigned transmitter( gsl_complex* time_signal, 
                      unsigned char* inf_bits, 
                      unsigned packet_size,
                      unsigned* ABL_bits_per_QAM_symbol,
                      unsigned bits_per_OFDM_symbol )
{
  unsigned i,j;

  unsigned char data_bits[MaxPacketSize+LDPCMessageLength];
  unsigned char coded_bits[MaxCodedPacketSize];
  unsigned char interl_bits[MaxCodedPacketSize];
  unsigned char punctured_bits[MaxCodedPacketSize];

  unsigned num_of_coded_bits;
  unsigned num_of_OFDM_symbols;
  unsigned num_of_pad_bits = 0;
  unsigned num_of_samples;
  unsigned num_of_LDPC_codewords;
  unsigned num_of_LDPC_zerobits;

  fxp_complex mod_symbols[48 + MaxCodedPacketSize]; // PLCP symbol + data symbols
  fxp_complex ofdm_symbols[MaxNumOfSymbols * NumOfSubcarriers];

  fxp_complex fxp_time_signal[PreambleLength + MaxNumOfSymbols * 
                              ( CyclicPrefixLength + NumOfSubcarriers )];

  unsigned num_of_total_bits;

  #ifdef LOGGING
  logging( "Transmitter...\n", 1 );
  #endif

  //[ Prepare PLCP header
  #ifdef LOGGING
  logging( "Prepare PLCP header...\n", 1 );
  #endif

  memset( data_bits, 0, 24 );
  get_PLCP_header( data_bits, packet_size );

  #ifdef TEST_SEQ
  dump_char_array( data_bits, 24, "PLCP_header.log", "PLCP header" );
  #endif

  if ( sim_params.useGolay )
  {
    GolayEncode( golay_handle, data_bits, coded_bits );
    GolayEncode( golay_handle, data_bits + 12, coded_bits + 24 );
  }
  else
    conv_encoder_11a( data_bits, 24, coded_bits );

  #ifdef TEST_SEQ
  dump_char_array( coded_bits, 48, "PLCP_header_coded.log", "PLCP header after encoding" );
  #endif

  interleaver_11a( coded_bits, interl_bits, 48, 48 );

  #ifdef TEST_SEQ
  dump_char_array( interl_bits, 48, "PLCP_header_interl.log", "PLCP header after interleaving" );
  #endif

  modulator_11a( mod_symbols, interl_bits, 48, 1 );

  #ifdef LOGGING
  logging( "Prepare PLCP header finished\n", -1 );
  #endif
  //]

  //[ Add SERVICE field to data bits and scramble its
  memset( data_bits, 0, 16 );
  memcpy( data_bits + 16, inf_bits, 8 * packet_size );
  //]

  //[ Add tail bits
  memset( data_bits + 16 + 8 * packet_size, 0, 6 );
  //]

  //[ Padding
  if ( !sim_params.useLDPC && !sim_params.useGolay )
  {
    //[ Add padding bits to the information bits in convolutional code case
    num_of_OFDM_symbols = (unsigned)ceil( (double)( 16 + 8 * packet_size + 6 ) /
                                          (double)(bits_per_OFDM_symbol * sim_params.code_rate) );

    num_of_pad_bits = (unsigned)( num_of_OFDM_symbols * bits_per_OFDM_symbol * sim_params.code_rate -
                                  ( 16 + 8 * packet_size + 6 ) );

    for ( i = 0; i < num_of_pad_bits; i++ )
    {
      data_bits[ 16 + 8 * packet_size + 6 + i ] = 0;
    }

    num_of_total_bits = 16 + 8 * packet_size + 6 + num_of_pad_bits;
    //]
  }
  else
  if ( sim_params.useGolay )
  {
    //[ Add padding bits to the information bits in Golay code case
    num_of_OFDM_symbols = (unsigned)ceil( (double)( 16 + 8 * packet_size ) /
                                          (double)(bits_per_OFDM_symbol * 0.5) );

    num_of_pad_bits = (unsigned)( num_of_OFDM_symbols * bits_per_OFDM_symbol * 0.5 -
                                  ( 16 + 8 * packet_size ) );

    for ( i = 0; i < num_of_pad_bits; i++ )
    {
      data_bits[ 16 + 8 * packet_size + i ] = 0;
    }

    num_of_total_bits = 16 + 8 * packet_size + num_of_pad_bits;
    //]
  }
  else
  {
    //[ Add padding bits to the information bits in LDPC code case
    num_of_LDPC_zerobits = LDPCMessageLength - ( 8 * packet_size + 16 ) % LDPCMessageLength;

    if ( num_of_LDPC_zerobits == LDPCMessageLength )
    {
      num_of_LDPC_codewords = ( 8 * packet_size + 16 ) / LDPCMessageLength;
    }
    else
    {
      num_of_LDPC_codewords = ( 8 * packet_size + 16 ) / LDPCMessageLength + 1;
    }

    //[ Add padding bits for last message
    for ( i = 0; i < num_of_LDPC_zerobits; i++ )
    {
      data_bits[ num_of_LDPC_codewords * LDPCMessageLength -
                 num_of_LDPC_zerobits + i ] = 0;
    }
    //]
 
    //[ Add pad bits for LDPC case due to LDPC codeword length may not divide by NumOfDataSubcarriers
    num_of_coded_bits = num_of_LDPC_codewords * LDPCCodewordLength;

    num_of_OFDM_symbols = (unsigned)ceil( (double)num_of_coded_bits / 
                                          bits_per_OFDM_symbol );

    num_of_pad_bits = num_of_OFDM_symbols * bits_per_OFDM_symbol - num_of_coded_bits;

    for ( i = 0; i < num_of_pad_bits; i++ )
    {
      data_bits[ num_of_LDPC_codewords * LDPCMessageLength + i ] = 0;
    }
    //]

    num_of_total_bits = num_of_LDPC_codewords * LDPCMessageLength + num_of_pad_bits;
    //]
  }
  //]

  #ifdef TEST_SEQ
  dump_char_array( data_bits, 
                   num_of_total_bits,
                   "data_bits.log",
                   "DATA bits" );
  #endif

  //[ Scrambling
  scrambler_11a( data_bits, num_of_total_bits, &tx_scrambler_state, 0 );
  //]
  
  if ( !sim_params.useLDPC && !sim_params.useGolay )
  {
    //[ Set tail bits to zero
    memset( data_bits + 16 + 8 * packet_size, 0, 6 );
    //]
  }

  //]

  #ifdef TEST_SEQ
  dump_char_array( data_bits, 
                   num_of_total_bits,
                   "data_bits_scrambled.log",
                   "DATA bits after scrambling" );
  #endif

  //[ Encode data
  if ( !sim_params.useLDPC && !sim_params.useGolay )
  {
    //[ Encode data using convolutional code
    num_of_coded_bits = 
      conv_encoder_11a( data_bits, 16 + 8 * packet_size + 6 + num_of_pad_bits, coded_bits );
    num_of_coded_bits = 
      puncturer_11a( coded_bits,
                     num_of_coded_bits,
                     punctured_bits,
                     sim_params.code_rate );

    #ifdef TEST_SEQ
    dump_char_array( punctured_bits, 
                     num_of_coded_bits,
                     "data_bits_coded.log",
                     "Coded DATA bits" );
    #endif
    //]
  }
  else
  if ( sim_params.useGolay )
  {
    // Golay encode
    num_of_coded_bits = 2 * num_of_total_bits;
    for( i = 0; i < 2 * num_of_OFDM_symbols; i++ )
    {
      GolayEncode( golay_handle, data_bits + 12 * i, punctured_bits + 24 * i );
    }
  }
  else
  {
    //[ Encode data using  LDPC code
    for ( i = 0; i < num_of_LDPC_codewords; i++ )
    {
      /*ldpc_encode( data_bits + i * LDPCMessageLength,
                   punctured_bits + i * LDPCCodewordLength );*/
        ldpc_encode((const char*)data_bits + i * LDPCMessageLength,
            (char*)punctured_bits + i * LDPCCodewordLength);
    }

    memcpy( punctured_bits + num_of_LDPC_codewords * LDPCCodewordLength, 
            data_bits + num_of_LDPC_codewords * LDPCMessageLength, num_of_pad_bits );

    #ifdef TEST_SEQ
    dump_char_array( punctured_bits, 
                     num_of_coded_bits + num_of_pad_bits,
                     "data_bits_coded_LDPC.log",
                     "LDPC Coded DATA bits" );
    #endif
    //]
  }
  //]

  if ( !sim_params.useLDPC )
  {
    //[ Interleave bits
    interleaver_11a( punctured_bits, 
                     interl_bits, 
                     num_of_OFDM_symbols * bits_per_OFDM_symbol,
                     bits_per_OFDM_symbol );

    #ifdef TEST_SEQ
    dump_char_array( interl_bits, 
                     num_of_coded_bits,
                     "data_bits_inter.log",
                     "DATA bits after interleaving" );
    #endif
    //]
  }

  num_of_OFDM_symbols ++; // due to PLCP header symbol

  //[ Modulate data bits to the mod_symbols array, mod_symbols precision is equal to fxp_params.QAM_precision
  if ( !sim_params.useABL )
  {
    modulator_11a( mod_symbols + NumOfDataSubcarriers, 
                   sim_params.useLDPC ? punctured_bits : interl_bits,
                   sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits, 
                   sim_params.bits_per_QAM_symbol );
  }
  else
  {
		#ifdef ABL_TEST
		dump_char_array( sim_params.useLDPC ? punctured_bits : interl_bits, 2*bits_per_OFDM_symbol, "Data_before_ABL_modulator.log", "Data_before_ABL_modulator" );
		#endif

    modulator_for_ABL( mod_symbols + NumOfDataSubcarriers, 
                       sim_params.useLDPC ? punctured_bits : interl_bits,
                       sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits,
                       ABL_bits_per_QAM_symbol,
                       num_of_OFDM_symbols - 1 );

	  #ifdef ABL_TEST
	  dump_fxp_complex_array( mod_symbols, 3*NumOfSubcarriers, "Data_after_ABL_modulator.log", "Data_after_ABL_modulator" );
    #endif
  }
  //]

  //[ Insert pilots symbols, ofdm_symbols precision is equal to fxp_params.QAM_precision
  insert_pilot_symbols_11a( ofdm_symbols, mod_symbols, num_of_OFDM_symbols );
  //]

  if ( sim_params.advTraining )
  {
    memmove( ofdm_symbols + 3 * NumOfSubcarriers,
             ofdm_symbols + NumOfSubcarriers,
             ( num_of_OFDM_symbols - 1 ) * NumOfSubcarriers * sizeof( fxp_complex ) );

	  memset( ofdm_symbols + NumOfSubcarriers, 0, 2 * NumOfSubcarriers * sizeof( fxp_complex ) );

    for ( j = 0; j < 2; j++ )
	    for ( i = 0; i < NumOfUsedSubcarriers; i++ )
	    {
        if ( sim_consts.LongTrainingSymbols[i] > 0 )
	        ofdm_symbols[ ( j + 1 ) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].re =
            sim_consts.QAMSignalLevels[0][0][1];
        else
	        ofdm_symbols[ ( j + 1 ) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].re =
            sim_consts.QAMSignalLevels[0][0][0];

        ofdm_symbols[ ( j + 1 ) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].im = 0;
	    }

    num_of_OFDM_symbols += 2;
  }


  #ifdef TEST_SEQ
  dump_fxp_complex_array( ofdm_symbols, 
                          NumOfSubcarriers,
                          "freq_PLCP_header.log",
                          "PLCP header in frequency domain" );
  #endif

  #ifdef TEST_SEQ
  dump_fxp_complex_array( ofdm_symbols + NumOfSubcarriers, 
                          ( num_of_OFDM_symbols - 1 ) * NumOfSubcarriers,
                          "freq_DATA.log",
                          "DATA in frequency domain" );
  #endif

  //[ Convert frequency domain symbols to time domain signal
  for ( i = 0; i < num_of_OFDM_symbols; i++ )
  {
    // Set ofdm_symbols precision to fxp_params.FFT_precision 
    // and exp. position to fxp_params.FFT_exp_position to avoid any fxp convertions
    fft_with_mapping_11a( ofdm_symbols + i * NumOfSubcarriers, 6, 1, 
                          fxp_params.FFT_precision,
                          fxp_params.FFT_exp_position,
                          fxp_params.IFFT_shift_control );
    // After IFFT ofdm_symbols have (fxp_params.FFT_precision - fxp_params.IFFT_shift_control) precision,
    // after IFFT exp. position doesn't matter for it
  }
  //]

  #ifdef TEST_SEQ
  dump_fxp_complex_array( ofdm_symbols, 
                          NumOfSubcarriers,
                          "time_PLCP.log",
                          "PLCP header in time domain" );
  #endif

  #ifdef TEST_SEQ
  dump_fxp_complex_array( ofdm_symbols + NumOfSubcarriers, 
                          ( num_of_OFDM_symbols - 1 ) * NumOfSubcarriers,
                          "time_DATA.log",
                          "DATA in time domain" );
  #endif

  //[ Add preamble
  add_preamble_11a( fxp_time_signal );
  //]

  //[ Add cyclic prefix 
  add_cyclic_prefix_11a( fxp_time_signal + PreambleLength, 
                         ofdm_symbols,
                         num_of_OFDM_symbols );
  //]

  num_of_samples = PreambleLength + num_of_OFDM_symbols *
                   ( CyclicPrefixLength + NumOfSubcarriers );

  #ifdef TEST_SEQ
  dump_fxp_complex_array( fxp_time_signal, 
                          num_of_samples,
                          "entire_packet.log",
                          "DATA in time domain" );
  #endif
  
  //[ Digital to analog convertion
  num_of_samples = DAC_11a( time_signal,
                            fxp_time_signal,
                            fxp_params.FFT_precision, 
                            fxp_params.FFT_exp_position,
                            num_of_samples );
  //]

  #ifdef LOGGING
  logging( "Transmitter finished\n", -1 );
  #endif

  return num_of_samples;
}