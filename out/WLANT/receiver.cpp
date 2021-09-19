/* File History
 * $History: receiver.c $
 * 
 * *****************  Version 24  *****************
 * User: Akozlov      Date: 2.09.05    Time: 13:27
 * Updated in $/WLAN
 * 
 * *****************  Version 23  *****************
 * User: Akozlov      Date: 17.01.05   Time: 12:39
 * Updated in $/WLAN
 * 
 * *****************  Version 22  *****************
 * User: Akozlov      Date: 24.12.04   Time: 19:43
 * Updated in $/WLAN
 * Advanced training added
 * 
 * *****************  Version 21  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:51
 * Updated in $/WLAN
 * 
 * *****************  Version 20  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:51
 * Updated in $/WLAN
 * 
 * *****************  Version 19  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:45
 * Updated in $/WLAN
 * Decoders input accuracy convertion added
 * 
 * *****************  Version 18  *****************
 * User: Akozlov      Date: 2.12.04    Time: 12:21
 * Updated in $/WLAN
 * 
 * *****************  Version 17  *****************
 * User: Akozlov      Date: 1.12.04    Time: 18:06
 * Updated in $/WLAN
 * Padding remove bug fixed
 *
 * *****************  Version 17  *****************
 * User: Eegorov      Date: 30.07.21    Time: 15:10
 * Updated in $/WLAN
 * Padding remove bug fixed
 *
 */

//#include "receiver.h"
#include "model.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include "find_packet_edge.h"
#include "agc.h"
#include "VGA.h"
#include "ADC.h"
#include "extract_LTS.h"
#include "channel_estimation.h"
#include "fft.h"
#include "equalizer.h"
#include "demodulator.h"
#include "deinterleaver.h"
#include "conv_decoder.h"
#include "depuncturer.h"
#include "remove_cyclic_prefix.h"
#include "scrambler.h"
#include "time_sync.h"
#include "frequency_sync.h"
#include "fine_freq_sync.h"
#include "rotor.h"
#include "phase_tracker.h"
#include "ldpc_decode_fxp.h"
#include "bitloading.h"
#include "demodulatorABL.h"
#include "Decimator.h"
#include "Interpolation.h"
#include <math.h>
#include <memory.h>
#include <iostream>
#include <fstream>
#include "hufc.h"
#include "golay_wrap.h"
using namespace std;

/**
////////////////////////////////////////////////////////////////////////////////
// Name: receiver
//
// Purpose: Implements OFDM receiver scheme: Synchronization, demodulation,
//          channel estimation, FEC decoding
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input, Received time domain signal stores 
//
//						 num_of_samples - input, Number of received samples
//
//						 out_bits - output, Demodulated and decoded data bits
//
//             ABL_bits_per_QAM_symbol - input/output, Subcarriers modulation
//                                                     for ABL
//
//             bits_per_OFDM_symbol - input/output, Coded bits per OFDM symbol
//
//             syncError - output, If 1 error of synchronization
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */

