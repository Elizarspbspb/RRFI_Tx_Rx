#include "interleaver_patt.h"

#include <memory.h>
#include <assert.h>
#include <stdio.h>

/**
////////////////////////////////////////////////////////////////////////////////
// Name: deinterleaver_11a
//
// Purpose: Implements deinterleaver
//
// Author: Alexandr Kozlov
//
// Parameters:  in_bits - input, Bits to be deinterleaved
//
//              intl_bits - output, Interleaved bits
//
//              num_of_bits - input, Number of bits
//
//              interleaver_depth - input, Interleaver depth
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void deinterleaver_11a( int* in_bits,
				   						  int* intl_bits,
										    unsigned num_of_bits,
										    unsigned interleaver_depth )
{
	unsigned i,j;
	unsigned* interleaver_patt;

  unsigned needTailInterleaving=0;
  int tail_buf[6 * NumOfDataSubcarriers];
  int tail_offset;
  int tail_length;
  int standard_interleaving_num=0;

  #ifdef LOGGING
  logging( "deinterleaver_11a...\n", 1 );
  #endif

  //[ Select interleaver 
	if ( interleaver_depth == NumOfDataSubcarriers )
	{
		interleaver_patt = interleaver1;
	}
	else
	if ( interleaver_depth == 2 * NumOfDataSubcarriers )
	{
		interleaver_patt = interleaver2;
	}
	else
	if ( interleaver_depth == 4 * NumOfDataSubcarriers )
	{
		interleaver_patt = interleaver4;
	}
	else
	if ( interleaver_depth == 6 * NumOfDataSubcarriers )
	{
		interleaver_patt = interleaver6;
	}
	else
	{
    //[ Use the longest interleaver with advanced tail interleaving
		interleaver_patt = interleaver6;
    interleaver_depth = 6 * NumOfDataSubcarriers;
    needTailInterleaving = 1;
    
    tail_length = num_of_bits % interleaver_depth;
    tail_offset = num_of_bits - interleaver_depth;

    if(tail_length>0){
      standard_interleaving_num =-1;
    }
    //]
	}
	//]

  standard_interleaving_num += num_of_bits/interleaver_depth;


  //[ Deinterleave tail bits if any
  if(needTailInterleaving && tail_length>0){
    assert(num_of_bits >= interleaver_depth);

    for ( j = 0; j < interleaver_depth; j++ )
		{
			intl_bits[ num_of_bits - interleaver_depth +j ] =
				in_bits[ tail_offset + interleaver_patt[j] ];
		}

    memcpy( tail_buf,
            in_bits + num_of_bits - tail_length - interleaver_depth,
            tail_length * sizeof(int) );
          
    memcpy(tail_buf + tail_length,
           intl_bits +num_of_bits - interleaver_depth, (interleaver_depth-tail_length) * sizeof(int));

    for ( j = 0; j < interleaver_depth; j++ )
		{
			intl_bits[ num_of_bits - tail_length - interleaver_depth +j ] =
				tail_buf[ interleaver_patt[j] ];
		}

  } //]

	//[ Deinterleave bits
	for ( i = 0; i < standard_interleaving_num; i++ )
	{
		for ( j = 0; j < interleaver_depth; j++ )
		{
			intl_bits[ i * interleaver_depth + j ] =
				in_bits[ i * interleaver_depth + interleaver_patt[j] ];
		}
	}
	//]

  #ifdef LOGGING
  logging( "deinterleaver_11a finished\n", -1 );
  #endif
}