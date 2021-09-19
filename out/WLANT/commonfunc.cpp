/* File History
 * $History: commonfunc.c $
 * 
 * *****************  Version 14  *****************
 * User: Akozlov      Date: 20.01.05   Time: 17:13
 * Updated in $/WLAN
 * dump_fxp_array function added
 * 
 * *****************  Version 13  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:46
 * Updated in $/WLAN
 * process_PLCP_header changed
 * 
 * *****************  Version 12  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:53
 * Updated in $/WLAN
 * 
 * *****************  Version 11  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:35
 * Updated in $/WLAN
 * 
 * *****************  Version 10  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:10
 * Updated in $/WLAN
 * Functions for fixed-point bits operations and checking are added
 * 
 * *****************  Version 9  *****************
 * User: Akozlov      Date: 1.12.04    Time: 18:06
 * Updated in $/WLAN
 * Estimating num_of_pad_bits
 *
 * 
 */

#include "commonfunc.h"
#include "model.h"
#include "sim_params.h"
#include "sim_consts.h"
#include <math.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_blas.h>
#include <stdio.h>

#define SET_FXP_MAX(result,precision) {result = 0; \
                                        result |= ( 1 << ( precision - 1 ) ) - 1;}

#define SET_FXP_MIN(result,precision) {result = 0; \
                                        result |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) \
                                        << ( precision - 1 ); }