//void receiver(gsl_complex* time_signal,  //
void receiver(fxp_complex* fxp_time_signal,
               unsigned num_of_samples,
               unsigned char* out_bits,
               unsigned* ABL_bits_per_QAM_symbol,
               unsigned* bits_per_OFDM_symbol,
               char* syncError,
               unsigned* feedBackNumOfOFDMSyms
             )
{
    printf("RECEIVER...");

    memset(out_bits, 0, num_of_samples);
    printf("\nnum_of_samples = %d\n", num_of_samples);


  unsigned i,j;

  fxp_complex freq_tr_syms[2 * NumOfUsedSubcarriers];

  fxp_complex channel_estimate[NumOfUsedSubcarriers];

  fxp_complex freq_OFDM_syms[NumOfSubcarriers * MaxNumOfSymbols];

  int soft_bits[MaxCodedPacketSize];
  int deinterl_bits[MaxCodedPacketSize];
  int depunctured_bits[MaxCodedPacketSize];
  unsigned char decoded_bits[MaxCodedPacketSize];
  unsigned char codeword[LDPCCodewordLength];
  int reliabilities[LDPCCodewordLength];
  memset(decoded_bits, 0, MaxCodedPacketSize);


  unsigned char PLCP_header[24];
  int PLCP_error = 0;
  unsigned num_of_bits;
  unsigned num_of_OFDM_symbols;
  unsigned bits_per_QAM_symbol;
  double code_rate;

  unsigned SERVICE;

  int radians_per_sample_coarse = 0;
  int radians_per_sample_fine = 0;

  unsigned cur_sample = 0;
  unsigned cur_fxp_sample = 0;
  unsigned cur_detect_sample = 0;
  unsigned packet_start_sample = 0;
	unsigned AGCFreeze_sample = 0;
	unsigned VGA_sample = 0;
  unsigned start_sample = 0;
  unsigned num_of_pad_bits = 0;

  char packet_detected = 0;
  char edge_found = 0;
  char num_of_edges = 0;
  unsigned num_of_clippings = 0;
	unsigned last_clipping_sample = 0;
  unsigned num_of_LDPC_codewords;

  int phase_sym_err = 0;
  int phase_subc_err = 0;
  int rotor_correction = 0;

  int channel_sq_amplitudies[NumOfDataSubcarriers];

  float profile[NumOfDataSubcarriers];  

  fxp_complex additional_freq_tr_syms[2 * NumOfUsedSubcarriers];
  fxp_complex additional_channel_estimate[NumOfUsedSubcarriers];
  char overflow_flag;


  char useABL;
  char useLDPC;

  int Bytes,Bits;
  unsigned char ucharABL_bits_per_QAM_symbol[NUM_SYMBOLS];
  unsigned char compressedABL_bits_per_QAM_symbol[1000];

  #ifdef LOGGING
  logging( "Receiver...\n", 1 );
  #endif

  *syncError = 0;

	//[ Set VGA gain to maximum
	VGA_gain = 127;
	curVGA_gain = 127;
	//]

  if( sim_params.useJitter )
  {
      //printf("Drift_And_Jitter\n");
    //[ Apply drift and jitter
      //num_of_samples = Drift_And_Jitter( time_signal, num_of_samples ); // Было так
      //num_of_samples = Drift_And_Jitter((gsl_complex*)time_signal, num_of_samples);
    //]
  }

  //[ Decimate oversampled signal to 80MHz
	for ( i = 0; i < sim_params.sampling_power - 2; i++ )
	{
		//num_of_samples = Decimator( time_signal, num_of_samples ); // Было так
        //num_of_samples = Decimator((gsl_complex*)time_signal, num_of_samples);
	}
	//]

  //////////////////////////////////////// packet detection routine ///////////////////////////////////////

    //AGCPacketDetectionBlock(time_signal, fxp_time_signal, num_of_samples, // Было так
    //AGCPacketDetectionBlock((gsl_complex*)time_signal, fxp_time_signal, num_of_samples,
		//&packet_start_sample, &AGCFreeze_sample, &VGA_sample );

	//packetDetection(time_signal, num_of_samples, &packet_start_sample, &VGA_gain, 12, 11, 1, 1, 48, 64, 150);
  //curVGA_gain = VGA_gain;
	//VGA_sample = packet_start_sample;


	
	//printf( " packet_start_sample=%d, VGA_gain=%d\n", packet_start_sample, VGA_gain );

  //[ Packet detection error
  /*if ( ( packet_start_sample > ( (ExtraNoiseSamples >> ( sim_params.sampling_power - 2 ) ) + 4 * 160 ) )	||
			 ( packet_start_sample < ( (ExtraNoiseSamples >> ( sim_params.sampling_power - 2 ) ) - 32 ) ) )
  {
    packet_start_sample = 0;
     *syncError = 1;
  }*/
  //]

	//[ Analog-to-Digital convertion
      //VGA_11a(time_signal + VGA_sample, // Было так
    //VGA_11a((gsl_complex*)time_signal + VGA_sample,
           //num_of_samples - VGA_sample );        

  //[ Decimation from 80 MHz to 20 MHz
  //num_of_samples = Decimator( time_signal + packet_start_sample, num_of_samples - packet_start_sample ); // Было так
  //num_of_samples = Decimator( time_signal + packet_start_sample, num_of_samples ); // Было так
    //num_of_samples = Decimator((gsl_complex*)time_signal + packet_start_sample, num_of_samples - packet_start_sample);
    //num_of_samples = Decimator((gsl_complex*)time_signal + packet_start_sample, num_of_samples );
  //]

    //ADC_11a(time_signal + packet_start_sample, // Было так
    //ADC_11a((gsl_complex*)time_signal + packet_start_sample,
           //fxp_time_signal, 
           //num_of_samples );

  // After ADC fxp_time_signal have fxp_params.ADC_DAC_precision,
  // exp. position doesn't matter for it here, it will be set later
  //]

  if ( !sim_params.idealFrequencySync )
  {
      //printf("idealFrequencySync\n");
    //[ Coarse frequency error estimation
    radians_per_sample_coarse = // here radians_per_sample precision is fxp_params.Arctan_precision
      coarse_frequency_sync_11a( fxp_time_signal );
    //]

    //[ Correct frequency offset
    rotor_state_angle = 0;
	  rotor_11a( fxp_time_signal,
						   radians_per_sample_coarse,
						   num_of_samples );
    //]
  }
  //printf("num_of_samples_after_receivers = %d\n", num_of_samples);
  //printf("\nfxp_signal_after correct frequency offset\n");

  //[ Timing synchronization
  if ( !sim_params.idealTiming )
  {
      std::cout << "\n idealTiming " << std::endl;
    //start_sample = (ExtraNoiseSamples >> sim_params.sampling_power) + 192 - packet_start_sample / 2;
    //printf("start_sample=%d\n", start_sample );
    start_sample = fine_timing_sync_11a( fxp_time_signal + ( AGCFreeze_sample - packet_start_sample ) / 4 ); 
    start_sample += ( AGCFreeze_sample - packet_start_sample ) / 4;
    //printf("start_sample=%d %d\n", start_sample, ( AGCFreeze_sample - packet_start_sample ) / 4 + 1 );
    //printf("VGA_gain=%d %d\n", VGA_gain, AGCFreeze_sample );
  }
  else
  {
      //printf("else idealTiming\n");
    start_sample = (ExtraNoiseSamples >> sim_params.sampling_power) + 192 - packet_start_sample / 4;
  }
  //]

  //[ Timing error
//  if ( start_sample > (ExtraNoiseSamples >> sim_params.sampling_power) + 192 - packet_start_sample / 4 + 1  )
//  {
//    start_sample = 0;
//  }
  //]

  if ( !sim_params.idealFrequencySync )
  {
      std::cout << "\n else idealFrequencySync " << std::endl;

    //[ Fine frequency offset estimation
	  radians_per_sample_fine = // here radians_per_sample precision is fxp_params.Arctan_precision
      fine_frequency_sync_11a( fxp_time_signal + start_sample );
	  //]

    //[ Correct frequency offset for LTS and PLCP header
    rotor_state_angle = 0;
      std::cout << "\n radians_per_sample_fine = " << radians_per_sample_fine << std::endl;
	  rotor_11a( fxp_time_signal + start_sample,
						   radians_per_sample_fine,
						   3 * NumOfSubcarriers + CyclicPrefixLength );
    //]
  }

  //[ Extract long training symbols
  extract_LTS_11a( freq_tr_syms, fxp_time_signal + start_sample );

  // Here freq_tr_syms have (fxp_params.FFT_precision - fxp_params.FFT_shift_control) precision
  //]

  //[ Channel estimation
  channel_estimation_11a( channel_estimate,
                          freq_tr_syms );
  // Here channel_estimate have fxp_params.Demodulator precision
  //]

  start_sample += 2 * NumOfSubcarriers + CyclicPrefixLength;

  //[ Process PLCP header 
  memcpy( freq_OFDM_syms,
          fxp_time_signal + start_sample, 
          NumOfSubcarriers * sizeof( fxp_complex ) );

    fft_with_mapping_11a( freq_OFDM_syms, 6, 0,
                        fxp_params.FFT_precision,
                        fxp_params.FFT_exp_position,
                        fxp_params.FFT_shift_control );

  // Here freq_OFDM_syms (fxp_params.FFT_precision - fxp_params.FFT_shift_control) precision, 
  // but the its will be changed in equalizer_11a to the fxp_params.Demodulator_precision

    equalizer_11a( freq_OFDM_syms, channel_estimate, 1 );

  // Here both freq_OFDM_syms and channel_estimate have fxp_params.Demodulator_precision 
  phase_tracker_11a( freq_OFDM_syms,
                     phase_sym_err,
                     phase_subc_err,
                     &phase_sym_err,
                     &phase_subc_err,
                     &rotor_correction,
                     0,
                     1 );

    demodulator_11a( soft_bits,
                   freq_OFDM_syms,
                   channel_estimate,
                   1, 1,
                   channel_sq_amplitudies );

  //[ Convertion soft_bits precision from fxp_params.Demodulator_precision to the fxp_params.Viterbi_precision
  for ( i = 0; i < 48; i++ )
  {
    #ifdef LOGGING
    sprintf( overflow_comment, 
		"Overflow: soft_bits[%d] = fxp_scaling\n", i );
    #endif

    soft_bits[i] = get_bits_from_fxp_number( soft_bits[i],
                                             fxp_params.first_bit_for_decoder, 
                                             fxp_params.last_bit_for_decoder );
  }
  //]

  //[ Decode
  deinterleaver_11a( soft_bits, deinterl_bits, 48, 48 );

  if ( sim_params.useGolay )
  {
      std::cout << "\nGolayDecode " << std::endl;
      GolayDecode( golay_handle, deinterl_bits, PLCP_header );
      GolayDecode( golay_handle, deinterl_bits + 24, PLCP_header + 12 );
  }
  else
  {
      std::cout << "\nviterbi_decoder_11a " << std::endl;

      viterbi_decoder_11a(deinterl_bits,
          fxp_params.Viterbi_precision,
          48,
          PLCP_header);
      std::cout << "\nPLCP_error = " << PLCP_error << std::endl;
  }
  //]

  PLCP_error = process_PLCP_header( &bits_per_QAM_symbol,
                                    &num_of_bits, 
                                    &code_rate,
                                    &useABL,
                                    &useLDPC,
                                    *bits_per_OFDM_symbol,
                                    &num_of_pad_bits,
                                    PLCP_header );

    std::cout << "\nPLCP_error = " << PLCP_error << std::endl;

  if ( !useABL )
  {
    *bits_per_OFDM_symbol = bits_per_QAM_symbol * NumOfDataSubcarriers;
  }

  if ( useLDPC )
  {
    if ( num_of_bits % LDPCMessageLength == 0 )
    {
      num_of_LDPC_codewords = num_of_bits / LDPCMessageLength;
    }
    else
    {
      num_of_LDPC_codewords = num_of_bits / LDPCMessageLength + 1;
    }

    num_of_OFDM_symbols = (unsigned)ceil( 
                            (double)(num_of_LDPC_codewords * LDPCCodewordLength) / 
                            *bits_per_OFDM_symbol );
  }
  else
  {
    num_of_OFDM_symbols = (unsigned)ceil( (double)num_of_bits / 
                                          *bits_per_OFDM_symbol );
  }
  //]

  if ( num_of_OFDM_symbols * *bits_per_OFDM_symbol > MaxCodedPacketSize )
  {
      PLCP_error = -1;
      std::cout << "\nPLCP_error = " << PLCP_error << std::endl;
      std::cout << "\nnum_of_OFDM_symbols * *bits_per_OFDM_symbol = " << num_of_OFDM_symbols * *bits_per_OFDM_symbol << std::endl;
  }

  start_sample += NumOfSubcarriers;
  //[ Additional training
  if ( sim_params.advTraining )
  {
    radians_per_sample_fine = fxp_add_angles( 
                                radians_per_sample_fine,
                                rotor_correction,
                                fxp_params.Arctan_precision );

    rotor_11a( fxp_time_signal + start_sample,
               radians_per_sample_fine,
               2 * ( NumOfSubcarriers + CyclicPrefixLength ) );

    remove_cyclic_prefix_11a( freq_OFDM_syms,
                              fxp_time_signal + start_sample,
                              2 );

    for ( i = 0; i < 2; i++ )
    {
      fft_with_mapping_11a( freq_OFDM_syms + i * NumOfSubcarriers, 6, 0,  
                            fxp_params.FFT_precision,
                            fxp_params.FFT_exp_position,
                            fxp_params.FFT_shift_control );
    }

    for ( j = 0; j < 2; j++ )
      for ( i = 0; i < NumOfUsedSubcarriers; i++ )
      {
        additional_freq_tr_syms[ j * NumOfUsedSubcarriers + i ].re = 
          freq_OFDM_syms[ j * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1 ].re;

        additional_freq_tr_syms[ j * NumOfUsedSubcarriers + i ].im =
          freq_OFDM_syms[ j * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1 ].im;
      }

    channel_estimation_11a( additional_channel_estimate,
                            additional_freq_tr_syms );

    for ( i = 0; i < NumOfUsedSubcarriers; i++ )
    {
      channel_estimate[i] =
        fxp_complex_add( channel_estimate[i],
                          additional_channel_estimate[i],
                          fxp_params.Demodulator_precision,
                          &overflow_flag,
                          overflow_comment );

      channel_estimate[i].re >>= 1;
      channel_estimate[i].im >>= 1;
    }

    start_sample += 2 * ( NumOfSubcarriers + CyclicPrefixLength ); 
  }
  //]

  //[ Process signal if no error in PLCP header

  if ( !PLCP_error )
  {
    //[ Process each OFDM symbol
    for ( i = 0; i < num_of_OFDM_symbols; i++ )
    {
      //[ Correct frequency offset
      /*    rotor_state_angle = fxp_add_angles(
                            rotor_state_angle,
                            phase_sym_err,
                            fxp_params.Arctan_precision );*/

      radians_per_sample_fine = fxp_add_angles( 
                                  radians_per_sample_fine,
                                  rotor_correction,
                                  fxp_params.Arctan_precision );

      rotor_11a( fxp_time_signal + start_sample + i * ( NumOfSubcarriers + CyclicPrefixLength ),
                 radians_per_sample_fine,
                 NumOfSubcarriers + CyclicPrefixLength );
      //]

      //[ Convert time domain signal to frequency domain
      remove_cyclic_prefix_11a( freq_OFDM_syms + i * NumOfSubcarriers,
                                fxp_time_signal + start_sample + i * ( NumOfSubcarriers + CyclicPrefixLength ),
                                1 );

      fft_with_mapping_11a( freq_OFDM_syms + i * NumOfSubcarriers, 6, 0,  
                            fxp_params.FFT_precision,
                            fxp_params.FFT_exp_position,
                            fxp_params.FFT_shift_control );
      //]

      // Here freq_OFDM_syms (fxp_params.FFT_precision - fxp_params.FFT_shift_control) precision, 
      // but the its will be changed in equalizer_11a to the fxp_params.Demodulator_precision

      //[ Equalize 
      equalizer_11a( freq_OFDM_syms + i * NumOfSubcarriers,
                     channel_estimate, 
                     1 );
      //]

      // Here both freq_OFDM_syms and channel_estimate have fxp_params.Demodulator_precision 

      phase_tracker_11a( freq_OFDM_syms + i * NumOfSubcarriers, 
                         phase_sym_err,
                         phase_subc_err,
                         &phase_sym_err,
                         &phase_subc_err,
                         &rotor_correction,
                         i+1,
                         0 );
    }
    //]

    //[ Demodulation
    if ( !useABL )
    {
      demodulator_11a( soft_bits,
                       freq_OFDM_syms,
                       channel_estimate,
                       num_of_OFDM_symbols,
                       bits_per_QAM_symbol,
                       channel_sq_amplitudies );
    }
    else
    {
      demodulator_for_ABL( soft_bits,
                           freq_OFDM_syms,
                           channel_estimate,
                           num_of_OFDM_symbols,
                           ABL_bits_per_QAM_symbol,
                           channel_sq_amplitudies );
    }

    if ( useLDPC )
    {
      //[ Convertion soft_bits precision from fxp_params.Demodulator_precision to the fxp_params.LDPC_precision
      for ( i = 0; i < num_of_LDPC_codewords * LDPCCodewordLength; i++ )
      {
        #ifdef LOGGING
        sprintf( overflow_comment, 
		    "Overflow: soft_bits[%d] = fxp_scaling\n", i );
        #endif

        soft_bits[i] = get_bits_from_fxp_number( soft_bits[i],
                                                 fxp_params.first_bit_for_decoder, 
                                                 fxp_params.last_bit_for_decoder );
      }
      //]
    }
    else
    {
      //[ Convertion soft_bits precision from fxp_params.Demodulator_precision to the fxp_params.Viterbi_precision
      for ( i = 0; i < num_of_bits; i++ )
      {
        #ifdef LOGGING
        sprintf( overflow_comment, 
		    "Overflow: soft_bits[%d] = fxp_scaling\n", i );
        #endif

        soft_bits[i] = get_bits_from_fxp_number( soft_bits[i],
                                                 fxp_params.first_bit_for_decoder, 
                                                 fxp_params.last_bit_for_decoder );
      }
      //]
    }
    //]

    //[ Decode
    if ( !useLDPC )
    {
      deinterleaver_11a( soft_bits, 
                         deinterl_bits, 
                         num_of_OFDM_symbols * (*bits_per_OFDM_symbol),
                         *bits_per_OFDM_symbol );

      if ( !sim_params.useGolay )
      {
        num_of_bits = depuncturer_11a( deinterl_bits,
                                      num_of_bits,
                                      depunctured_bits,
                                      code_rate );

        num_of_bits -= 2 * num_of_pad_bits;

        num_of_bits = viterbi_decoder_11a( depunctured_bits, 
                                          fxp_params.Viterbi_precision,
                                          num_of_bits, 
                                          decoded_bits );
      }
      else
      {
        num_of_bits = 16 + 8 * sim_params.packet_size;
        for ( i = 0; i < 2 * num_of_OFDM_symbols; i++ )
          GolayDecode( golay_handle, deinterl_bits + 24 * i, decoded_bits + 12 * i );
      }
    }
    else
    {
      for ( i = 0; i < num_of_LDPC_codewords; i++ )
      {
        ldpc_decode_fxp( soft_bits + i * LDPCCodewordLength,
                         (char*)decoded_bits + i * LDPCMessageLength,
                         (char*)codeword,
                         reliabilities,
                         fxp_params.LDPC_precision );
      }

    }
    //]

    //[ Descramble data
    SERVICE = 0;
    for ( i = 0; i < 7; i++ )
    {
      SERVICE |= ((unsigned)decoded_bits[i]) << i;
    }

    scrambler_11a( decoded_bits, num_of_bits, &(descrambler_table[SERVICE]), 1 );

    memcpy( out_bits, decoded_bits + 16, num_of_bits - 16 );
    //]

    //[ Perform ABL procedure
    if ( useABL )
    {
      if ( sim_params.advTraining )
      {
        Wraper_for_BPLA( freq_tr_syms,
                         additional_freq_tr_syms,
                         ABL_bits_per_QAM_symbol,
                         profile );

      }
      else
      {
        Wraper_for_BPLA( freq_tr_syms,
                         NULL,
                         ABL_bits_per_QAM_symbol,
                         profile );

      }
	  	//BPLA_fxp( freq_tr_syms,
	    //    		  ABL_bits_per_QAM_symbol, 12, 12 );

      //[ Calculate number of coded bits per OFDM symbol (after ABL is needed)
      *bits_per_OFDM_symbol = 0;
      for ( i = 0; i < NumOfDataSubcarriers; i++ )
      {
        *bits_per_OFDM_symbol += ABL_bits_per_QAM_symbol[i];
        ucharABL_bits_per_QAM_symbol[i] = ABL_bits_per_QAM_symbol[i];
      }
      //]

      encode( ucharABL_bits_per_QAM_symbol, NUM_SYMBOLS, compressedABL_bits_per_QAM_symbol, &Bytes, &Bits, 0 );  
      *feedBackNumOfOFDMSyms = 2 * ( Bits + 6 + 8 ) / 48;

      //[ If bad channel situation then throughput set to 6 Mbit/s (code rate 0.5) or 9 Mbit/s (code rate 0.75)
      if ( *bits_per_OFDM_symbol < 8 )
      {
        for ( i = 0; i < NumOfDataSubcarriers; i++ ) 
        {
          ABL_bits_per_QAM_symbol[i] = 1;
        }
        *bits_per_OFDM_symbol = NumOfDataSubcarriers;
      }
      //]
    }
    //]
  }

  FILE* decoded_bitsD = NULL;
  decoded_bitsD = fopen("decoded_bitsD.txt", "w+");
  FILE* decoded_bitsU = NULL;
  decoded_bitsU = fopen("decoded_bitsU.txt", "w+");
  FILE* out_bitsD = NULL;
  out_bitsD = fopen("out_bits.txt", "w+");
  for (int i = 0; i < MaxCodedPacketSize; i++)
  {
      fprintf(decoded_bitsU, "%u ", decoded_bits[i]);
      fprintf(decoded_bitsD, "%d ", decoded_bits[i]);
  }
  for (int i = 0; i < num_of_samples; i++)
      fprintf(out_bitsD, "%d ", out_bits[i]);

    fclose (decoded_bitsD);
    fclose (decoded_bitsU);
    fclose (out_bitsD);
  //]

  #ifdef LOGGING
  logging( "Receiver finished\n", -1 );
  #endif
} 