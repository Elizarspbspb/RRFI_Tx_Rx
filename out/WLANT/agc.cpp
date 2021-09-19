/* File History
 * $History: agc.c $
 * 
 * *****************  Version 16  *****************
 * User: Akozlov      Date: 26.07.05   Time: 14:29
 * Updated in $/WLAN
 * 32 samples RMS in every state
 * 
 * *****************  Version 15  *****************
 * User: Akozlov      Date: 24.12.04   Time: 19:41
 * Updated in $/WLAN
 * 
 * *****************  Version 14  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:45
 * Updated in $/WLAN
 * Some functions for AGC states added
 * 
 * *****************  Version 13  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:50
 * Updated in $/WLAN
 * GainUpdate RMS calculation changed to 32 samples of 40MHz signal
 * 
 * *****************  Version 12  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:07
 * Updated in $/WLAN
 * Ideal packet detection now, due to decoders precisions optimization
 * 
 * *****************  Version 11  *****************
 * User: Akozlov      Date: 2.12.04    Time: 12:23
 * Updated in $/WLAN
 * AGC always check 4 samples, Packet detection works only then AGC locked
 * 
 * *****************  Version 10  *****************
 * User: Akozlov      Date: 1.12.04    Time: 18:13
 * Updated in $/WLAN
 * 
 * *****************  Version 9  *****************
 * User: Akozlov      Date: 1.12.04    Time: 14:51
 * Updated in $/WLAN
 * AGC events storing in AGC_Events array
 */


#include "agc.h"
#include "sim_consts.h"
#include "sim_params.h"
#include <math.h>
#include "model.h"
#include "VGA.h"
#include "ADC.h"
#include <memory.h>
#include "find_packet_edge.h"

static int AGC_thresholds[6];

static int AGCAddrModLUT[128]; //K.Zakharchenko


static AGCEvents AGC_Events[200];
static int num_of_AGC_events;

double round(double a)
{
	if ( fabs(a-ceil(a))<fabs(a-floor(a)) )
		return (int)ceil(a);
	else
		return (int)floor(a);
}

