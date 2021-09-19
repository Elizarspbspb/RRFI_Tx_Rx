#include "commonfunc.h"
#include "channel.hpp"
#include "model.h"
#include "sim_consts.h"
#include "sim_params.h"
#include <math.h>
#include <memory.h>
#include <gsl/gsl_complex_math.h>

////////////////////////////////////////////////////////////////////////////////
// Name: create_freq_offset
//
// Purpose: Create frequency offset
//					
// Author: Alexandr Kozlov
//
// Parameters: input_signal - input/output, Time domain signal
//
//             num_of_samples - input, Number of input signal samples  
//           
// Returning value: None
////////////////////////////////////////////////////////////////////////////////
void create_freq_offset( gsl_complex* input_signal, // [num_of_samples]
                         unsigned num_of_samples )
{
	unsigned i;
	double time_base;
  gsl_complex phase_rotation;

  for( i = 0; i < num_of_samples; i++)
  {
	  time_base = (double)i / (double)(SampFreq* (1 << sim_params.sampling_power));
     
    phase_rotation = gsl_complex_exp( 
                      gsl_complex_mul_real( 
                        gsl_complex_rect( 0, 2 * Pi ), 
                        FreqOffset * time_base ) 
                     );  

    input_signal[i] = gsl_complex_mul( input_signal[i], phase_rotation );
	}
}

////////////////////////////////////////////////////////////////////////////////
// Name: get_channel_ir_len
//
// Purpose: Returns channel IR length
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: none
//  
// Returning value: Impulse response duration
////////////////////////////////////////////////////////////////////////////////
unsigned get_channel_ir_len(){
   int Kmax = (unsigned)ceil( 10 * ( ExpDecayTrms * (1e-9) ) * (SampFreq* (1 << sim_params.sampling_power)) );

   return Kmax+1;
}

////////////////////////////////////////////////////////////////////////////////
// Name: get_channel_ir
//
// Purpose: Generate channel impulse response
//					
// Author: Alexandr Kozlov
//
// Parameters: cir - output, Channels impulse response
//  
// Returning value: Impulse response duration
////////////////////////////////////////////////////////////////////////////////
unsigned get_channel_ir( gsl_complex* cir )
{
  unsigned i, Kmax;
  double var0, env, std_dev;

  Kmax = get_channel_ir_len()-1;
  var0 = ( 1 - exp ( - 1 / ( (SampFreq* (1 << sim_params.sampling_power)) * ( ExpDecayTrms * (1e-9) ) ) ) ) / 
         ( 1 - exp( -1 * ( ( Kmax + 1 ) * (SampFreq* (1 << sim_params.sampling_power)) / ( ExpDecayTrms * (1e-9) ) ) ) );

 // Kmax = 0;

  for ( i = 0; i < Kmax + 1; i++ )
  {
    env = var0 * exp( -(double)i / ( (SampFreq* (1 << sim_params.sampling_power)) * ( ExpDecayTrms * (1e-9) ) ) );
    std_dev = sqrt(env/2);
   
    cir[i] = gsl_complex_rect( gsl_ran_gaussian( RNG, std_dev ),
                               gsl_ran_gaussian( RNG, std_dev ) );
  }

  return Kmax + 1;
}

