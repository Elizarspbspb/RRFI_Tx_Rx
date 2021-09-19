#include "ADC.h"
#include "sim_consts.h"
#include "sim_params.h"
#include <math.h>
#include <malloc.h>
#include <stdio.h>

// local function prototypes
int ADC_linear(double a);
int ADC_table(double a);

#ifndef USE_TABLE_ADC
#  define routine_ADC ADC_linear
#else
#  define routine_ADC ADC_table
#endif


////////////////////////////////////////////////////////////////////////////////
// Name: ADC_11a
//
// Purpose: Analog to digital convertion
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input, Floating-point time domain signal
//              
//             fxp_time_signal - output, Fixed-point time domain signal
//
//             num_of_samples - input, Number of samples
//
// Returning value: None
////////////////////////////////////////////////////////////////////////////////
void ADC_11a( gsl_complex* time_signal,
              fxp_complex* fxp_time_signal,
              unsigned num_of_samples )
{
  unsigned i,l;

  gsl_complex flp_signal[PreambleLength + MaxNumOfSymbols *
      (CyclicPrefixLength + NumOfSubcarriers)];

  l = 0;

  for ( i = 0; i < num_of_samples; i++ )
  {
    fxp_time_signal[l].re = routine_ADC(time_signal[i].dat[0]);
    fxp_time_signal[l].im = routine_ADC(time_signal[i].dat[1]);

    l++;
  } 
}

////////////////////////////////////////////////////////////////////////////////
// Name: ADC_linear
//
// Purpose: Analog to digital value convertion (linear)
//
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: a - input, Floating-point value
//
// Returning value: fixed-point quantized value
////////////////////////////////////////////////////////////////////////////////
int ADC_linear(double a){
  int N = 1 << fxp_params.ADC_DAC_precision;

  if ( a >= fxp_params.ADC_DAC_max ){
    return (N/2 - 1);
  }
  else if ( a < -fxp_params.ADC_DAC_max ){
    return (-N / 2);
  }
  else {
    return (int)(
      floor( ( a + fxp_params.ADC_DAC_max ) * N /
               ( 2 * fxp_params.ADC_DAC_max ) ) - N / 2
           );
  }

}

//// Table for non-linear ADC
double* ADC_table_values;

////////////////////////////////////////////////////////////////////////////////
// Name: ADC_table
//
// Purpose: Analog to digital value convertion (non-linear, table)
//
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: a - input, Floating-point value
//
// Returning value: fixed-point quantized value
////////////////////////////////////////////////////////////////////////////////
int ADC_table(double a){
  int l,u;
  int m;
  int N;
  int r;
    
  N= 1 << fxp_params.ADC_DAC_precision;
  l = 0;
  u = N-1;

  if ( a >= ADC_table_values[u] ){
    r=u;
  }
  else if ( a < ADC_table_values[l] ){
    r=l;
  }
  else {
    while(l<u-1){
      m = (l+u)/2;
      if(a<ADC_table_values[m]){
        u=m;
      } else {
        l=m;
      }
    }
    r=l;
  }

  return r - (N/2);
}

////////////////////////////////////////////////////////////////////////////////
// Name: init_ADC_table
//
// Purpose: Initializes table for analog to digital non-linear value convertion
//
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: none
//
// Returning value: none
////////////////////////////////////////////////////////////////////////////////
void init_ADC_table(){
  int i;
  int N;
  double d;
  double s;

  N = 1 << fxp_params.ADC_DAC_precision;
  ADC_table_values = (double*) malloc( N*sizeof(double) );

  s=-fxp_params.ADC_DAC_max;
  d=2.0*fxp_params.ADC_DAC_max / N;

  for(i=0;i<N;i++){
    ADC_table_values[i]=s+i*d;
  }

  //////////////////////////////////
  //     Added array deletion, otherwise there was a memory leak
  //
        delete[] ADC_table_values;
  //
  //
  ////////////////////////////////////
}