////////////////////////////////////////////////////////////////////////////////
// Name: AGC_11a
//
// Purpose: Automatic gain control
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input, Fixed-point time domain signal
//
//             gain - input/output, GainAddr (0..127)
//              
// Returning value: Number of clippings
////////////////////////////////////////////////////////////////////////////////
/*int AGC_11a( fxp_complex* time_signal, int* gain, int AGC_status )
{
  unsigned i;
  unsigned num_of_clippings = 0;
  int RMS;

  char overflow_flag;

	int new_AGC_status;

	int Vrms_min;

	int Vrms_max;

	if ( AGC_status != AGCCheckRMS )
	{
		Vrms_min = convert_flp_to_fxp( 0.77 * 0.1768,
																	 fxp_params.AGC_precision,
																	 fxp_params.ADC_DAC_precision - 1,
																	 &overflow_flag,
																	 overflow_comment );

		Vrms_max = convert_flp_to_fxp( 1.22 * 0.1768,
																	 fxp_params.AGC_precision,
																	 fxp_params.ADC_DAC_precision - 1,
																	 &overflow_flag,
																	 overflow_comment );
	}
	else
	{
		Vrms_min = convert_flp_to_fxp( 0.9 * 0.77 * 0.1768,
																	 fxp_params.AGC_precision,
																	 fxp_params.ADC_DAC_precision - 1,
																	 &overflow_flag,
																	 overflow_comment );

		Vrms_max = convert_flp_to_fxp( 1.1 * 1.22 * 0.1768,
																	 fxp_params.AGC_precision,
																	 fxp_params.ADC_DAC_precision - 1,
																	 &overflow_flag,
																	 overflow_comment );
	}

  


  //[ Calculate number of clippings
  for ( i = 0; i < 4; i++ )
	{
		if ( ( time_signal[i].re ==
						( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) - 1 ) ||
				 ( time_signal[i].re == 
					 -( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) ) ) 
		{
			num_of_clippings ++;
		}

		if ( ( time_signal[i].im ==
						( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) - 1 ) ||
				 ( time_signal[i].im == 
					 -( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) ) ) 
		{
			num_of_clippings ++;
		}
	}
	//]

	#ifdef AGC_TEST
	printf( "num_of_clippings=%d\n", num_of_clippings );
	#endif

	//[ Change gain 
  switch( num_of_clippings )
  {
    case 0: break;
    case 1: *gain -= 10; break;
    case 2: *gain -= 10; break;
    case 3: *gain -= 13; break;
    case 4: *gain -= 13; break;
    case 5: *gain -= 16; break;
    case 6: *gain -= 16; break;
    case 7: *gain -= 19; break;
    case 8: *gain -= 19; break;

    default: *gain -= 19; break;
  }
	//]

	if ( num_of_clippings )
	{
		new_AGC_status = AGCClippings;
	}
	else
	{
		if ( AGC_status == AGCLocked )
		{
			new_AGC_status = AGCLocked;
		}
		else
		if ( AGC_status == AGCWait4SamplesAfter16 )
		{
			new_AGC_status = AGCWait64Samples;
		}
		else
		{
			new_AGC_status = AGCWait16Samples;
		}
	}
	
	if ( new_AGC_status != AGCClippings )
	{
		if ( AGC_status == AGCWait16Samples )
		{
			RMS = calculateRMS( time_signal, 16 );

			gain_change_via_table( gain, RMS );

			new_AGC_status = AGCWaitAfter16Samples;
		}

		if ( AGC_status == AGCWait64Samples )
		{
			RMS = calculateRMS( time_signal, 64 );

			if ( ( Vrms_min < RMS ) && ( RMS < Vrms_max ) )
			{
				new_AGC_status = AGCLocked;
			}
			else
			{
				gain_change_via_table( gain, RMS );

				new_AGC_status = AGCWait64Samples;
			}
		}

		if ( AGC_status == AGCCheckRMS )
		{
			RMS = calculateRMS( time_signal, 64 );

			if ( ( Vrms_min < RMS ) && ( RMS < Vrms_max ) )
			{
				new_AGC_status = AGCLocked;
			}
			else
			{
				gain_change_via_table( gain, RMS );

				new_AGC_status = AGCWait64Samples;
			}
		}
	}

  if ( *gain < 0 )
  {
    *gain = 0;
  }

  if ( *gain > 127 )
  {
    *gain = 127;
  }
	
  return new_AGC_status;
}

void init_AGC_table() 
{
  int i;
  int RMS;
  char overflow_flag;

  RMS = convert_flp_to_fxp( 0.1768,
                            fxp_params.AGC_precision,
                            fxp_params.ADC_DAC_precision - 1,
                            &overflow_flag,
                            overflow_comment );

  for ( i = -3; i < 3; i++ )
  {
    AGC_thresholds[3+i] = (int)( RMS * (1 + i * 0.22) );
  }
}*/

////////////////////////////////////////////////////////////////////////////////
// Name: init_AGC_table
//
// Purpose: Init AGC LUT table
//
// Author: K.Zakharchenko, modified by Alexandr Kozlov 
//
// Parameters: None
//              
// Returning value: None
////////////////////////////////////////////////////////////////////////////////
void init_AGCAddrModLUT()
{
	int i;
	double required_gain_addr_change;

	
	for(i=1;i<128;i++)
	{
	  required_gain_addr_change=10*log10(23.0/(double)i/0.9375);
		AGCAddrModLUT[i]=round(required_gain_addr_change);
	}
	AGCAddrModLUT[0]=15;
	AGCAddrModLUT[26]=-1; // modification
	AGCAddrModLUT[27]=-1;
}

