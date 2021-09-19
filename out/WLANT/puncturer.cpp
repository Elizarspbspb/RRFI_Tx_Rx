#include "puncturer.h"
#include "commonfunc.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: puncturer_11a
//
// Purpose: Implements puncturer
//
// Author: Alexandr Kozlov
//
// Parameters:  in_bits - input, Bits to be punctured
//
//              num_of_bits - input, Number of bits
//
//              out_bits - output, Punctured bits
//
//              code_rate - input, Code rate ( 0.5, 2.0/3.0, 0.75 )
//  
// Returning value: Number of punctured bits
//////////////////////////////////////////////////////////////////////////////// */
unsigned puncturer_11a( unsigned char* in_bits, 
                        unsigned num_of_bits, 
                        unsigned char* out_bits,
                        double code_rate )
{
  unsigned i, j;
  
  unsigned punc_pattern[6];
  unsigned pattern_size;
  unsigned punc_length;

  unsigned num_of_rem_bits;

  unsigned char* cur_data = in_bits;
  unsigned char* cur_dest = out_bits;

  #ifdef LOGGING
  logging( "puncturer_11a...\n", 1 );
  #endif

  get_punc_params( punc_pattern, &pattern_size, &punc_length, code_rate );

  //[ Remainder bits are the bits in the end of the packet that are not integer multiple of the puncture window size
  num_of_rem_bits = num_of_bits % pattern_size;
  //]

  //[ Puncture
  for ( i = 0; i < ( num_of_bits - num_of_rem_bits ) / pattern_size ; i++ )
  {
    for ( j = 0; j < punc_length; j++ )
    {
      cur_dest[j] = cur_data[punc_pattern[j]];
      
    }
    cur_data += pattern_size;
    cur_dest += punc_length;
  }
  //]

  //[ Puncture remainder bits
  for ( i = 0; i < num_of_rem_bits; i++ )  
  {
    if ( punc_pattern[i] < num_of_rem_bits )
    {
      *cur_dest = cur_data[punc_pattern[i]];
      cur_dest++;
    }
  }
  //]

  #ifdef LOGGING
  logging( "puncturer_11a finished\n", -1 );
  #endif

  return cur_dest - out_bits;
}

