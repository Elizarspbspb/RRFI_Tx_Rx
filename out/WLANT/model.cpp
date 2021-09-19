#include "model.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include <math.h>
#include <gsl/gsl_complex_math.h>
#include <iostream>
#include "channel.hpp"
#include "ADC.h"
#include "bitloading.h"
#include "demodulatorABL.h"
#include "agc.h"
#include "time_sync.h"
#include "golay_lib/golay_wrap.h"

gsl_rng* RNG;

void initialize_model()
{
  int i,k,z;
  double distance;
  double distance_step;
  double power;
  char overflow_flag;
  int num_of_signal_levels;
  gsl_complex exp_value;

#ifdef LOGGING
  debug_log = fopen( "debug_log.log", "w" );
  logging( "Model initialized\n", 0 );
#endif

  //[ Initialize random generator
  //gsl_rng_env_setup();          // Commented out because there was a memory leak
  //RNG = gsl_rng_alloc( gsl_rng_mt19937 );       // Commented out because there was a memory leak
  //]

  //[ Initialize coding and modulation parameters
  if ( sim_params.data_rate == 6 )
  {
    sim_params.bits_per_QAM_symbol = 1;
    sim_params.code_rate = 0.5;
    sim_params.data_bits_per_OFDM_symbol = 24;
  }
  else 
  if ( sim_params.data_rate == 9 )
  {
    sim_params.bits_per_QAM_symbol = 1;
    sim_params.code_rate = 0.75;
    sim_params.data_bits_per_OFDM_symbol = 36;
  }
  else 
  if ( sim_params.data_rate == 12 )
  {
    sim_params.bits_per_QAM_symbol = 2;
    sim_params.code_rate = 0.5;
    sim_params.data_bits_per_OFDM_symbol = 48;
  }
  else 
  if ( sim_params.data_rate == 18 )
  {
    sim_params.bits_per_QAM_symbol = 2;
    sim_params.code_rate = 0.75;
    sim_params.data_bits_per_OFDM_symbol = 72;
  }
  else 
  if ( sim_params.data_rate == 24 )
  {
    sim_params.bits_per_QAM_symbol = 4;
    sim_params.code_rate = 0.5;
    sim_params.data_bits_per_OFDM_symbol = 96;
  }
  else 
  if ( sim_params.data_rate == 36 )
  {
    sim_params.bits_per_QAM_symbol = 4;
    sim_params.code_rate = 0.75;
    sim_params.data_bits_per_OFDM_symbol = 144;
  }
  else 
  if ( sim_params.data_rate == 48 )
  {
    sim_params.bits_per_QAM_symbol = 6;
    sim_params.code_rate = 2.0/3.0;
    sim_params.data_bits_per_OFDM_symbol = 192;
  }
  else 
  if ( sim_params.data_rate == 54 )
  {
    sim_params.bits_per_QAM_symbol = 6;
    sim_params.code_rate = 0.75;
    sim_params.data_bits_per_OFDM_symbol = 216;
  }
  else
  {
      std::cout << "Invalid data_rate in Configuration.cfg" << std::endl;
      exit(1);
  }
  //]

  distance_step = 2.16 / ( 1 << fxp_params.QAM_precision );

  //[ Initialize QAM (signal levels and Gray mapping)
  for ( z = 0; z <= 36; z++ )
  {
    power = sqrt( (double)NumOfDataSubcarriers / (double)( NumOfDataSubcarriers - z ) );

    sim_consts.QAMSignalLevels[z][0][0] = 
      find_closest_fxp_value( -1 * power, fxp_params.QAM_precision, distance_step );
    sim_consts.QAMSignalLevels[z][0][1] = 
      find_closest_fxp_value( 1 * power, fxp_params.QAM_precision, distance_step );
  


    for ( k = 1; k <= MaxBitsPerQAMSymbol / 2; k++ )
    {
      num_of_signal_levels = 1 << k;

      distance = sqrt ( 3.0 / ( ( num_of_signal_levels + 1 ) * 
                                  ( 2 * num_of_signal_levels - 2 ) ) );

      #ifdef LOGGING
      sprintf( overflow_comment, "Overflow: QAMDistances[%d] = convert_flp_to_fxp\n", k );
      #endif

      sim_consts.QAMThresholds[z][k] =
        convert_flp_to_fxp( 2 * distance * ( (double)num_of_signal_levels / 4 ) * power,
                            fxp_params.Demodulator_precision,
                            fxp_params.Demodulator_exp_position,
                            &overflow_flag,
                            overflow_comment );

      for( i = 0; i < num_of_signal_levels; i++ )
      {
        sim_consts.QAMSignalLevels[z][k][i ^ (i >> 1)] = 
          find_closest_fxp_value( ( 2 * i + 1 - num_of_signal_levels ) * distance * power,
                                  fxp_params.QAM_precision,
                                  distance_step );        
      }
    }
  }
  //]

  //[ Init exp and atan tables
	for ( i = 0; i < 16; i++ )
	{
		sim_consts.Arctan[i] = find_closest_fxp_value(
																atan( exp( log(2) * (-i) ) ),
                                fxp_params.Arctan_precision,
                                2 * Pi / ( 1 << fxp_params.Arctan_precision ) );
	}

  for ( i = -(1 << (fxp_params.Arctan_precision - 1)); 
        i < (1 << (fxp_params.Arctan_precision - 1)); 
        i++ )
  {
    exp_value =
      gsl_complex_exp( 
        gsl_complex_rect( 0, 
                          i * ( 2.0 * Pi / ( 1 << fxp_params.Arctan_precision ) )
                        )
                     );

    sim_consts.Exp[i+(1 << (fxp_params.Arctan_precision - 1))].re =  
      convert_flp_to_fxp( exp_value.dat[0],
                          fxp_params.Exp_precision,
                          fxp_params.Exp_exp_position,
                          &overflow_flag,
                          overflow_comment );

    sim_consts.Exp[i+(1 << (fxp_params.Arctan_precision - 1))].im =  
      convert_flp_to_fxp( exp_value.dat[1],
                          fxp_params.Exp_precision,
                          fxp_params.Exp_exp_position,
                          &overflow_flag,
                          overflow_comment );
  }
  //]
    init_ADC_table();
	init_AGCAddrModLUT(); //K.Zakharchenko
	init_timing_matched_filter();
	//golay_handle = createGolayCode(); // memory leak
  //init_NEW_bitloading_constants();

  if(ctDoppler == sim_params.channel_type){
    initDopplerChannel(sim_params.DOPPLER_MODEL_FILENAME, sim_params.DOPPLER_MODEL_NAME);
  }
}

void destroy_model()
{
  //[ Free random generator
  gsl_rng_free (RNG); 
  //]

  destroyGolayCode( golay_handle );

  if(ctDoppler == sim_params.channel_type){
    destroyDopplerChannel();
  }

  #ifdef LOGGING
  logging( "Model finished\n", 0 );
  fclose( debug_log );
  #endif

}

#ifdef LOGGING
void logging( char* log_string, char block_begin )
{
  unsigned i;
  static unsigned num_of_tabs = 0;

  if ( block_begin < 0 )
  {
    num_of_tabs += block_begin;
  }

  for ( i = 0; i < num_of_tabs; i++ )
  {
    fprintf( debug_log, "\t" );
  }

  fprintf( debug_log, log_string );

  fflush( debug_log );

  if ( block_begin > 0 )
  {
    num_of_tabs += block_begin;
  }
}
#endif