////////////////////////////////////////////////////////////////////////////////
// Name: calculateRMS
//
// Purpose: Calculate root-mean-square
//
// Author: Alexandr Kozlov 
//
// Parameters: time_signal - input, time signal
//
//             num_of_samples - input, number of samples
//              
// Returning value: RMS
////////////////////////////////////////////////////////////////////////////////
int calculateRMS( fxp_complex* time_signal, unsigned num_of_samples )
{
	unsigned i;
	int RMS;
	char overflow_flag;

  //[ Calculate root-mean-square
  RMS = 0;

  for ( i = 0; i < num_of_samples; i++ )
  {
    RMS = fxp_add( RMS,
									 fxp_complex_abs2_beta( time_signal[i],
																					fxp_params.AGC_precision,
																					&overflow_flag,
																					overflow_comment ),
 									 fxp_params.AGC_precision + 1, // due to RMS is unsigned we can use sign bit
									 &overflow_flag,
									 overflow_comment );
  }

	if ( num_of_samples == 16 )
	{
		RMS >>= 5;
	}
	else
	if ( num_of_samples == 32 )
	{
		RMS >>= 6;
	}
  else
	if ( num_of_samples == 64 )
	{
		RMS >>= 7;
	}

	RMS = fxp_mul( RMS,
 								 convert_flp_to_fxp( 1.2533,
								 										 fxp_params.AGC_precision,
																		 fxp_params.ADC_DAC_precision - 1,
																		 &overflow_flag,
																		 overflow_comment ),
								 fxp_params.AGC_precision,
								 fxp_params.ADC_DAC_precision - 1,
								 &overflow_flag,
								 overflow_comment );
  //]

	#ifdef AGC_TEST
  printf( "RMS=%d\n", RMS );
	#endif

	return RMS;
}

void gain_change_via_table( int* gain, int RMS )
{
	int RMS_MAX_int;
	char overflow_flag;
	int VrmsAddr;

	RMS_MAX_int=convert_flp_to_fxp( 1,
														fxp_params.AGC_precision,
														fxp_params.ADC_DAC_precision - 1,
//							fxp_params.AGC_precision-1,
														&overflow_flag,
														overflow_comment )-1;

//	VrmsAddr=__fmax((int)0, round(RMS*128.0/RMS_MAX_int)-1 ); // Compute VrmsAddr accordung to RMS level
//	VrmsAddr=__fmin(VrmsAddr, 127);
	VrmsAddr=fmax((int)0, round(RMS*128.0/RMS_MAX_int)-1 ); // Compute VrmsAddr accordung to RMS level
	VrmsAddr=fmin(VrmsAddr, 127); // стало
	
//	*gain=__fmax(0, __fmin(*gain+AGCAddrModLUT[VrmsAddr],127)); //Compute changes in gain of AGC
	*gain=fmax(0, fmin(*gain+AGCAddrModLUT[VrmsAddr],127)); //Compute changes in gain of AGC // стало
}

////////////////////////////////////////////////////////////////////////////////
// Name: check4Samples
//
// Purpose: Check 4 samples for clipping
//
// Author: Alexandr Kozlov 
//
// Parameters: time_signal - input, time signal
//
// Returning value: Number of clipping
////////////////////////////////////////////////////////////////////////////////
unsigned check4Samples( gsl_complex* time_signal )
{
	gsl_complex temp_samples[4];
	fxp_complex fxp_samples[4];
	unsigned num_of_clippings = 0;
	unsigned i;

	memcpy( temp_samples, time_signal, 4 * sizeof( gsl_complex ) );
	
	VGA_11a( temp_samples, 4 );
	ADC_11a( temp_samples, fxp_samples, 4 );

  //[ Calculate number of clippings
  for ( i = 0; i < 4; i++ )
	{
		if ( ( fxp_samples[i].re ==
						( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) - 1 ) ||
				 ( fxp_samples[i].re == 
					 -( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) ) ) 
		{
			num_of_clippings ++;
		}

		if ( ( fxp_samples[i].im ==
						( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) - 1 ) ||
				 ( fxp_samples[i].im == 
					 -( 1 << ( fxp_params.ADC_DAC_precision - 1 ) ) ) ) 
		{
			num_of_clippings ++;
		}
	}
	//]

	return num_of_clippings;
}