////////////////////////////////////////////////////////////////////////////////
// Name: channel
//
// Purpose: Implements signal transmission over 802.11 exponential decay channel
//					
// Author: Alexandr Kozlov
//
// Parameters: received_signal - output, Time domain signal after channel 
//                               
//             time_signal - input, Time domain signal 
//
//             cir - input, channel impulse response 
//
//						 num_of_samples - input, number of input signal samples	
//
//             ir_duration - input, impulse response duration
//
//             SNR - input, signal-to-noise ratio at receiver
//  
// Returning value: Number of received samples
////////////////////////////////////////////////////////////////////////////////
unsigned channel( gsl_complex* received_signal, 
                  gsl_complex* time_signal,
                  gsl_complex* cir, 
                  unsigned num_of_samples,
                  unsigned ir_duration, 
                  double SNR )
{
  unsigned i;
	double noise_std_dev;
  double average_signal_energy = 0;
  double new_average_signal_energy = 0;

  memset( received_signal, 
          0, 
          ( ExtraNoiseSamples + num_of_samples + ir_duration - 1 ) * 
          sizeof( gsl_complex )
        );

  signal_complex_convolution( time_signal, 
                              num_of_samples, 
                              cir,
                              ir_duration,
                              received_signal + ExtraNoiseSamples );


  //[ Convert signal energy using SNR and NOISE_POWER (for AGC test)
  for ( i = 0; i < ( num_of_samples + ir_duration - 1 ); i++ )
  {
    average_signal_energy += gsl_complex_abs2( received_signal[i + ExtraNoiseSamples] );
  }

  average_signal_energy /= ( num_of_samples + ir_duration - 1 );

  noise_std_dev = sqrt( exp( log(10) * ( (double)NOISE_POWER / 10 ) ) * (10e3) / 
                    ((double)SampFreq ) / 2 );

  new_average_signal_energy = 2 * noise_std_dev * noise_std_dev * 
                              exp( log(10) * ( SNR / 10 ) );

 	for ( i = 0; i < num_of_samples + ir_duration - 1; i++ )
	{
  	received_signal[i+ExtraNoiseSamples] =
      gsl_complex_mul( received_signal[i+ExtraNoiseSamples], 
                       gsl_complex_rect( sqrt( new_average_signal_energy /
                                               average_signal_energy ), 0 ) );
	}

  noise_std_dev *= sqrt( (double) ( 1 << sim_params.sampling_power ) );
  //]

	//[ Add noise
	for ( i = 0; i < ExtraNoiseSamples + 160 * (1 << sim_params.sampling_power); i++ )
	{
  	received_signal[i] = gsl_complex_add( received_signal[i],
															gsl_complex_rect( 
                                gsl_ran_gaussian( RNG, noise_std_dev ),
                                gsl_ran_gaussian( RNG, noise_std_dev )
                              ) 
                         );
	}

  if ( !sim_params.idealChannelEstimation )
  {
    for ( i = ExtraNoiseSamples + 160 * (1 << sim_params.sampling_power);
          i < ExtraNoiseSamples + 
          ( 160 + 2 * CyclicPrefixLength + 2 * NumOfSubcarriers ) * 
          (1 << sim_params.sampling_power); i++ )
	  {
  	  received_signal[i] = gsl_complex_add( received_signal[i],
															  gsl_complex_rect( 
                                  gsl_ran_gaussian( RNG, noise_std_dev ),
                                  gsl_ran_gaussian( RNG, noise_std_dev )
                                ) 
                           );
	  }
  }

	for ( i = ExtraNoiseSamples + 
        ( 160 + 2 * CyclicPrefixLength + 2 * NumOfSubcarriers ) * (1 << sim_params.sampling_power); 
        i < ExtraNoiseSamples + num_of_samples + ir_duration - 1; i++ )
	{
  	received_signal[i] = gsl_complex_add( received_signal[i],
															gsl_complex_rect( 
                                gsl_ran_gaussian( RNG, noise_std_dev ),
                                gsl_ran_gaussian( RNG, noise_std_dev )
                              ) 
                         );
	}
  //]

  if ( !sim_params.idealFrequencySync )
  {
    //[ Create frequency offset
    create_freq_offset( received_signal, 
                        ExtraNoiseSamples + num_of_samples + ir_duration - 1 );
    //]
  }
 
	return ExtraNoiseSamples + num_of_samples + ir_duration - 1;
}


///////////////////////////////////////// AWGN channel

