#include "interleaver_patt.h"
#include "model.h"  
#include <memory.h>
#include <assert.h>

#include <stdio.h>

/**
////////////////////////////////////////////////////////////////////////////////
// Name: interleaver_11a
//
// Purpose: Implements interleaver
//
// Author: Alexandr Kozlov
//
// Parameters:  in_bits - input, Bits to be interleaved
//
//              intl_bits - output, Interleaved bits
//
//              num_of_bits - input, Number of bits
//
//              interleaver_depth - input, Interleaver depth
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void interleaver_11a( unsigned char* in_bits,
							   		  unsigned char* intl_bits,
								      unsigned num_of_bits,
									    unsigned interleaver_depth )
{
	unsigned i,j;
	unsigned* interleaver_patt;

  unsigned needTailInterleaving=0;
  unsigned char tail_buf[6 * NumOfDataSubcarriers];
  int tail_offset;
  int tail_length;

#ifdef LOGGING
  logging( "interleaver_11a...\n", 1 );
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
    //]
	}
	//]

	//[ Interleave bits
	for ( i = 0; i < num_of_bits / interleaver_depth; i++ )
	{
		for ( j = 0; j < interleaver_depth; j++ )
		{
			intl_bits[ i * interleaver_depth + interleaver_patt[j] ] =
				in_bits[ i * interleaver_depth + j ];
		}
	}
	//]
  
  //[ Interleave tail bits if any
  if(needTailInterleaving && tail_length>0){
    assert(num_of_bits >= interleaver_depth);

    memcpy(tail_buf, intl_bits + tail_offset, interleaver_depth - tail_length);
    memcpy(tail_buf + interleaver_depth - tail_length,
           in_bits + num_of_bits - tail_length, tail_length);

    for ( j = 0; j < interleaver_depth; j++ )
		{
			intl_bits[ tail_offset + interleaver_patt[j] ] =
				tail_buf[ j ];
		}
  } //]

#ifdef LOGGING
  logging( "interleaver_11a finished\n", -1 );
#endif
}