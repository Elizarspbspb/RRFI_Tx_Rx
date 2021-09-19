#include <iostream>
#include "conv_decoder.h"
#include "conv_code.h"
#include "commonfunc.h"
#include "sim_consts.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: viterbi_decoder_11a
//
// Purpose: Implements fixed-point Viterbi decoder
//
// Author: Alexandr Kozlov
//
// Parameters:  rx_bits - input, Soft input
//
//              precision - input, Fixed-point precision 
//
//              num_of_bits - input, Number of soft bits
//
//              out_bits - output, Decoded bits
//  
// Returning value: Number of decoded bits
//////////////////////////////////////////////////////////////////////////////// */
unsigned viterbi_decoder_11a( int* rx_bits,
                              unsigned precision,
                              unsigned num_of_bits,
                              unsigned char* out_bits )
{
    std::cout << " viterbi_decoder_11a " << std::endl;
  //unsigned char overflow_flag; // Было
  char overflow_flag;
  unsigned i;
  unsigned state;
  int data_bit;
  int cum_metrics[64];
  int tmp_cum_metrics[64];
  int path_metric1;
  int path_metric2;

  //unsigned char max_paths[64][MaxCodedPacketSize + 6]; //MaxCodedPacketSize = 66444
  std::cout << "MaxCodedPacketSize = " << MaxCodedPacketSize << std::endl;

  auto **max_paths = new unsigned char* [64];
  for (int count = 0; count < 64; count++)
      max_paths[count] = new unsigned char [MaxCodedPacketSize + 6];


  int tmp_max_cum_metric;
  int cum_metrics1;
  int cum_metrics2;

  #ifdef LOGGING
  logging( "viterbi_decoder_11a...\n", 1 );
  #endif

  //[ Initialize paths metrics
  cum_metrics[0] = 0;
  for( i = 1; i < 64; i++ )
  {
      cum_metrics[i] = 1;
  }
  //]

  for ( data_bit = 0; data_bit < num_of_bits; data_bit += 2 )
	{
    tmp_max_cum_metric = 0;

    //[ Compute metrics for all states
		for ( state = 0; state < 64; state++ )
		{
      #ifdef LOGGING
      sprintf( overflow_comment, 
		  "Overflow: path_metric1 = fxp_add= fxp_add, data_bit = %d, state = %d\n", data_bit, state );
      #endif

	  	path_metric1 = 
        fxp_add( conv_code.prev_state_outbits[0][state] * rx_bits[data_bit],
                 conv_code.prev_state_outbits[2][state] * rx_bits[data_bit+1],
                 precision,
                 &overflow_flag,
                 overflow_comment );

      #ifdef LOGGING
      sprintf( overflow_comment, 
		  "Overflow: path_metric2 = fxp_add= fxp_add, data_bit = %d, state = %d\n", data_bit, state );
      #endif

			path_metric2 = 
        fxp_add( conv_code.prev_state_outbits[1][state] * rx_bits[data_bit],
                 conv_code.prev_state_outbits[3][state] * rx_bits[data_bit+1],
                 precision,
                 &overflow_flag,
                 overflow_comment );

      #ifdef LOGGING
      sprintf( overflow_comment, 
		  "Overflow: cum_metrics1 = fxp_add, data_bit = %d, state = %d\n", data_bit, state );
      #endif

      cum_metrics1 = fxp_add( cum_metrics[conv_code.prev_state[0][state]],
                              path_metric1,
                              precision,
                              &overflow_flag,
                              overflow_comment );

      #ifdef LOGGING
      sprintf( overflow_comment, 
		  "Overflow: cum_metrics2 = fxp_add, data_bit = %d, state = %d\n", data_bit, state );
      #endif

      cum_metrics2 = fxp_add( cum_metrics[conv_code.prev_state[1][state]],
                              path_metric2,
                              precision,
                              &overflow_flag,
                              overflow_comment );

			if ( cum_metrics1 > cum_metrics2 )
			{
				tmp_cum_metrics[state] = cum_metrics1;
        max_paths[state][data_bit/2] = 0;            
			}
			else
			{
		    tmp_cum_metrics[state] = cum_metrics2;
			  max_paths[state][data_bit/2] = 1;            
			}

      if ( tmp_max_cum_metric < tmp_cum_metrics[state] )
      {
        tmp_max_cum_metric = tmp_cum_metrics[state];
      }
		}
    //]

    //[ Decrease metric by maximum value to avoid overflows
    for ( i = 0; i < 64; i++ )
    {
      #ifdef LOGGING
      sprintf( overflow_comment, 
		  "Overflow: cum_metrics[%d] = fxp_add, data_bit = %d\n", i, data_bit );
      #endif

      cum_metrics[i] = fxp_sub( tmp_cum_metrics[i],
                                tmp_max_cum_metric,
                                precision,
                                &overflow_flag,
                                overflow_comment );
    }
    //]
	}  
  
  //[ Trace back
  state = 0;

  for ( data_bit = num_of_bits / 2 - 1; data_bit >= 0; data_bit-- )
	{	 
    if ( data_bit < num_of_bits / 2 - ConvCodeGenPolySize + 1 )
    {
      out_bits[data_bit] = (unsigned char)(state % 2);
    }
    state = conv_code.prev_state[max_paths[state][data_bit]][state];
  }
  //]

  #ifdef LOGGING
  logging( "viterbi_decoder_11a finished\n", -1 );
  #endif

  for (int count = 0; count < 64; count++)
      delete[] max_paths[count];
  delete[] max_paths;

  return ( num_of_bits / 2 - ConvCodeGenPolySize + 1 );
}