////////////////////////////////////////////////////////////////////////////////
// Name: get_AWGN_channel_ir_len
//
// Purpose: Returns AWGN channel IR length (always equal to 1)
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: none
//  
// Returning value: Impulse response duration (always equal to 1)
////////////////////////////////////////////////////////////////////////////////
unsigned get_AWGN_channel_ir_len(){
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Name: get_AWGN_channel_ir
//
// Purpose: AWGN channel wrapper
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: cir - output, Channels impulse response
//  
// Returning value: Impulse response duration
////////////////////////////////////////////////////////////////////////////////
unsigned get_AWGN_channel_ir( gsl_complex* cir )
{
  cir[0] = gsl_complex_rect( 1.0, 0 );

  return get_AWGN_channel_ir_len();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
// DopplerChannel Wrapper functions

#include "DopplerChannel/DopplerChannel.h"
#include "DopplerChannel/RNG.h"

// Random generators for channel updating
static RNG_StateStruct RngChannelState;

// Doppler channel parameters and state
static ChannelParamsStruct DopplerChannelParams;
static ChannelStateStruct DopplerChannelState;

// Current time in muSec. Used to propagate doppler effect.
static float curTime_muSec;

#define DOPPLER_CHANNEL_SUBPACKET_LEN 1000

////////////////////////////////////////////////////////////////////////////////
// Name: initDopplerChannel
//
// Purpose: Initializes channel, allocates memory
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: channelFileName - input, filename that hold the channel description 
//                               
//             channelModelName - input, the name of model to use 
//  
// Returning value: Non-zero if error occured
////////////////////////////////////////////////////////////////////////////////
int initDopplerChannel(
    char                       *channelFileName ,
    char                       *channelModelName               
                   
                  ){

  float freqCoef=1;

  if(sim_params.sampling_power<3){
    freqCoef = 8.0;
  } else {
    freqCoef = (float)(1 << sim_params.sampling_power);
  }


  InitRNG(1,&RngChannelState);

  Initialize_Channel (
    /*char                       *channelFileName */              channelFileName,
    /*char                       *channelModelName */             channelModelName,  
    /*int                         numTxAntenna */                 1,
    /*int                         numRxAntenna */                 1,
    /*float                       sampleRate_Msps */              SampFreq/1.0e6*freqCoef, // for oversampling
    /*boolean                     pwrNormFlag */                  (boolean)0,
    /*boolean                     freqDomainFlag */               (boolean)0,
    /*int                         numSubCarrier */                NumOfSubcarriers, // (?)Not used in TimeDomain mode
    /*float                       subCarrierSpacing_kHz */        0, // Not used in TimeDomain mode
    /*RNG_StateStruct            *chRngState */                   &RngChannelState,
    /*RNG_StateStruct            *noiseRngState */                NULL,// not used
    /*ChannelStateStruct         *channelState */                 &DopplerChannelState,
    /*ChannelParamsStruct        *channelParams*/                 &DopplerChannelParams
  );

  curTime_muSec=0;
  GenerateImpulseResponse( /*time_muSec*/ curTime_muSec, &DopplerChannelState, &DopplerChannelParams );

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Name: destroyDopplerChannel
//
// Purpose: Destroys channel, deallocates memory
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: none
//  
// Returning value: Non-zero if error occured
////////////////////////////////////////////////////////////////////////////////
int destroyDopplerChannel(){
  Wrapup_Channel( &DopplerChannelState, &DopplerChannelParams );

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Name: updateCurrentImpulseResponse
//
// Purpose: Updates channel impulse response propagating Doppler effect if time passed since
//             last update is big enough.
//          The generated impulse response is contained in channelState->h_I and channelState->h_Q.
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: none
//  
// Returning value: Non-zero if IR was updated during last call to this method
////////////////////////////////////////////////////////////////////////////////
int updateCurrentImpulseResponse(){
  return UpdateImpulseResponse( curTime_muSec, &DopplerChannelState, &DopplerChannelParams );
}

////////////////////////////////////////////////////////////////////////////////
// Name: dopplerChannel
//
// Purpose: Implements signal transmission over 802.11 exponential decay channel with Doppler effect
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: received_signal - output, Time domain signal after channel 
//                               
//             time_signal - input, Time domain signal 
//
//             cir - input, channel impulse response 
//
//						 num_of_samples - input, number of input signal samples	
//
//             ir_duration - input, impulse response duration
//
//             SNR - input, signal-to-noise ratio at receiver
//  
// Returning value: Number of received samples
////////////////////////////////////////////////////////////////////////////////
unsigned dopplerChannel( gsl_complex* received_signal, 
                         gsl_complex* time_signal,
                         gsl_complex* cir, 
                         unsigned num_of_samples,
                         unsigned ir_duration,  
                         double SNR )
{
  unsigned i;
	double noise_std_dev;
  double average_signal_energy = 0;
  double new_average_signal_energy = 0;
  unsigned samplesLeft=num_of_samples;
  unsigned samplesToTransmit;

  memset( received_signal, 
          0, 
          ( ExtraNoiseSamples + num_of_samples + ir_duration - 1 ) * 
          sizeof( gsl_complex )
        );

  for(i=0;i<num_of_samples;i+=DOPPLER_CHANNEL_SUBPACKET_LEN){
    samplesToTransmit=(DOPPLER_CHANNEL_SUBPACKET_LEN<samplesLeft)?DOPPLER_CHANNEL_SUBPACKET_LEN:samplesLeft;
    signal_complex_convolution( time_signal+i, 
                                samplesToTransmit, 
                                cir,
                                ir_duration,
                                received_signal+i + ExtraNoiseSamples );

    samplesLeft-=DOPPLER_CHANNEL_SUBPACKET_LEN;

    //[ Propagate Doppler to current impulse response
    curTime_muSec+=(float)((float)samplesToTransmit * 1.0e6f / ( SampFreq * (1 << sim_params.sampling_power) ) );
    get_dopplerChannel_ir(cir);
    //]
  }

  updateCurrentImpulseResponse();

  //[ Convert signal energy using SNR and NOISE_POWER (for AGC test)
  for ( i = 0; i < ( num_of_samples + ir_duration - 1 ); i++ )
  {
    average_signal_energy += gsl_complex_abs2( received_signal[i + ExtraNoiseSamples] );
  }

  average_signal_energy /= ( num_of_samples + ir_duration - 1 );

  noise_std_dev = sqrt( exp( log(10) * ( (double)NOISE_POWER / 10 ) ) * (10e3) / 
                    ((double)SampFreq ) / 2 );

  new_average_signal_energy = 2 * noise_std_dev * noise_std_dev * 
                              exp( log(10) * ( SNR / 10 ) );

 	for ( i = 0; i < num_of_samples + ir_duration - 1; i++ )
	{
  	received_signal[i+ExtraNoiseSamples] = 
      gsl_complex_mul( received_signal[i+ExtraNoiseSamples], 
                       gsl_complex_rect( sqrt( new_average_signal_energy /
                                               average_signal_energy ), 0 ) );
	}
  
  noise_std_dev *= sqrt( (double) ( 1 << sim_params.sampling_power ) );
  //]
    
	//[ Add noise
	for ( i = 0; i < ExtraNoiseSamples + 160 * (1 << sim_params.sampling_power); i++ )
	{
  	received_signal[i] = gsl_complex_add( received_signal[i],
															gsl_complex_rect( 
                                gsl_ran_gaussian( RNG, noise_std_dev ),
                                gsl_ran_gaussian( RNG, noise_std_dev )
                              ) 
                         );
	}

  if ( !sim_params.idealChannelEstimation )
  {
    for ( i = ExtraNoiseSamples + 160 * (1 << sim_params.sampling_power);
          i < ExtraNoiseSamples + 
          ( 160 + 2 * CyclicPrefixLength + 2 * NumOfSubcarriers ) * 
          (1 << sim_params.sampling_power); i++ )
	  {
  	  received_signal[i] = gsl_complex_add( received_signal[i],
															  gsl_complex_rect( 
                                  gsl_ran_gaussian( RNG, noise_std_dev ),
                                  gsl_ran_gaussian( RNG, noise_std_dev )
                                ) 
                           );
	  }
  }

	for ( i = ExtraNoiseSamples + 
        ( 160 + 2 * CyclicPrefixLength + 2 * NumOfSubcarriers ) * (1 << sim_params.sampling_power); 
        i < ExtraNoiseSamples + num_of_samples + ir_duration - 1; i++ )
	{
  	received_signal[i] = gsl_complex_add( received_signal[i],
															gsl_complex_rect( 
                                gsl_ran_gaussian( RNG, noise_std_dev ),
                                gsl_ran_gaussian( RNG, noise_std_dev )
                              ) 
                         );
	}
  //]

  if ( !sim_params.idealFrequencySync )
  {
    //[ Create frequency offset
    create_freq_offset( received_signal, 
                        ExtraNoiseSamples + num_of_samples + ir_duration - 1 );
    //]
  }
 
	return ExtraNoiseSamples + num_of_samples + ir_duration - 1;

}

////////////////////////////////////////////////////////////////////////////////
// Name: get_dopplerChannel_ir_len
//
// Purpose: Returns Doppler channel IR length
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: none
//  
// Returning value: Impulse response duration
////////////////////////////////////////////////////////////////////////////////
unsigned get_dopplerChannel_ir_len(){
  if(sim_params.sampling_power<3){
    return DopplerChannelParams.numDelay >> (3 - sim_params.sampling_power);
  }
  return DopplerChannelParams.numDelay;
}

////////////////////////////////////////////////////////////////////////////////
// Name: get_dopplerChannel_ir
//
// Purpose: Generate channel impulse response applying Doppler effect
//					
// Author: Alexandr Kozlov, Vladislav Chernyshev
//
// Parameters: cir - output, Channels impulse response
//  
// Returning value: Impulse response duration
////////////////////////////////////////////////////////////////////////////////
unsigned get_dopplerChannel_ir( gsl_complex* cir )
{
  unsigned i, ir_len;
  int dec;

  if(sim_params.sampling_power<3){
    dec = (3 - sim_params.sampling_power);
  } else {
    dec = 0;
  }

  ir_len = get_dopplerChannel_ir_len();

  for ( i = 0; i < ir_len; i++ ){
    cir[i] = gsl_complex_rect( DopplerChannelState.h_I[0][0][i<<dec], DopplerChannelState.h_Q[0][0][i<<dec]);
  }

  return ir_len;
}