/**
////////////////////////////////////////////////////////////////////////////////
// Name: get_bits_from_fxp_number
//
// Purpose: Get least significant bits
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, fixed-point argument 
//
//             first_bits - input, first bit index which will be get from number (0..15)
//
//             last_bits - input, last bit index which will be get from number (0..15)
//
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int get_bits_from_fxp_number( int value, unsigned first_bits, unsigned last_bits )
{
  return get_least_bits_from_fxp_number( value >> first_bits, last_bits + 1 - first_bits );
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: get_least_bits_from_fxp_number
//
// Purpose: Get least significant bits
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, fixed-point argument 
//
//             precision - input, precision
//
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int get_least_bits_from_fxp_number( int value, unsigned precision )
{
  int min;
  int max;
    
  SET_FXP_MIN( min, precision );
  SET_FXP_MAX( max, precision );

  if ( value > max )
  {
    value = max;
  }

  if ( value < min )
  {
    value = min;
  }

  return value;
}


/**
////////////////////////////////////////////////////////////////////////////////
// Name: get_num_of_used_digits
//
// Purpose: Check how many bits in number used
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, fixed-point argument 
//
// Returning value: Number of used bits 
//////////////////////////////////////////////////////////////////////////////// */
unsigned get_num_of_used_digits( int value )
{
  unsigned i = 0;

  for ( i = 1; i <= 16; i++ )
  {
    if ( ( value >= 0 ) && ( value <= (1<<i) - 1 ) )
    {
      break; 
    }

    if ( ( value < 0 ) && ( value >= -(1<<i) ) )
    {
      break; 
    }
  }

  return i + 1;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: make2bit_value
//
// Purpose: Make 2 bit precision value
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, fixed-point argument 
//
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int make2bit_value( int value )
{
	if ( value == 0 )
	{
		return 0;
	}
	else
	if ( value > 0 )
	{
		return 1;
	}
	else
	if ( value < 0 )
	{
		return -1;
	}
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_abs2_beta
//
// Purpose: Fixed-point complex |a.re|+|a.im|
//					
// Author: Konstantin Zakhachenko, 17.nov.2004
//
// Parameters: a - input, Complex fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int fxp_complex_abs2_beta( fxp_complex a,
                      unsigned precision,
                      char* overflow_flag,
                      char* overflow_comment )
{
	int result;

  char flag;

  *overflow_flag = 0;

  result = fxp_add( abs(a.re),
										abs(a.im),
										precision,
										&flag,
										overflow_comment );	
  if ( flag )
  { 
    *overflow_flag = 1;
  }

	return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: find_closest_fxp_value
//
// Purpose: Convert floating-point value to fixed-point using predefined step
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, Floating-point value
//
//             precision - input, Fixed-point precision 
//
//             step - input, Floating-point step
//
// Returning value: Fixed-point value
//////////////////////////////////////////////////////////////////////////////// */
int find_closest_fxp_value( double value, 
                            unsigned precision, 
                            double step )
{
  int i;
  int fxp_value;
  double min;

  min = HUGE_VAL;
      
  for ( i = -( 1 << precision );
        i < 1 << precision; i++ )
  {
    if ( min > fabs( i * step - value ) )
    {
      min = fabs( i * step - value );
      fxp_value = i;
    }
  }  

  return fxp_value;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: signal_complex_convolution
//
// Purpose: Computes linear convolution of the signal with some kind of 
//          impulse response
//          WARNING! Results are SUMMED with the contents of Convolution.
//					
// Author: Alexandr Kozlov
//
// Parameters: input_signal - input, Vector of original signal samples 
//
//             num_of_samples - input, Number of signal samples 
//
//             impulse_response - input, Impulse response to be convolved with 
//
//             ir_duration - input, Impulse response duration 
//
//             Convolution - output, Output buffer 
//           
// Returning value: Convolution length
//////////////////////////////////////////////////////////////////////////////// */
unsigned signal_complex_convolution (
  const gsl_complex* input_signal, // [num_of_samples] 
  unsigned           num_of_samples,    
  const gsl_complex* impulse_response, // [ir_duration] 
  unsigned           ir_duration,       
  gsl_complex*       Convolution // [num_of_samples + ir_duration - 1]       
) 
{
  unsigned result_length = num_of_samples + ir_duration - 1;
  gsl_complex* cur_signal;
  unsigned i;

	gsl_vector_complex  tempIR;
  gsl_vector_complex  convolution;

	tempIR.size = ir_duration;
	tempIR.data = (double*)impulse_response;
	tempIR.stride = 1;

	convolution.size = ir_duration;
	convolution.stride = 1;

  for( i = 0; i < num_of_samples; i++) {
		cur_signal = (gsl_complex*)(input_signal + i);
		convolution.data = (double*)( Convolution + i );
		gsl_blas_zaxpy( *cur_signal, &tempIR, &convolution );
  }

  return result_length;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: get_punc_params
//
// Purpose: Get puncturer parameters
//
// Author: Alexandr Kozlov
//
// Parameters:  punc_pattern - output, Punturer pattern
//
//              pattern_size - output, Punturer pattern size
//          
//              punc_length - output, Punturered block length
//
//              code_rate - input, Code rate ( 0.5, 2.0/3.0, 0.75 )
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void get_punc_params( unsigned* punc_pattern, 
                      unsigned* pattern_size,
                      unsigned* punc_length,
                      double code_rate )
{
	if ( code_rate == 0.75 )
	{
	   punc_pattern[0] = 0;
	   punc_pattern[1] = 1;
	   punc_pattern[2] = 2;
	   punc_pattern[3] = 5;

	   *pattern_size = 6;  
	   *punc_length = 4;
	}
	else if ( code_rate == 2.0/3.0 )
	{
	   punc_pattern[0] = 0;
	   punc_pattern[1] = 1;
	   punc_pattern[2] = 2;

	   *pattern_size = 4;
	   *punc_length = 3;
	}
	else if ( code_rate == 0.5 )
	{
	   punc_pattern[0] = 0;
	   punc_pattern[1] = 1;
	   punc_pattern[2] = 2;
	   punc_pattern[3] = 3;
	   punc_pattern[4] = 4;
	   punc_pattern[5] = 5;

	   *pattern_size = 6;
	   *punc_length = 6;
	}
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: get_PLCP_header
//
// Purpose: Prepare PLCP header for transmission
//
// Author: Alexandr Kozlov
//
// Parameters: PLCP_header - output, PLCP header
//
//             packet_size - input, Packet size in bytes
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void get_PLCP_header( unsigned char* PLCP_header, 
                      unsigned packet_size )
{
  unsigned i;

  //[ Fill rate field
  if ( !sim_params.useABL )
  {
    if ( sim_params.data_rate == 6 )
    {
      if ( sim_params.useLDPC )
      {
        printf( "Unsupported data_rate for LDPC\n");
        exit(1);
      }
      else
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 1;
        PLCP_header[2] = 0;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 9 )
    {
      if ( sim_params.useLDPC )
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 1;
        PLCP_header[2] = 1;
        PLCP_header[3] = 0;
      }
      else
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 1;
        PLCP_header[2] = 1;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 12 )
    {
      if ( sim_params.useLDPC )
      {
        printf( "Unsupported data_rate for LDPC\n");
        exit(1);
      }
      else
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 1;
        PLCP_header[2] = 0;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 18 )
    {
      if ( sim_params.useLDPC )
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 1;
        PLCP_header[2] = 1;
        PLCP_header[3] = 0;
      }
      else
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 1;
        PLCP_header[2] = 1;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 24 )
    {
      if ( sim_params.useLDPC )
      {
        printf( "Unsupported data_rate for LDPC\n");
        exit(1);
      }
      else
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 0;
        PLCP_header[2] = 0;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 36 )
    {
      if ( sim_params.useLDPC )
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 0;
        PLCP_header[2] = 1;
        PLCP_header[3] = 0;
      }
      else
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 0;
        PLCP_header[2] = 1;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 48 )
    {
      if ( sim_params.useLDPC )
      {
        printf( "Unsupported data_rate for LDPC\n");
        exit(1);
      }
      else
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 0;
        PLCP_header[2] = 0;
        PLCP_header[3] = 1;
      }
    }
    else
    if ( sim_params.data_rate == 54 )
    {
      if ( sim_params.useLDPC )
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 0;
        PLCP_header[2] = 1;
        PLCP_header[3] = 0;
      }
      else
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 0;
        PLCP_header[2] = 1;
        PLCP_header[3] = 1;
      }
    }
  }
  else
  {
    if ( sim_params.useLDPC )
    {
      PLCP_header[0] = 0;
      PLCP_header[1] = 0;
      PLCP_header[2] = 0;
      PLCP_header[3] = 0;
    }
    else
    {
      if ( sim_params.code_rate == 0.5 ) 
      {
        PLCP_header[0] = 0;
        PLCP_header[1] = 1;
        PLCP_header[2] = 0;
        PLCP_header[3] = 0;
      }
      else
      if ( sim_params.code_rate == 2.0/3.0 ) 
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 0;
        PLCP_header[2] = 0;
        PLCP_header[3] = 0;
      }
      else
      if ( sim_params.code_rate == 0.75 ) 
      {
        PLCP_header[0] = 1;
        PLCP_header[1] = 1;
        PLCP_header[2] = 0;
        PLCP_header[3] = 0;
      }
    }
  }
  //]

  //[ Reserved bit
  PLCP_header[4] = 0;  
  //]

  //[ Fill LENGTH field
  for ( i = 0; i < 12; i++ )
  {
    PLCP_header[5 + i] = ( packet_size >> i ) & 1;
  }
  //]

  //[ Parity bit
  PLCP_header[17] = 0;
  for ( i = 0; i < 17; i++ )
  {
    PLCP_header[17] ^= PLCP_header[i];
  }
  //]
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: process_PLCP_header
//
// Purpose: Process PLCP header
//
// Author: Alexandr Kozlov
//
// Parameters: bits_per_QAM_symbol - output, Number of bits per one modulated symbol
//
//						 num_of_bits - output, Number of coded transmitted bits
//
//             code_rate - output, Convolutional code rate
//													        
// 						 PLCP_header - input, Decoded PLCP header					        
//
// Returning value: 0 - ok, -1 - error
//////////////////////////////////////////////////////////////////////////////// */
int process_PLCP_header( unsigned* bits_per_QAM_symbol, 
                         unsigned* num_of_bits,
                         double* code_rate,
                         char* useABL,
                         char* useLDPC,
                         unsigned bits_per_OFDM_symbol,
                         unsigned* num_of_pad_bits,
                         unsigned char* PLCP_header )
{
  unsigned i;
  unsigned rate = 0;
  unsigned length = 0;
  unsigned char parity;

  unsigned num_of_OFDM_symbols;

  //[ Extract rate field
  for ( i = 0; i < 4; i++ )
  {
    rate ^= ((unsigned)PLCP_header[i]) << (3 - i);
  }
  //]
  printf("\nrate = %d\n", rate);
  //[ Extract LENGTH field
  for ( i = 0; i < 12; i++ )
  {
    length ^= ((unsigned)PLCP_header[5 + i]) << i; 
  } 
  printf("length = %d\n", length); 
  //]

  if ( rate == 13 )  // 6 Mbit/s
  {
    *bits_per_QAM_symbol = 1;
    *code_rate = 0.5;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 15 )  // 9 Mbit/s
  {
    *bits_per_QAM_symbol = 1;
    *code_rate = 0.75;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 5 )   // 12 Mbit/s
  {
    *bits_per_QAM_symbol = 2;
    *code_rate = 0.5;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 7 )   // 18 Mbit/s
  {
    *bits_per_QAM_symbol = 2;
    *code_rate = 0.75;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 9 )   // 24 Mbit/s
  {
    *bits_per_QAM_symbol = 4;
    *code_rate = 0.5;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 11 )  // 36 Mbit/s
  {
    *bits_per_QAM_symbol = 4;
    *code_rate = 0.75;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 1 )   // 48 Mbit/s
  {
    *bits_per_QAM_symbol = 6;
    *code_rate = 2.0/3.0;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 3 )   // 54 Mbit/s
  {
    *bits_per_QAM_symbol = 6;
    *code_rate = 0.75;
    *useABL = 0;
    *useLDPC = 0;
  }
  else
  if ( rate == 0 )   // LDPC + ABL
  {
    *useABL = 1;
    *useLDPC = 1;
    *code_rate = 0.75;
  }
  else
  if ( rate == 2 )   // LDPC + 64QAM
  {
    *useABL = 0;
    *useLDPC = 1;
    *code_rate = 0.75;
    *bits_per_QAM_symbol = 6;
  }
  else
  if ( rate == 10 )   // LDPC + 16QAM
  {
    *useABL = 0;
    *useLDPC = 1;
    *code_rate = 0.75;
    *bits_per_QAM_symbol = 4;
  }
  else
  if ( rate == 6 )   // LDPC + QPSK
  {
    *useABL = 0;
    *useLDPC = 1;
    *code_rate = 0.75;
    *bits_per_QAM_symbol = 2;
  }
  else
  if ( rate == 14 )   // LDPC + BPSK
  {
    *useABL = 0;
    *useLDPC = 1;
    *code_rate = 0.75;
    *bits_per_QAM_symbol = 1;
  }
  else
  if ( rate == 4 )   // CC (0.5) + ABL
  {
    *useABL = 1;
    *useLDPC = 0;
    *code_rate = 0.5;
  }
  else
  if ( rate == 8 )   // CC (2.0/3.0) + ABL
  {
    *useABL = 1;
    *useLDPC = 0;
    *code_rate = 2.0/3.0;
  }
  else
  if ( rate == 12 )   // CC (0.75) + ABL
  {
    *useABL = 1;
    *useLDPC = 0;
    *code_rate = 0.75;
  }
  else
  {
    return -1; // error unsupported data rate
  }

  if ( !(*useLDPC) )
  {
    //[ Calculate number of transmitted coded bits
    num_of_OFDM_symbols = (unsigned)ceil( (double)( 16 + 8 * length + 6 ) /
                                          (double)(bits_per_OFDM_symbol * *code_rate) );
    
    *num_of_pad_bits = (unsigned)( num_of_OFDM_symbols * bits_per_OFDM_symbol * *code_rate -
                                  ( 16 + 8 * length + 6 ) );

    *num_of_bits = num_of_OFDM_symbols * bits_per_OFDM_symbol;
    //]
  }
  else
  {
    *num_of_bits = ( 8 * length + 16 );
  }

  //[ Check length
  if ( length > sim_params.packet_size ) // to avoid memory problems
  {
    printf("length > sim_params.packet_size\n");
    printf("length = %d\n", length);
    return -1; // error unsupported LENGTH
  }
  //]
  
  //[ Check parity bit
  parity = 0;
  for ( i = 0; i < 17; i++ )
  {
    parity ^= PLCP_header[i];
  }

  if ( parity != PLCP_header[17] )
  {
    printf("parity != PLCP_header[17] \n");
    return -1; // parity bit check failed 
  }
  //]

  return 0;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: convert_flp_to_fxp
//
// Purpose: Convert floating-point value to fixed-point value
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, Floating-point value
//
//             precision - input, Fixed-point precision 
//
//						 exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
//						 overflow_flag - output, Overflow flag, 
//                                     -1 - downflow, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result
//////////////////////////////////////////////////////////////////////////////// */
int convert_flp_to_fxp( double value, 
                        unsigned precision,
                        unsigned exp_position,
                        char* overflow_flag,
                        char* overflow_comment )
{
  int fxp = 0;

  if ( value < -( 1 << ( precision - exp_position - 1 ) ) )
  {
    //[ Set fixed-point number to minimum, due to downflow
    *overflow_flag = -1;
    fxp |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) << ( precision - 1 );
    //]
  }
  else if ( value > ( 1 << ( precision - exp_position - 1 ) ) -
                      1.0 / ( 1 << exp_position ) ) 
  {
    //[ Set fixed-point number to maximum, due to overflow
    *overflow_flag = 1;
    fxp |= ( 1 << ( precision - 1 ) ) - 1;
    //]
  }
  else
  {
    //[ Convert floating-point number to fixed-point number
    *overflow_flag = 0;
    fxp = (int)( value * ( 1 << exp_position ) );
    //]
  }

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return fxp;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: convert_fxp_to_flp
//
// Purpose: Convert fixed-point value to floating-point value
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, Fixed-point value
//
//             precision - input, Fixed-point precision 
//
//						 exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
// Returning value: Result
//////////////////////////////////////////////////////////////////////////////// */
double convert_fxp_to_flp( int value, 
                           unsigned exp_position )
{
  return (double)value / (double) ( 1 << exp_position );
}


/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_scaling
//
// Purpose: Changes fixed-point value precision
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, Fixed-point value
//
//             precision - input, Fixed-point precision 
//
//             new_precision - input, New fixed-point precision
//
// Returning value: Result
//////////////////////////////////////////////////////////////////////////////// */
int fxp_scaling( int value,
                 unsigned precision,
                 unsigned new_precision,
                 char range_extention_flag,
                 char* overflow_flag,
                 char* overflow_comment )
{
  int result;

  if ( precision == new_precision )
  {
    return value; 
  }
  else 
  if ( precision > new_precision )
  {
    result = value >> ( precision - new_precision );
  }
  else
  if ( precision < new_precision )
  {
    if ( range_extention_flag )
    {
      result = value << ( new_precision - precision );

      //[ Overflow checking
      if ( result > 0 )
      {
        if ( result > ( 1 << ( new_precision - 1 ) ) - 1 )  
        {
          //[ Set fixed-point number to maximum, due to overflow
          result = 0;
          result |= ( 1 << ( precision - 1 ) ) - 1;
          *overflow_flag = 1;
          //]
        }
      }
      else
      {
        if ( result < - ( 1 << ( new_precision - 1 ) ) )
        {
          //[ Set fixed-point number to minimum, due to downflow
          result = 0;
          result |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) << ( precision - 1 );
          *overflow_flag = -1;
          //]
        }      
      }
      //]
    }
    else
    {
      result = value;
    }
  }


  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: convert_precision
//
// Purpose: Convert fixed-point value precision
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, Fixed-point value
//
//             old_precision - input, Fixed-point precision 
//
//						 old_exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
//             new_precision - input, New fixed-point precision 
//
//						 new_exp_position - input, New exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
// Returning value: Result
//////////////////////////////////////////////////////////////////////////////// */
int convert_precision( int value,
                       unsigned old_precision,
                       unsigned old_exp_position,
                       unsigned new_precision,  
                       unsigned new_exp_position,
                       char* overflow_flag,
                       char* overflow_comment )
{
  if ( ( old_precision == new_precision ) && 
       ( old_exp_position == new_exp_position ) )
  {
    return value;
  }
  else
  {
    return convert_flp_to_fxp( 
            convert_fxp_to_flp( value, old_exp_position ), 
            new_precision, 
            new_exp_position, 
            overflow_flag, overflow_comment );
  }

/*  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif */
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_add
//
// Purpose: Fixed-point addition
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, First fixed-point argument 
//
//             b - input, Second fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                     -1 - downflow, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result
//////////////////////////////////////////////////////////////////////////////// */
int fxp_add( int a, 
             int b,
             unsigned precision,
             char* overflow_flag,
             char* overflow_comment )
{
  int result = a + b;
    
  *overflow_flag = 0;

  //[ Overflow checking
  if ( ( a & ( 1 << ( precision - 1 ) ) ) && 
       ( b & ( 1 << ( precision - 1 ) ) ) && 
       !( result & ( 1 << ( precision - 1 ) ) ) )
  {
    //[ Set fixed-point number to minimum, due to downflow
    result = 0;
    result |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) << ( precision - 1 );
    *overflow_flag = -1;
    //]
  }
  else
  if ( !( a & ( 1 << ( precision - 1 ) ) ) && 
       !( b & ( 1 << ( precision - 1 ) ) ) && 
       ( result & ( 1 << ( precision - 1 ) ) ) )
  {
    //[ Set fixed-point number to maximum, due to overflow
    result = 0;
    result |= ( 1 << ( precision - 1 ) ) - 1;
    *overflow_flag = 1;
    //]
  }
  //]

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 
 
  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_sub
//
// Purpose: Fixed-point subtraction
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, First fixed-point argument 
//
//             b - input, Second fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                     -1 - downflow, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result ( a - b )
//////////////////////////////////////////////////////////////////////////////// */
int fxp_sub( int a,
             int b,
             unsigned precision,
             char* overflow_flag, 
             char* overflow_comment )
{
  int result = a - b;
    
  *overflow_flag = 0;

  //[ Overflow checking
  if ( ( a & ( 1 << ( precision - 1 ) ) ) && 
       !( b & ( 1 << ( precision - 1 ) ) ) && 
       !( result & ( 1 << ( precision - 1 ) ) ) )
  {
    //[ Set fixed-point number to minimum, due to downflow
    result = 0;
    result |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) << ( precision - 1 );
    *overflow_flag = -1;
    //]
  }
  else
  if ( !( a & ( 1 << ( precision - 1 ) ) ) && 
       ( b & ( 1 << ( precision - 1 ) ) ) && 
       ( result & ( 1 << ( precision - 1 ) ) ) )
  {
    //[ Set fixed-point number to maximum, due to overflow
    result = 0;
    result |= ( 1 << ( precision - 1 ) ) - 1;
    *overflow_flag = 1;
    //]
  }
  //]

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_mul
//
// Purpose: Fixed-point multiplication
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, First fixed-point argument 
//
//             b - input, Second fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
//						 overflow_flag - output, Overflow flag, 
//                                     -1 - downflow, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int fxp_mul( int a, 
             int b, 
             unsigned precision,
             unsigned exp_position,
             char* overflow_flag,
             char* overflow_comment )
{
  int sign = ( ( a & ( 1 << ( precision - 1 ) ) ) ^ 
               ( b & ( 1 << ( precision - 1 ) ) ) );
  int result = (a * b) >> exp_position;

  if ( a == 0 || b == 0 )  
  { 
    sign = 0;
  }

  *overflow_flag = 0;

  //[ Overflow checking
  if ( sign )
  {
    //[ Downflow checking
    if ( ( result ^ ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) << ( precision - 1 ) ) 
           >> ( precision - 1 ) )
    {
      //[ Set fixed-point number to minimum, due to downflow
      result = 0;
      result |= ( ( 1 << ( 32 - precision + 1 ) ) - 1 ) << ( precision - 1 );
      //]
      *overflow_flag = -1;
    }
    //]
  }
  else
  {
    //[ Overflow checking
    if ( result >> ( precision - 1 ) )
    {
      //[ Set fixed-point number to maximum, due to overflow
      result = 0;
      result |= ( 1 << ( precision - 1 ) ) - 1;
      *overflow_flag = 1;
      //]
    }
    //]
  }
  //]

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_not
//
// Purpose: Fixed-point invertion
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, Fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                     -1 - downflow, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result ( -a )
//////////////////////////////////////////////////////////////////////////////// */
int fxp_not( int a,
             unsigned precision, 
             char* overflow_flag,
             char* overflow_comment )
{
  int result = fxp_sub( 0, a, precision, overflow_flag, overflow_comment );

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_add
//
// Purpose: Fixed-point complex addition
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, First complex fixed-point argument 
//
//             b - input, Second complex fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
fxp_complex fxp_complex_add( fxp_complex a,
                             fxp_complex b,
                             unsigned precision, 
                             char* overflow_flag,
                             char* overflow_comment )
{
  fxp_complex result;
  char overflow_flag_re = 0;
  char overflow_flag_im = 0;
  
  *overflow_flag = 0;

  result.re = fxp_add( a.re, b.re, precision, &overflow_flag_re, overflow_comment );
  result.im = fxp_add( a.im, b.im, precision, &overflow_flag_im, overflow_comment );

  if ( overflow_flag_re || overflow_flag_im )
  {
    *overflow_flag = 1;
  }

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_sub
//
// Purpose: Fixed-point complex subtraction
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, First complex fixed-point argument 
//
//             b - input, Second complex fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
fxp_complex fxp_complex_sub( fxp_complex a, 
                             fxp_complex b, 
                             unsigned precision,
                             char* overflow_flag,
                             char* overflow_comment )
{
  fxp_complex result;
  char overflow_flag_re = 0;
  char overflow_flag_im = 0;

  *overflow_flag = 0;

  result.re = fxp_sub( a.re, b.re, precision, &overflow_flag_re, overflow_comment );
  result.im = fxp_sub( a.im, b.im, precision, &overflow_flag_im, overflow_comment );

  if ( overflow_flag_re || overflow_flag_im )
  {
    *overflow_flag = 1;
  }

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_mul
//
// Purpose: Fixed-point complex subtraction
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, First complex fixed-point argument 
//
//             b - input, Second complex fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
fxp_complex fxp_complex_mul( fxp_complex a,
                             fxp_complex b,
                             unsigned precision,
                             unsigned exp_position,
                             char* overflow_flag,
                             char* overflow_comment )
{
  fxp_complex result;
  int temp1;
  int temp2;

  char flag;

  *overflow_flag = 0;

  temp1 = fxp_mul( a.re, b.re, precision, exp_position, &flag, overflow_comment );

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  temp2 = fxp_mul( a.im, b.im, precision, exp_position, &flag, overflow_comment );

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  result.re = fxp_sub( temp1, temp2, precision, &flag, overflow_comment );

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  temp1 = fxp_mul( a.re, b.im, precision, exp_position, &flag, overflow_comment );

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  temp2 = fxp_mul( a.im, b.re, precision, exp_position, &flag, overflow_comment );

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  result.im = fxp_add( temp1, temp2, precision, &flag, overflow_comment );

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_conjugate
//
// Purpose: Fixed-point complex conjugate
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, Complex fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
fxp_complex fxp_complex_conjugate( fxp_complex a,
                                   unsigned precision,
                                   char* overflow_flag,
                                   char* overflow_comment )
{
  fxp_complex result;

  result.re = a.re;
  result.im = fxp_not( a.im, precision, overflow_flag, overflow_comment );

  #ifdef LOGGING
  if ( *overflow_flag )
  {
    logging( overflow_comment, 0 );
  }
  #endif 

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_abs2
//
// Purpose: Fixed-point complex |a|^2
//					
// Author: Alexandr Kozlov
//
// Parameters: a - input, Complex fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int fxp_complex_abs2( fxp_complex a,
                      unsigned precision,
											unsigned exp_position,
                      char* overflow_flag,
                      char* overflow_comment )
{
	int re2, im2, result;

  char flag;

  *overflow_flag = 0;

  re2 = fxp_mul( a.re,
								 a.re,
								 precision,
								 exp_position,
								 &flag,
								 overflow_comment );	

  if ( flag )
  { 
    *overflow_flag = 1;
  }

  im2 = fxp_mul( a.im,
								 a.im,
								 precision,
								 exp_position,
								 &flag,
								 overflow_comment );	
  if ( flag )
  { 
    *overflow_flag = 1;
  }

  result = fxp_add( re2,
										im2,
										precision,
										&flag,
										overflow_comment );	
  if ( flag )
  { 
    *overflow_flag = 1;
  }

	return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_add_angles
//
// Purpose: Fixed-point angles addition
//					
// Author: Alexandr Kozlov
//
// Parameters: phi1 - input, Fixed-point argument 
//
//             phi2 - input, Fixed-point argument 
//
//             precision - input, Fixed-point precision 
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int fxp_add_angles( int phi1,
                    int phi2,
                    unsigned precision )
{
  int result = phi1 + phi2;

  if ( result > 1 << ( precision - 1 ) )
  {
    result -= 2 * ( 1 << ( precision - 1 ) );
  }
  else
  if ( result < - ( 1 << ( precision - 1 ) ) )
  {
    result += 2 * ( 1 << ( precision - 1 ) );
  }

  return result;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: fxp_complex_arg
//
// Purpose: Estimating of fixed-point complex phase using CORDIC algorithm
//					
// Author: Alexandr Kozlov
//
// Parameters: value - input, Fixed-point argument 
//
//             precision - input, Fixed-point precision 
//
//						 overflow_flag - output, Overflow flag, 
//                                      1 - overflow,
//                                      0 - no overflow
//  
// Returning value: Result 
//////////////////////////////////////////////////////////////////////////////// */
int fxp_complex_arg( fxp_complex value,
                     unsigned precision,
                     char* overflow_flag,
                     char* overflow_comment )
{
  int i;

  int temp_x;

  int sign_x = value.re >= 0 ? 1 : -1;
  int sign_y = value.im >= 0 ? 1 : -1;

  int x = sign_x * value.re;
  int y = sign_y * value.im;

  int phi = 0;
  int d;
  
  for ( i = 0; i < 16; i++ )
  {
    if ( y > 0 )
    {
      d = 1;
    }
    else
    {
      d = -1;
    }
    
    temp_x = x;
    
    x = fxp_add( x,
                 ((d * y) >> i),
                 precision,
                 overflow_flag,
                 overflow_comment );

    y = fxp_sub( y,
                 ((d * temp_x) >> i),
                 precision,
                 overflow_flag,
                 overflow_comment );

    phi = fxp_sub( phi,
								   d * sim_consts.Arctan[i],
									 fxp_params.Arctan_precision,
									 overflow_flag,
									 overflow_comment );
  }

  if ( sign_x > 0 && sign_y > 0 )
  {
    return -phi;
  }
  else
  if ( sign_x > 0 && sign_y < 0 )
  {
    return phi;
  }
  else
  if ( sign_x < 0 && sign_y > 0 )
  {
    return fxp_add_angles( phi,
                            find_closest_fxp_value( Pi,
                              fxp_params.Arctan_precision,
                              2 * Pi / ( 1 << fxp_params.Arctan_precision ) ),
                            fxp_params.Arctan_precision );
  }
  else if ( sign_x < 0 && sign_y < 0 )
  {
    return fxp_add_angles(  -phi,
                            find_closest_fxp_value( -Pi,
                              fxp_params.Arctan_precision,
                              2 * Pi / ( 1 << fxp_params.Arctan_precision ) ),
                            fxp_params.Arctan_precision );
  }

  return 0;
}

fxp_complex fxp_complex_rotate( fxp_complex value,
                                int angle,
                                char* overflow_flag,
                                char* overflow_comment )
{
  return fxp_complex_mul( value,
                          sim_consts.Exp[angle+(1 << (fxp_params.Arctan_precision - 1))],
                          fxp_params.Exp_precision,
                          fxp_params.Exp_exp_position,
                          overflow_flag,
                          overflow_comment );
}

/*fxp_complex fxp_complex_rotate( fxp_complex value,
                                double phi,
                                unsigned precision,
                                unsigned exp_position,
                                char* overflow_flag,
                                char* overflow_comment )
{
  fxp_complex result;

  int i;

  int temp_x;

  int x = value.re;
  int y = value.im;
  int d;

  fxp_complex A;
  A.re = convert_flp_to_fxp( 0.6073,
                             16,
                             13,
                             overflow_flag,
                             overflow_comment );
  A.im = 0;

    precision = 16;
  
  for ( i = 0; i < 16; i++ )
  {
    if ( phi > 0 )
    {
      d = 1;
    }
    else
    {
      d = -1;
    }
    
    temp_x = x;
    
    x = fxp_add( x,
                 ((d * y) >> i),
                 precision,
                 overflow_flag,
                 overflow_comment );

    y = fxp_sub( y,
                 ((d * temp_x) >> i),
                 precision,
                 overflow_flag,
                 overflow_comment );

    phi = phi - d * sim_consts.Arctan[i];
			//atan( exp( log(2.0)*(-i) ) );
  }

  result.re = x;
  result.im = y;

  result = fxp_complex_mul( result,
                            A,
                            precision,
                            exp_position,
                            overflow_flag,
                            overflow_comment );

  return result;
}*/

void dump_gsl_complex_array(gsl_complex* data, int data_len, char* filename)
{
  FILE* fff;
  int i;

  fff=fopen(filename,"w");

  for(i=0;i<data_len;i++){
    fprintf(fff,"%6.4f\t%6.4f\n",data[i].dat[0],data[i].dat[1]);
  }

  fclose(fff);
}

void dump_fxp_complex_array(fxp_complex* data, int data_len, char* filename, char* comment )
{
  FILE* fff;
  int i;

  fff=fopen(filename,"w");

  fprintf(fff,"%s\n", comment );

  fprintf(fff,"Re\tIm\n" );

  for(i=0;i<data_len;i++){
    fprintf(fff,"%d\t%d\n",data[i].re,data[i].im );
  }

  fclose(fff);
}

void dump_fxp_array(int* data, int data_len, char* filename, char* comment )
{
  FILE* fff;
  int i;

  fff=fopen(filename,"w");

  fprintf(fff,"%s\n", comment );

  fprintf(fff,"Re\n" );

  for(i=0;i<data_len;i++){
    fprintf(fff,"%d\n",data[i] );
  }

  fclose(fff);
}


void dump_char_array( unsigned char* data, int data_len, char* filename, char* comment )
{
  FILE* fff;
  int i;

  fff=fopen(filename,"w");

  fprintf( fff, "%s\n", comment );

  for(i=0;i<data_len;i++){
    fprintf(fff,"%d\n", (unsigned)data[i] );
  }

  fclose(fff);
}

