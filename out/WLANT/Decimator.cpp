#include "Decimator.h"
#include "sim_consts.h"
#include "sim_params.h"
#include <memory.h>
#include <gsl/gsl_complex_math.h>
#include "Interpolation.h"
#include <assert.h>
#include "model.h"
#include <math.h>
#include <vector>
#include <complex>



static gsl_complex buf[( ExtraNoiseSamples +
                               ( PreambleLength +  MaxNumOfSymbols * 
                               ( CyclicPrefixLength + NumOfSubcarriers ) ) + 
                              1000 - 1 )
                            ];

/**
////////////////////////////////////////////////////////////////////////////////
// Name: Decimator
//
// Purpose: Decimates oversampled signal
//
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: time_signal - input/output, Floating-point time domain signal
//      
//             num_of_samples - input, Number of samples
//              
// Returning value: number of downsampled samples
//////////////////////////////////////////////////////////////////////////////// */
int Decimator( gsl_complex* time_signal, 
	            unsigned num_of_samples )
{
  int num_of_decimated_samples;

  //[ Decimate signal
  num_of_decimated_samples=Decimation( time_signal, buf, num_of_samples, 2 );
  //]
  memcpy( time_signal, buf,
          num_of_decimated_samples * sizeof(gsl_complex)
        );
  return num_of_decimated_samples;
}

/**
////////////////////////////////////////////////////////////////////////////////
// Name: Drift_And_Jitter
//
// Purpose: Performs drift and jittering
//
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: time_signal - input/output, Floating-point time domain signal
//      
//             num_of_samples - input, Number of samples
//              
// Returning value: number samples in processed signal
//////////////////////////////////////////////////////////////////////////////// */
int Drift_And_Jitter( gsl_complex* time_signal, 
	            unsigned num_of_samples )
{
  //[ params
  double DRIFT_FREQ_K=0.25e-7 / (double)(1<<sim_params.sampling_power);
  double JITTER_STDDEV=0.0333;
  double JITTER_MAX_DELTA=0.1;
  //]

  double d;
  double delta_d;
  double real_d;
  double floor_d;
  int k=0;
  int i1,i2;
  gsl_complex s1,s2;

//  dump_gsl_complex_array(time_signal+num_of_samples-100,100,"src.log");

  //[ Initial ofset 
  d=gsl_ran_flat( RNG, 0, 1<<sim_params.sampling_power );
  //]

  while(d<=num_of_samples-1){
    //[ Current sample ofset
//    delta_d=gsl_ran_gaussian(RNG,JITTER_STDDEV);
    delta_d=gsl_ran_flat(RNG, -JITTER_MAX_DELTA, +JITTER_MAX_DELTA);
    if(delta_d > JITTER_MAX_DELTA){
      delta_d = JITTER_MAX_DELTA;
    }
    else if(delta_d < -JITTER_MAX_DELTA){
      delta_d = -JITTER_MAX_DELTA;
    }
    //]

    real_d=d+delta_d;
    floor_d=floor(real_d);
    i1=(int)floor_d;
    i2=i1+1;

    s1=time_signal[i1];
    s2=time_signal[i2];

    //[ linear interpolation using neighbour samples
    if(real_d-floor_d>0.5){
      buf[k]=gsl_complex_add(s1,gsl_complex_mul_real(
                                gsl_complex_sub(s2,s1), real_d-floor_d
                              )
            );
    } else {
      buf[k]=gsl_complex_add(s2,gsl_complex_mul_real(
                                gsl_complex_sub(s1,s2), 1.0-(real_d-floor_d)
                              )
            );

    } //]

    //[ Applying drift
    d+=1.0;
    d+=DRIFT_FREQ_K;
    //]

    k++;

  }

  num_of_samples=k;
  memcpy( time_signal, buf,
          num_of_samples * sizeof(gsl_complex)
        );


//  dump_gsl_complex_array(time_signal+num_of_samples-100,100,"dest.log");
//  exit(1);

  return num_of_samples;

}