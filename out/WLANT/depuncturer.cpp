#include "string.h"
#include "depuncturer.h"
#include "commonfunc.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: depuncturer_11a
//
// Purpose: Implements depuncturer
//
// Author: Alexandr Kozlov
//
// Parameters:  in_bits - input, Bits to be depunctured
//
//              num_of_bits - input, Number of bits
//
//              out_bits - output, Depunctured bits
//
//              code_rate - input, Code rate ( 0.5, 2.0/3.0, 0.75 )
//  
// Returning value: Number of depunctured bits
//////////////////////////////////////////////////////////////////////////////// */
unsigned depuncturer_11a( int* in_bits, 
                          unsigned num_of_bits, 
                          int* out_bits,
                          double code_rate )
{
  unsigned i, j;
  
  unsigned punc_pattern[6];
  unsigned pattern_size;
  unsigned punc_length;

  unsigned num_of_rem_bits;

  int* cur_data = in_bits;
  int* cur_dest = out_bits;

  get_punc_params( punc_pattern, &pattern_size, &punc_length, code_rate );

  //[ Remainder bits are the bits in the end of the packet that are not integer multiple of the puncture window size
  num_of_rem_bits = num_of_bits % punc_length;
  //]

   //[ Depuncture
  memset( out_bits, 0, sizeof( int ) * ( ( num_of_bits - num_of_rem_bits ) / 
                                              punc_length ) * pattern_size );
  for ( i = 0; i < ( num_of_bits - num_of_rem_bits ) / punc_length ; i++ )
  {
    for ( j = 0; j < punc_length; j++ )
    {
      cur_dest[punc_pattern[j]] = cur_data[j];
    }
    cur_data += punc_length;
    cur_dest += pattern_size;
  }
  //]

  if ( num_of_rem_bits )
  {
    memset( cur_dest, 0, sizeof( int ) * 
                         ( punc_pattern[ num_of_rem_bits - 1 ] + 1 ) );

    //[ Depuncture remainder bits
    for ( i = 0; i < num_of_rem_bits; i++ )  
    {
      cur_dest[punc_pattern[i]] = cur_data[i];
    } 
    //]

    return cur_dest + punc_pattern[ num_of_rem_bits - 1 ] + 1 - out_bits;
  }
  else
  {
    return cur_dest - out_bits;
  }
}

