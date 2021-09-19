
#ifndef _RNG_H_
#define _RNG_H_

///////////////////////////////////////////////////////////////////////////////////////////////////
// RNG State Data Structure
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {

    int   u[97] ;
    int   c ;
    int   i97 ;
    int   j97 ;

    int   randomBitBucket ;
    int   numBitsInBucket ;

} RNG_StateStruct ;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitRNG(
  int              seed ,
  RNG_StateStruct *RNG_State
) ;

int Integer24BitRNG(
  RNG_StateStruct *RNG_State
) ;

int Integer32BitRNG(
  RNG_StateStruct *RNG_State
) ;

char OneBitRNG(
  RNG_StateStruct *RNG_State
) ;

float  UniformRNG(
  RNG_StateStruct *RNG_State
) ;

float GaussianRNG(
  RNG_StateStruct *RNG_State
) ;

void BitVectorRNG(
  int              numBit ,
  char            *randomBits ,
  RNG_StateStruct *RNG_State
) ;

void GaussVectorRNG(
  int              n ,
  float           *gv ,
  RNG_StateStruct *RNG_State
) ;

#endif
