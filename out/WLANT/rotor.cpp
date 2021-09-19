#include "rotor.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "model.h"

/**
////////////////////////////////////////////////////////////////////////////////
// Name: rotor_11a
//
// Purpose: Frequency error correction
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input/output, Received time domain
//
//             radians_per_sample - input, radians per sample rotation
//
//						 num_of_samples - input, Number of received samples
//  
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void rotor_11a( fxp_complex* time_signal, 
                int radians_per_sample,
                unsigned num_of_samples )
{
  unsigned j;
  char overflow_flag;

  #ifdef LOGGING
  logging( "rotor_11a...\n", 1 );
  #endif

	//[ Correct signal
  for ( j = 0; j < num_of_samples; j++ )
  {
	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: time_signal[j] = fxp_complex_rotate( time_signal[%d]\n", j );
	  #endif    

		time_signal[j] =
      fxp_complex_rotate( time_signal[j],
      	                  rotor_state_angle,
                          &overflow_flag,
                          overflow_comment );

	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: rotor_state_angle = fxp_add_angles, j=%d\n", j );
	  #endif    

    rotor_state_angle = fxp_add_angles(
                                rotor_state_angle,
                                radians_per_sample,
                                fxp_params.Arctan_precision );

  }
  //]

  #ifdef LOGGING
  logging( "rotor_11a finished\n", -1 );
  #endif
}