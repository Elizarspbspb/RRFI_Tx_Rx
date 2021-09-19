#include "conv_encoder.h"
#include "conv_code.h"
#include <memory.h>
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: conv_encoder_11a
//
// Purpose: Implements convolutional encoder
//
// Author: Alexandr Kozlov
//
// Parameters:  inf_bits - input, Bits to be encoded
//
//              num_of_bits - input, Number of information bits
//
//              coded_bits - output, Encoded bits
//  
// Returning value: Number of coded bits
//////////////////////////////////////////////////////////////////////////////// */
unsigned conv_encoder_11a( unsigned char* inf_bits, 
                           unsigned num_of_bits,
                           unsigned char* coded_bits )
{
  unsigned i,j;

  #ifdef LOGGING
  logging( "conv_encoder_11a...\n", 1 );
  #endif

  memset( coded_bits, 0, 2 * num_of_bits );

  //[ Encode information bits using two generator polynoms
  for( i = 0; i < num_of_bits; i++ )
  {
    for( j = 0; j < ConvCodeGenPolySize; j++ )
    {
       coded_bits[2 * ( i + j )] ^= inf_bits[i] & conv_code.ConvCodeGenPoly1[j];
       coded_bits[2 * (i + j) + 1] ^= inf_bits[i] & conv_code.ConvCodeGenPoly2[j];
    }
  }
  //]

  #ifdef LOGGING
  logging( "conv_encoder_11a finished\n", -1 );
  #endif

  return 2 * num_of_bits;
}

