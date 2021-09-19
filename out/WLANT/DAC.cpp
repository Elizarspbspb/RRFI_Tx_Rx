#include "DAC.h" 
#include "model.h"
#include "sim_params.h"
#include "sim_consts.h"
#include "commonfunc.h"
#include "Interpolation.h"
#include <memory.h>
#include <gsl/gsl_complex_math.h>

/**
////////////////////////////////////////////////////////////////////////////////
// Name: DAC_11a
//
// Purpose: Digital to analog convertion
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - output, Floating-point time domain signal
//              
//             fxp_time_signal - input, Fixed-point time domain signal
//
//             precision - input, Fixed-point precision 
//
//						 exp_position - input, Exponent point position,
//                                   for example: 1010.10 exp_position = 2
//
//             num_of_samples - input, Number of samples
//
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
unsigned DAC_11a( gsl_complex* time_signal,
                  fxp_complex* fxp_time_signal,
                  unsigned precision, 
                  unsigned exp_position,
                  unsigned num_of_samples )
{
  unsigned i;
  unsigned num_of_oversampled_samples = ( 1 << sim_params.sampling_power ) * num_of_samples ;

  gsl_complex flp_signal[ PreambleLength + MaxNumOfSymbols * 
                          ( CyclicPrefixLength + NumOfSubcarriers )];

  #ifdef LOGGING
  logging( "DAC_11a...\n", 1 );
  #endif

  //[ Convert to floating-point signal
  printf("\nDAC\n");
  for(i=0;i<num_of_samples;i++)
  {
    flp_signal[i].dat[0]=convert_fxp_to_flp( fxp_time_signal[i].re, exp_position );
    flp_signal[i].dat[1]=convert_fxp_to_flp( fxp_time_signal[i].im, exp_position );
  }
  //]

//  dump_gsl_complex_array(flp_signal, num_of_samples, "before_inter.log");

  num_of_oversampled_samples = Interpolation( flp_signal,
                                              time_signal,
                                              num_of_samples,
                                              1 << sim_params.sampling_power
                                            );

  #ifdef LOGGING
  logging( "DAC_11a finished\n", -1 );
  #endif

  return num_of_oversampled_samples;
}