////////////////////////////////////////////////////////////////////////////////
// Name: getAGCStatus_in_sample
//
// Purpose: Return AGC status at sample
//
// Author: Alexandr Kozlov 
//
// Parameters: sample_number - input, sample number
//
// Returning value: AGC status
////////////////////////////////////////////////////////////////////////////////
unsigned getAGCStatus_in_sample( unsigned sample_number )
{
  unsigned i = 0;

  while ( i < num_of_AGC_events )
  {
    if ( AGC_Events[i].sample >= sample_number )
    {
      break;
    }

    i++;
  }

  if ( i == num_of_AGC_events )
  {
    return AGC_Events[num_of_AGC_events-1].AGCStatus;
  }
  else
  {
    return AGC_Events[ i == 0 ? i : (i - 1) ].AGCStatus;
  }
}

unsigned getPrevAGCStatus_in_sample( unsigned sample_number )
{
  unsigned i = 0;

  while ( i < num_of_AGC_events )
  {
    if ( AGC_Events[i].sample >= sample_number )
    {
      break;
    }

    i++;
  }

  if ( i < 2 )
  {
    return 5;
  }
  else if ( i == num_of_AGC_events )
  {
    return AGC_Events[num_of_AGC_events-1].AGCStatus;
  }
  else
  {
    return AGC_Events[ i == 0 ? i : (i - 2) ].AGCStatus;
  }
}

