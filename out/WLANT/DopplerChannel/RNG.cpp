
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// RNG.c
//
// This file contains random number generation programs.  The internal state is contained in a
// dynamically allocated structure.  This allows a multi-thread simulation to initialize different
// states for different threads.

// The algorithm used here was originally published in "Toward a Universal Random Number Generator"
// by George Marsaglia and Arif Zaman.  Florida State University Report: FSU-SCRI-87-50 (1987).
// It passes ALL of the tests for random number generators and has a period of 2^144. The algorithm
// is a combination of a Fibonacci sequence (with lags of 97 and 33, and operation "subtraction
// plus one, modulo one") and an "arithmetic sequence" (using subtraction).
//
// This code was from an original FORTRAN program by Marsaglia and Zaman.  K-L.Noell and H. Weber
// converted the FORTRAN to C, and distributed it on the web, where I (John Sadowsky) got it.  The
// original Noell & Weber code performed most operation in floating point, but they were actually
// relying on integer mantissa arithmatic.  They claimed bit exact portability for any manchine
// having a 24 bit double precision mantissa.  I converted all core rng back to integer arithmatic.
//
// The basic rng function is i_rng() which returns an int.  It produces a 24 bit uniform random
// number in the range 0, .., 2^24 -1 = 16777215 (include endpoints).
//
// u_rng() returns a double uniform in the [ 2^-25, 1 - 2^-25 ], on a uniform grid of points separated
// by 2^-24. In particular, u_rng() does not return 0.0 or 1.0 -- which is important to know when using
// certain transformation to generate non-uniform random variables.
//
// The Gaussian RNGs g_rng() and gv_rng() use ALGORITHM 712, COLLECTED ALGORITHMS FROM ACM. THIS WORK
// IS PUBLISHED IN TRANSACTIONS ON MATHEMATICAL SOFTWARE, VOL. 18, NO. 4, DECEMBER, 1992, PP. 434-435.
// The algorithm uses the ratio of uniforms method of A.J. Kinderman and J.F. Monahan augmented
// with quadratic bounding curves.
//
// The following test program will verify that everything works on your machine.  It verifies the
// test values provided by Noell and H. Weber.
//
//  #include <stdlib.h>
//  #include "Rng.h"
//  void main( void ) {
//
//    int k, xx ;
//    int x_test[6] = { 6533892 , 14220222 , 7275067 ,
//                       6172232 , 8354498 , 10633180 } ;
//    RNG_StateStruct RNG_State ;
//
//    init_rng( -1 , -1 , &RNG_State ) ;
//
//    for ( k = 0 ; k < 20000 ; k++ ) xx = i_rng( RNG_State ) ;
//    for ( k = 0 ; k < 6 ; k++ ){
//      printf("  %10d  %10df\n", i_rng( RNG_State ) , x_test[k] ) ;
//    }
//
//  }
//

