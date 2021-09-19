#include "VGA.h"
#include <math.h>

#define VGAGainChangePerSample exp( 1.0 / 4.0 )

int VGA_gain = 127;

int curVGA_gain = 127;

/**
////////////////////////////////////////////////////////////////////////////////
// Name: VGA_11a
//
// Purpose: Variable gain amplifier
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input/output, Floating-point time domain signal
//      
//             num_of_samples - input, Number of samples
//              
// Returning value: None
//////////////////////////////////////////////////////////////////////////////// */
void VGA_11a( gsl_complex* time_signal, 
	            unsigned num_of_samples )
{
  unsigned i;
	
	double gain_change_per_sample;

  double gain_factor; 
	char up = 0;

	if ( curVGA_gain > VGA_gain )
	{
		gain_change_per_sample = 1.0 / VGAGainChangePerSample;
		up = -1;
	}
	else
	if ( curVGA_gain < VGA_gain )
	{
		gain_change_per_sample = VGAGainChangePerSample;
		up = 1;
	}

//	curVGA_gain = VGA_gain;

  for ( i = 0; i < num_of_samples; i++ )
  {
		gain_factor = exp( log(10.0) * 0.9375 * curVGA_gain / 10.0 );

    time_signal[i].dat[0] *= gain_factor;
    time_signal[i].dat[1] *= gain_factor;

		if ( curVGA_gain != VGA_gain )
		{
			curVGA_gain *= gain_change_per_sample;
		}

		if ( ( up == -1 ) && ( curVGA_gain <= VGA_gain ) )
		{
			curVGA_gain = VGA_gain;
		}

		if ( ( up == 1 ) && ( curVGA_gain >= VGA_gain ) )
		{
			curVGA_gain = VGA_gain;
		}
	}
}