int isAGCLocked_in_interval( unsigned sample_left, unsigned sample_right )
{
  unsigned i;

  for ( i = sample_left; i <= sample_right; i++ )
  {
    if ( getAGCStatus_in_sample( i ) != AGCLocked )
    {
      return 0;
    }
  }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Name: AGCPacketDetectionBlock
//
// Purpose: AGC and packet detection 
//
// Author: Alexandr Kozlov 
//
// Parameters:  time_signal - input, 80 MHz time domain signal
//
//	            fxp_signal - input, fixed-point signal buffer
//
//              num_of_samples - input, number of samples
//
//              packet_start_sample - output, packet start sample
//
//              AGCFreeze_sample - output, AGC Freeze sample
//
//						  VGA_sample - output, current VGA sample number
//
// Returning value: None
////////////////////////////////////////////////////////////////////////////////
void AGCPacketDetectionBlock( gsl_complex* time_signal, // 80 MHz signal
														  fxp_complex* fxp_signal, // need for temp
															unsigned num_of_samples, 
															unsigned* packet_start_sample,
															unsigned* AGCFreeze_sample,
															unsigned* VGA_sample )
{
	unsigned i,j;

	unsigned cur_sample = 0;
	unsigned cur_detect_sample = 0;
	unsigned num_of_clippings = 0;
	fxp_complex fxp_signal_for_detection[32];
	int RMS;
	int AGCStatus = AGCStateUnLock;

	char overflow_flag;

	char AGCUnlock = 1;

	char edge_found;
	unsigned num_of_edges;
	unsigned locked_sample = 0;

	int Vrms_min;

	int Vrms_max;

  int state_now;
  int state_prev;


  num_of_AGC_events = 0;

	while ( ( cur_sample < num_of_samples - 64 ) && 
					( AGCStatus != AGCFreeze ) )
	{
		num_of_clippings = check4Samples( time_signal + cur_sample );


		if ( num_of_clippings )
		{
				VGA_11a( time_signal + cur_sample, 4 ); 
				ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, 4 );
				cur_sample += 4; // 4 samples for check

				//[ Change gain 
				switch( num_of_clippings )
				{
					case 0: break;
					case 1: VGA_gain -= 10; break;
					case 2: VGA_gain -= 10; break;
					case 3: VGA_gain -= 13; break;
					case 4: VGA_gain -= 13; break;
					case 5: VGA_gain -= 16; break;
					case 6: VGA_gain -= 16; break;
					case 7: VGA_gain -= 19; break;
					case 8: VGA_gain -= 19; break;

					default: VGA_gain -= 19; break;
				}
			  //]

				VGA_11a( time_signal + cur_sample, 4 ); 
				ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, 4 );
				cur_sample += 4; // 4 waiting samples

        AGC_Events[num_of_AGC_events].sample = cur_sample - 8;
        AGC_Events[num_of_AGC_events].AGCStatus = AGCStateUnLock;
        AGC_Events[num_of_AGC_events].gain = curVGA_gain;
        AGC_Events[num_of_AGC_events].RMS = -1;
        num_of_AGC_events++;

				AGCStatus = AGCStateUnLock;
		}
		else
		{
			if ( AGCStatus == AGCStateUnLock )
			{
				VGA_11a( time_signal + cur_sample, 32 );
				ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, 32 );

				RMS = calculateRMS( fxp_signal + cur_sample, 32 );

				cur_sample += 32;
				gain_change_via_table( &VGA_gain, RMS );

				VGA_11a( time_signal + cur_sample, 4 ); // waiting while VGA gain changing
				ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, 4 );
				cur_sample += 4; // 4 waiting samples

				AGCStatus = AGCGainUpdate;

        AGC_Events[num_of_AGC_events].sample = cur_sample;
        AGC_Events[num_of_AGC_events].AGCStatus = AGCGainUpdate;
        AGC_Events[num_of_AGC_events].gain = curVGA_gain;
        AGC_Events[num_of_AGC_events].RMS = RMS;
        num_of_AGC_events++;
			}
			else
			if ( AGCStatus == AGCGainUpdate )
			{
        for ( i = 0; i < 8; i++ )
        {
          if ( check4Samples( time_signal + cur_sample + i * 4 ) )
          {
            AGCStatus = AGCStateUnLock;
            VGA_11a( time_signal + cur_sample, i * 4 );
				    ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, i * 4 );
            cur_sample += i * 4;
            break;
          }
        }

        if ( i == 8 )
        {
          VGA_11a( time_signal + cur_sample, 32 );
				  ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, 32 );

				  RMS = calculateRMS( fxp_signal + cur_sample, 32 );

				  Vrms_min = convert_flp_to_fxp( 0.77 * 0.1768,
																			   fxp_params.AGC_precision,
																			   fxp_params.ADC_DAC_precision - 1,
																			   &overflow_flag,
																			   overflow_comment );

				  Vrms_max = convert_flp_to_fxp( 1.22 * 0.1768,
																			   fxp_params.AGC_precision,
																			   fxp_params.ADC_DAC_precision - 1,
																			   &overflow_flag,
																			   overflow_comment );

				  if ( ( Vrms_min < RMS ) && ( RMS < Vrms_max ) )
				  {
					  AGCStatus = AGCLocked;
					  locked_sample = cur_sample;
					  //printf( "AGCLocked=%d\n", cur_sample );
            AGC_Events[num_of_AGC_events].sample = cur_sample + 32;
            AGC_Events[num_of_AGC_events].AGCStatus = AGCLocked;
            AGC_Events[num_of_AGC_events].gain = curVGA_gain;
            AGC_Events[num_of_AGC_events].RMS = RMS;
            num_of_AGC_events++;
				  }
				  else
				  {
					  gain_change_via_table( &VGA_gain, RMS );

					  AGCStatus = AGCGainUpdate;
				  }	

				  cur_sample += 32;
        }
			}
			else
			if ( AGCStatus == AGCLocked )
			{
        for ( i = 0; i < 8; i++ )
        {
          if ( check4Samples( time_signal + cur_sample + i * 4 ) )
          {
            AGCStatus = AGCStateUnLock;
            VGA_11a( time_signal + cur_sample, i * 4 );
				    ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, i * 4 );
            cur_sample += i * 4;
            break;
          }
        }

        if ( i == 8 )
        {
				  if ( AGCUnlock )
				  {
					  VGA_11a( time_signal + cur_sample, 32 );
					  ADC_11a( time_signal + cur_sample, fxp_signal + cur_sample, 32 );

					  RMS = calculateRMS( fxp_signal + cur_sample, 32 );

					  Vrms_min = convert_flp_to_fxp( 0.9 * 0.77 * 0.1768,
																				   fxp_params.AGC_precision,
																				   fxp_params.ADC_DAC_precision - 1,
																				   &overflow_flag,
																				   overflow_comment );

					  Vrms_max = convert_flp_to_fxp( 1.1 * 1.22 * 0.1768,
																				   fxp_params.AGC_precision,
																				   fxp_params.ADC_DAC_precision - 1,
																				   &overflow_flag,
																				   overflow_comment );

					  if ( ( Vrms_min < RMS ) && ( RMS < Vrms_max ) )
					  {
						  AGCStatus = AGCLocked;
					  }
					  else
					  {
						  gain_change_via_table( &VGA_gain, RMS );

						  AGCStatus = AGCGainUpdate;

              AGC_Events[num_of_AGC_events].sample = cur_sample + 32;
              AGC_Events[num_of_AGC_events].AGCStatus = AGCGainUpdate;
              AGC_Events[num_of_AGC_events].gain = curVGA_gain;
              AGC_Events[num_of_AGC_events].RMS = RMS;
              num_of_AGC_events++;
    			  }	

					  cur_sample += 32;
				  }
				  else
				  {
            for ( i = 0; i < num_of_AGC_events; i++ )
            {
              if ( ( AGC_Events[i].sample >= *packet_start_sample ) && 
                   ( AGC_Events[i].AGCStatus == AGCLocked ) )
              {
					      *AGCFreeze_sample = AGC_Events[i].sample;
					      AGCStatus = AGCFreeze;
                break;
              }
            }
				  }
        }
			}
  	}
	

		while ( ( cur_sample - cur_detect_sample > 96 ) && ( AGCUnlock ) )
		{
			num_of_edges = 0;

			#ifdef AQ_LOG
		/*		aq_log=fopen("aq_log.txt","a");
				fprintf(aq_log,"cur_detect_sample=%d\n", cur_detect_sample );
				fclose(aq_log);*/
			#endif

			for ( i = 0; i < 16; i++ )
			{
				for ( j = 0; j < 32; j ++ )
				{
					fxp_signal_for_detection[j] =  *(fxp_signal + cur_detect_sample + 2 * j + i);
				}

        state_now = getAGCStatus_in_sample( cur_detect_sample + 96 );
        state_prev = getAGCStatus_in_sample( cur_detect_sample );

        

        // Uncomment this for ideal packet detection
        if ( cur_detect_sample > 800 )
        {
          num_of_edges = 10;
        }
        else
        {
          edge_found = 0;
        }
        
        
           //( ( ( state_now == AGCGainUpdate ) && ( state_prev == AGCGainUpdate ) ) ||
           //( ( ( state_now == AGCLocked ) && ( state_prev == AGCGainUpdate ) ) ||
        /*if ( isAGCLocked_in_interval( cur_detect_sample, cur_detect_sample + 95 ) ) 
        {
          edge_found = find_packet_edge_11a( fxp_signal_for_detection );
        }
        else
        {
          edge_found = 0;
        }*/

				if ( edge_found ) 
				{
					num_of_edges ++;
				}
									
				if ( num_of_edges >= 10 ) 
				{
					AGCUnlock = 0;

					if ( getAGCStatus_in_sample( cur_detect_sample ) == AGCLocked )
					{
						*AGCFreeze_sample = cur_detect_sample;
						AGCStatus = AGCFreeze;
					}

					*packet_start_sample = cur_detect_sample;
					break;
				}
			}
      
      cur_detect_sample ++;
		}
	}

	*VGA_sample = cur_sample;


  if ( *AGCFreeze_sample < *packet_start_sample )
  {
    *AGCFreeze_sample = *packet_start_sample;
  }

#ifdef AQ_LOG
  aq_log=fopen("aq_log.txt","w");
	for ( i = 0; i < num_of_AGC_events; i++ )
  {
    fprintf( aq_log, "%d\t%d\t%d\t%d\n", AGC_Events[i].AGCStatus,
      AGC_Events[i].sample, AGC_Events[i].gain, AGC_Events[i].RMS );
  }
  fprintf(aq_log, "packet_start=%d\n", *packet_start_sample);
  fprintf(aq_log,"********\n");
  fclose(aq_log);
#endif
}