#include <math.h>
#include <stdlib.h>
#include "RNG.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// init_rng()
//
//  This function is the initialization routine.  There are two seeds, which must satisfy
//
//      0 <= ij <= 31328 and 0 <= kl <= 30081.
//
//  Violation of these input ranges results in substitution of default test values.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitRNG(
  int              seed ,
  RNG_StateStruct *RNG_State
) {

  int s,t ;
  int ij, kl ;
  int *u ;
  int ii,i,j,k,l,jj,m ;

  // initialize seed
  if ( seed < 0 ) {
    ij = 1802 ;                         // these are the standard seeds
    kl = 9373 ;
  } else {
    ij = (int) (seed & 0x00007fff) ;
    while ( ij > 31328 ) ij >>= 1 ;     // Sadowsky fix - ensures seeds are in proper range
    kl = (int) (seed >> 17) ;
    while ( kl > 30081 ) kl >>= 1 ;
  }

  // initialize state

  u = RNG_State->u ;

  i = (ij / 177) % 177 + 2 ;
  j = (ij % 177)       + 2 ;
  k = (kl / 169) % 178 + 1 ;
  l = (kl % 169);

  for (ii=0; ii<97; ii++) {
    s = 0;
    t = 8388608 ;
    for (jj=0; jj<24; jj++) {
      m = (((i * j) % 179) * k) % 179 ;
      i = j ;
      j = k ;
      k = m ;
      l = (53 * l + 1) % 169 ;
      if (((l * m % 64)) >= 32) s += t ;
      t /= 2 ;
    }
    u[ii] = s ;
  }

  RNG_State->c   = 362436 ;
  RNG_State->i97 = 97;
  RNG_State->j97 = 33;

  RNG_State->numBitsInBucket = 0 ;

  return ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Integer24BitRNG()
//
//  Generates a single uniform random integer the range 0,.., 2^24 -1 = 16777215.
//////////////////////////////////////////////////////////////////////////////////////////////////

int Integer24BitRNG( RNG_StateStruct *RNG_State ) {

  int  uni, c, i97, j97 ;
  int  *u ;

  u   = RNG_State->u ;
  i97 = RNG_State->i97 ;
  j97 = RNG_State->j97 ;
  c   = RNG_State->c ;

  uni = u[--i97] - u[--j97] ;
  if (uni < 0L) uni += 16777216L ;
  u[i97] = uni ;
  c -= 7654321L ;
  if ( c < 0L ) c += 16777213L ;
  uni -= c ;
  if (uni < 0L) uni += 16777216L ;

  RNG_State->i97 = (i97 == 0) ? 97 : i97 ;
  RNG_State->j97 = (j97 == 0) ? 97 : j97 ;
  RNG_State->c   = c ;

  return uni ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Integer32BitRNG()
//////////////////////////////////////////////////////////////////////////////////////////////////

int Integer32BitRNG( RNG_StateStruct *RNG_State ) {

   return ((Integer24BitRNG( RNG_State ) << 8) ^ Integer24BitRNG( RNG_State ))  ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  OneBitRNG()
///////////////////////////////////////////////////////////////////////////////////////////////////

char OneBitRNG( RNG_StateStruct *RNG_State ) {

  char  x ;

  if ( RNG_State->numBitsInBucket <= 0 ) {
    RNG_State->numBitsInBucket = 24 ;
    RNG_State->randomBitBucket = Integer24BitRNG( RNG_State ) ;
  }

  RNG_State->numBitsInBucket-- ;
  x = (char) (RNG_State->randomBitBucket & 1) ;
 (RNG_State->randomBitBucket)-- ;

  return x ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  BitVectorRNG()
///////////////////////////////////////////////////////////////////////////////////////////////////

void BitVectorRNG(
  int              numBit ,
  char            *randomBits ,
  RNG_StateStruct *RNG_State
) {

  int   k, kx, kb, bitBucket ;

  kb = 0 ;
  for ( kx = 0 ; kx < numBit/24 ; kx++) {
    bitBucket = Integer24BitRNG( RNG_State ) ;
    for ( k = 0 ; k < 23 ; k++ ) {
      randomBits[kb++] = (char) (bitBucket & 1) ;
      bitBucket >>= 1 ;
    }
  }

  bitBucket = Integer24BitRNG( RNG_State ) ;
  while ( kb < numBit ) {
    randomBits[kb++] = (char) (bitBucket & 1) ;
    bitBucket >>= 1 ;
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//  UniformRNG()
//
//  Generates a single uniform random variable.  The range is [ 2^-25 , 1 - 2^-25 ] on a uniform
//  grid with spacing 2^-24 .
//
///////////////////////////////////////////////////////////////////////////////////////////////////

float UniformRNG( RNG_StateStruct *RNG_State ) {

  return ( ((float) Integer24BitRNG( RNG_State )) + 0.5f ) / 16777216.0f ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  GaussianRNG() and GaussVectorRNG()
//
//  Returns a single standard Gaussian random variable. GaussVectorRNG() fills a vector with independent
//  standard Gaussians.  The single variate function uses Integer24BitRNG(), but for maximal efficiency,
//  the vector function reproduces the Integer24BitRNG() code internally.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

float GaussianRNG( RNG_StateStruct *RNG_State ) {

 float  q,u,v,x,y;

  do {
    u = ( ((float) Integer24BitRNG( RNG_State )) + 0.5f ) / 16777216.0f ;
    v = ( ((float) Integer24BitRNG( RNG_State )) + 0.5f ) / 16777216.0f ;
    v = 1.7156f * (v - 0.5f) ;
    //  Evaluate the quadratic form
    x = u - 0.449871f;
    y = ((v > 0) ? v : -v) + 0.386595f;
    q = x * x + y * (0.19600f * y - 0.25472f * x) ;
    // Accept P if inside inner ellipse
    if (q < 0.27597) break;
    //  Reject P if outside outer ellipse, or outside acceptance region
  } while ((q > 0.27846) || (v * v > -4.0 * log(u) * u * u)) ;

  //  Return ratio of P's coordinates as the normal deviate
  return v / u ;

}

void GaussVectorRNG(
  int              n ,
  float           *gv ,
  RNG_StateStruct *RNG_State
) {

  float  q,u,v,x,y;
  int  uni, c, k, i97, j97 ;
  int *uu ;

  uu  = RNG_State->u ;
  i97 = RNG_State->i97 ;
  j97 = RNG_State->j97 ;
  c   = RNG_State->c ;

  for ( k = 0 ; k < n ; k++ ) {

    do {

      uni = uu[--i97] - uu[--j97] ;
      if (uni < 0L) uni += 16777216L ;
      uu[i97] = uni ;
      i97 = (i97 == 0) ? 97 : i97 ;
      j97 = (j97 == 0) ? 97 : j97 ;
      c -= 7654321L ;
      if ( c  < 0L )   c += 16777213L ;
      uni -= c ;
      if (uni < 0L ) uni += 16777216L ;
      u = ( ((float) uni) + 0.5f ) / 16777216.0f ;

      uni = uu[--i97] - uu[--j97] ;
      if (uni < 0L) uni += 16777216L ;
      uu[i97] = uni ;
      i97 = (i97 == 0) ? 97 : i97 ;
      j97 = (j97 == 0) ? 97 : j97 ;
      c -= 7654321L ;
      if ( c  < 0L )   c += 16777213L ;
      uni -= c ;
      if (uni < 0L ) uni += 16777216L ;
      v = ( ((float) uni) + 0.5f ) / 16777216.0f ;
      v = 1.7156f * (v - 0.5f) ;

      // Evaluate the quadratic form
      x = u - 0.449871f;
      y = ((v > 0) ? v : -v) + 0.386595f;
      q = x * x + y * (0.19600f * y - 0.25472f * x) ;
      // Accept P if inside inner ellipse
      if (q < 0.27597) break;
      // Reject P if outside outer ellipse, or outside acceptance region
    } while ((q > 0.27846f) || (v * v > -4.0f * ((float) log(u)) * u * u)) ;

    gv[k] = v / u ;

  }  // End of for loop

  RNG_State->c = c ;
  RNG_State->i97 = i97 ;
  RNG_State->j97 = j97 ;

  return ;

}


