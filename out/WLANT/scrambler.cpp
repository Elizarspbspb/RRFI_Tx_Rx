#include "model.h"
#include "scrambler.h"

Scrambler_state tx_scrambler_state =
{
  0x05,
  0x0D
};

Scrambler_state descrambler_table[128] =
{
  { 0,	0 },
  { 6,	4 },
  { 3,	2 },
  { 5,	6 },
  { 5,	9 },
  { 3,	13 },
  { 6,	11 },
  { 0,	15 },
  { 4,	8 },
  { 2,	12 },
  { 7,	10 },
  { 1,	14 },
  { 1,	1 },
  { 7,	5 },
  { 2,	3 },
  { 4,	7 },
  { 2,	4 },
  { 4,	0 },
  { 1,	6 },
  { 7,	2 },
  { 7,	13 },
  { 1,	9 },
  { 4,	15 },
  { 2,	11 },
  { 6,	12 },
  { 0,	8 },
  { 5,	14 },
  { 3,	10 },
  { 3,	5 },
  { 5,	1 },
  { 0,	7 },
  { 6,	3 },
  { 1,	2 },
  { 7,	6 },
  { 2,	0 },
  { 4,	4 },
  { 4,	11 },
  { 2,	15 },
  { 7,	9 },
  { 1,	13 },
  { 5,	10 },
  { 3,	14 },
  { 6,	8 },
  { 0,	12 },
  { 0,	3 },
  { 6,	7 },
  { 3,	1 },
  { 5,	5 },
  { 3,	6 },
  { 5,	2 },
  { 0,	4 },
  { 6,	0 },
  { 6,	15 },
  { 0,	11 },
  { 5,	13 },
  { 3,	9 },
  { 7,	14 },
  { 1,	10 },
  { 4,	12 },
  { 2,	8 },
  { 2,	7 },
  { 4,	3 },
  { 1,	5 },
  { 7,	1 },
  { 4,	9 },
  { 2,	13 },
  { 7,	11 },
  { 1,	15 },
  { 1,	0 },
  { 7,	4 },
  { 2,	2 },
  { 4,	6 },
  { 0,	1 },
  { 6,	5 },
  { 3,	3 },
  { 5,	7 },
  { 5,	8 },
  { 3,	12 },
  { 6,	10 },
  { 0,	14 },
  { 6,	13 },
  { 0,	9 },
  { 5,	15 },
  { 3,	11 },
  { 3,	4 },
  { 5,	0 },
  { 0,	6 },
  { 6,	2 },
  { 2,	5 },
  { 4,	1 },
  { 1,	7 },
  { 7,	3 },
  { 7,	12 },
  { 1,	8 },
  { 4,	14 },
  { 2,	10 },
  { 5,	11 },
  { 3,	15 },
  { 6,	9 },
  { 0,	13 },
  { 0,	2 },
  { 6,	6 },
  { 3,	0 },
  { 5,	4 },
  { 1,	3 },
  { 7,	7 },
  { 2,	1 },
  { 4,	5 },
  { 4,	10 },
  { 2,	14 },
  { 7,	8 },
  { 1,	12 },
  { 7,	15 },
  { 1,	11 },
  { 4,	13 },
  { 2,	9 },
  { 2,	6 },
  { 4,	2 },
  { 1,	4 },
  { 7,	0 },
  { 3,	7 },
  { 5,	3 },
  { 0,	5 },
  { 6,	1 },
  { 6,	14 },
  { 0,	10 },
  { 5,	12 },
  { 3,	8 }
};

/**
////////////////////////////////////////////////////////////////////////////////
// Name: scrambler
//
// Purpose: Implements scrambler/descrambler
//
// Author: Alexandr Kozlov
//
// Parameters: inf_bits - input/output, Bits to be scrambled/descrambled
//
//             num_of_bits - input, Number of bits to be scrambled
//
//						 scrambler_state - input, Scrambler registers state
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void scrambler_11a( unsigned char* inf_bits,
                    unsigned num_of_bits, 
                    Scrambler_state* scrambler_state,
                    char descrambler_flag )
{
  unsigned i;
  unsigned char bit1;
  unsigned char bit2;

  Scrambler_state temp = *scrambler_state;

  #ifdef LOGGING
  logging( "scrambler_11a...\n", 1 );
  #endif
  
  for ( i = 0; i < num_of_bits; i++ )
  {
    scrambler_state->reg1 <<= 1;
    scrambler_state->reg2 <<= 1;

    bit1 = ((scrambler_state->reg1 & 0x8) >> 3);
    bit2 = ((scrambler_state->reg2 & 0x10) >> 4);

    scrambler_state->reg1 ^= bit2;
    scrambler_state->reg2 ^= (bit1 ^ bit2);
    inf_bits[i] ^= (bit1 ^ bit2);
  }

  if ( descrambler_flag )
  {
    *scrambler_state = temp;
  }

  #ifdef LOGGING
  logging( "scrambler_11a finished\n", -1 );
  #endif
}

