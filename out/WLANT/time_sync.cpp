/* File History
 * $History: time_sync.c $
 * 
 * *****************  Version 20  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:39
 * Updated in $/WLAN
 * Autocorrelation + cross-correlation algorithm
 * 
 * *****************  Version 19  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:45
 * Updated in $/WLAN
 * Autocorrelation calculation added
 * 
 * *****************  Version 18  *****************
 * User: Akozlov      Date: 2.12.04    Time: 12:21
 * Updated in $/WLAN
 * 
 * *****************  Version 17  *****************
 * User: Akozlov      Date: 1.12.04    Time: 14:51
 * Updated in $/WLAN
 * Timing matched filter size changed to 32
 */


#include "time_sync.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "fft.h"
#include "model.h"
#include <memory.h>

//#define SyncLTS


static fxp_complex timing_matched_filter[32];

#ifdef SyncLTS

#define StartSearch 0
#define EndSearch 300
#define RxTimingOffset (-3)

////////////////////////////////////////////////////////////////////////////////
// Name: fine_timing_sync_11a
//
// Purpose: Timing synchronization
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input, fixed-point time signal
//
// Returning value: Sample number 
////////////////////////////////////////////////////////////////////////////////
unsigned fine_timing_sync_11a( fxp_complex* time_signal )
{
	int i,j;
    
	fxp_complex long_training_seq[NumOfSubcarriers];
	fxp_complex long_training[NumOfSubcarriers];
	fxp_complex* dest;
	fxp_complex time_corr[EndSearch - StartSearch];
	int abs_corr[EndSearch - StartSearch];
	int abs_sum[EndSearch - StartSearch - NumOfSubcarriers];

	int re_signal[EndSearch-StartSearch+NumOfSubcarriers];
	
	char overflow_flag;
	int max_corr;
	int max_index;
	int cur_index;
	fxp_complex time_value;
	
	//fxp_complex time_value;
	
#ifdef LOGGING
	logging( "fine_timing_sync_11a...\n", 1 );
#endif
	
	//[ Generate long training sequence in time domain
	memset( long_training_seq, 0, NumOfSubcarriers * sizeof( fxp_complex ) );
	
	for ( i = 0; i < NumOfUsedSubcarriers; i++ )
	{
		long_training_seq[sim_consts.UsedSubcIdx[i] - 1].re =
			find_closest_fxp_value( sim_consts.LongTrainingSymbols[i],
			fxp_params.QAM_precision,
			2.16 / ( 1 << fxp_params.QAM_precision ) );
		
		long_training_seq[sim_consts.UsedSubcIdx[i] - 1].im = 0;
	}
	
	fft_with_mapping_11a( long_training_seq, 6, 1,
		fxp_params.FFT_precision,
		fxp_params.FFT_exp_position,
		fxp_params.IFFT_shift_control );
	//]
	
	//[ Add cyclic prefix
	dest = long_training;
	for ( i = - 2 * CyclicPrefixLength;
				i < NumOfSubcarriers - 2 * CyclicPrefixLength; i++ )
				{
					cur_index  = (i + NumOfSubcarriers) % (NumOfSubcarriers);
					
#ifdef LOGGING
					sprintf( overflow_comment, 
						"Overflow: *dest = fxp_complex_conjugate( long_training_seq[%d]\n", cur_index  );
#endif
					
					*dest = fxp_complex_conjugate( long_training_seq[cur_index], 
						fxp_params.FFT_precision,
						&overflow_flag, 
						overflow_comment );
					
					
					
#if 0								
						if ( dest->re == 0 )
						{
						dest->re = 0;
						}
						else
						if ( dest->re > 0 )
						{
						dest->re = 1;
						}
						else
						if ( dest->re < 0 )
						{
						dest->re = -1;
						}
						
						  if ( dest->im == 0 )
						  {
						  dest->im = 0;
						  }
						  else
						  if ( dest->im > 0 )
						  {
						  dest->im = 1;
						  }
						  else
						  if ( dest->im < 0 )
						  {
						  dest->im = -1;
						  }
#else
						  dest->re=convert_precision(dest->re,fxp_params.FFT_precision,
							  fxp_params.FFT_exp_position,3,8,&overflow_flag,overflow_comment);
						dest->im=convert_precision(dest->im,fxp_params.FFT_precision,
							  fxp_params.FFT_exp_position,3,8,&overflow_flag,overflow_comment);

	

#endif
						  
							dest++;
							}
							
					
					//]
					
					//[ Calculate correlation
					for ( i = StartSearch; i < EndSearch; i++ ) 
					{
						time_corr[i-StartSearch].re = 0;
						time_corr[i-StartSearch].im = 0;
						
						for ( j = 0; j < NumOfSubcarriers; j++ )
						{
#ifdef LOGGING
							sprintf( overflow_comment, 
								"Overflow: time_corr[%d] = fxp_complex_add( time_corr[%d]\n", i );
#endif
							

#if 0							
							
							if ( time_signal[i+j].re == 0 )
							{
							time_value.re = 0;
							}
							else
							if ( time_signal[i+j].re > 0 )
							{
							time_value.re = 1;
							}
							else
							if ( time_signal[i+j].re < 0 )
							{
							time_value.re = -1;
							}
							
							  if ( time_signal[i+j].im == 0 )
							  {
							  time_value.im = 0;
							  }
							  else
							  if ( time_signal[i+j].im > 0 )
							  {
							  time_value.im = 1;
							  }
							  else
							  if ( time_signal[i+j].im < 0 )
							  {
							  time_value.im = -1;
							  }
							  
#else							
							time_value.re = convert_precision(time_signal[i+j].re,
								fxp_params.FFT_precision,
								fxp_params.FFT_exp_position,3,8,&overflow_flag,overflow_comment);
							time_value.im = convert_precision(time_signal[i+j].im,
								fxp_params.FFT_precision,
								fxp_params.FFT_exp_position,3,8,&overflow_flag,overflow_comment);
							
							re_signal[i-StartSearch+j]=time_value.re;

#endif							
							
							time_corr[i-StartSearch] = 
								fxp_complex_add( time_corr[i-StartSearch], 
								fxp_complex_mul( time_value,
								long_training[j],
								fxp_params.Time_Sync_precision,
								0, // due to only +/- 1 is used
								&overflow_flag,
								overflow_comment ),
								fxp_params.Time_Sync_precision,
								&overflow_flag,
								overflow_comment                                       
								);
						}
					}
					//]
					
					//[ Calculate absolute correlation value
					for ( i = 0; i < EndSearch - StartSearch; i++ )
					{
#ifdef LOGGING
						sprintf( overflow_comment, 
							"Overflow: abs_corr[i] = fxp_complex_abs2( time_corr[%d]\n", i );
#endif
						
						abs_corr[i] = fxp_complex_abs2( time_corr[i],
							fxp_params.Time_Sync_precision,
							fxp_params.Time_Sync_exp_position,
							&overflow_flag,
							overflow_comment );
					}
					//]
					
					//[ Find correlation peak
					for ( i = 0; i < EndSearch - StartSearch - NumOfSubcarriers; i++ )
					{
#ifdef LOGGING
						sprintf( overflow_comment, 
							"Overflow: abs_sum[%d] = fxp_add( abs_corr[i]\n", i );
#endif
						
						abs_sum[i] = fxp_add( abs_corr[i] >> 1, // shift to avoid overflow
							abs_corr[i+NumOfSubcarriers] >> 1, // shift to avoid overflow
							fxp_params.Time_Sync_precision,
							&overflow_flag,
							overflow_comment );
					}
					max_corr = 0;
					for ( i = 0; i < EndSearch - StartSearch - NumOfSubcarriers; i++ )
					{
#ifdef LOGGING
						sprintf( overflow_comment, 
							"Overflow: abs_corr = fxp_complex_abs2( time_corr[%d]\n", i );
#endif
						
						if ( abs_sum[i] > max_corr )
						{
							max_corr = abs_sum[i];
							max_index = i;
						}
					}
					//]
					
#ifdef LOGGING
					logging( "fine_timing_sync_11a finished\n", -1 );
#endif
					
			return StartSearch + max_index +
						2 * CyclicPrefixLength + RxTimingOffset ;
}

#else

#define StartSearch 0
#define EndSearch 160
#define RxTimingOffset (-3)

////////////////////////////////////////////////////////////////////////////////
// Name: fine_timing_sync_11a
//
// Purpose: Timing synchronization
//
// Author: Alexandr Kozlov
//
// Parameters: time_signal - input, fixed-point time signal
//
// Returning value: Sample number 
////////////////////////////////////////////////////////////////////////////////
unsigned fine_timing_sync_11a( fxp_complex* time_signal )
{
  int i,j;
    
  fxp_complex time_corr[EndSearch - StartSearch];
  int abs_corr[EndSearch - StartSearch];

	fxp_complex time_value;
  fxp_complex delay_xcorr[EndSearch - StartSearch];
  int abs2_delay_xcorr[EndSearch - StartSearch];

	int max;
	int max_index;
	int last_peak;
  int max_delayed_corr;
  int av_delayed_corr;


  char overflow_flag;

  #ifdef LOGGING
  logging( "fine_timing_sync_11a...\n", 1 );
  #endif

	for( i = 0; i < EndSearch - StartSearch; i++ )
	{
    #ifdef LOGGING
 	  sprintf( overflow_comment, 
    "Overflow: delay_xcorr =, i = %d\n", i );
    #endif    

    delay_xcorr[i].re = 0;
    delay_xcorr[i].im = 0;
 
    for ( j = 0; j < Delay; j++ )
    {    
      delay_xcorr[i] =
        fxp_complex_add( delay_xcorr[i],
          fxp_complex_mul( time_signal[i+j],
                           fxp_complex_conjugate(
                             time_signal[i+j+Delay],
                             fxp_params.AGC_precision,  
                             &overflow_flag,
                             overflow_comment ),
                           fxp_params.AGC_precision,
                           fxp_params.ADC_DAC_precision - 1,
                           &overflow_flag,
                           overflow_comment ),
                         fxp_params.AGC_precision,
                         &overflow_flag,
                         overflow_comment );
    }

    abs2_delay_xcorr[i] = fxp_complex_abs2_beta( delay_xcorr[i], 
                                   fxp_params.AGC_precision, 
																	 //fxp_params.ADC_DAC_precision - 1,
                                   &overflow_flag,
                                   overflow_comment );
	}
  
  

	//[ Calculate correlation
  for ( i = StartSearch; i < EndSearch; i++ ) 
  {
     time_corr[i-StartSearch].re = 0;
     time_corr[i-StartSearch].im = 0;

     for ( j = 0; j < 2 * Delay; j++ )
     {
			 #ifdef LOGGING
			 sprintf( overflow_comment, 
			 "Overflow: time_corr[%d] = fxp_complex_add( time_corr[%d]\n", i );
			 #endif

       time_value.re = time_signal[i+j].re;
       time_value.im = time_signal[i+j].im;


       time_corr[i-StartSearch] = 
				 fxp_complex_add( time_corr[i-StartSearch], 
                          fxp_complex_mul( time_value,
                                           timing_matched_filter[j],
                                           fxp_params.Time_Sync_precision,
                                           fxp_params.Time_Sync_exp_position, // due to timing_matched_filter values +/- 1
                                           &overflow_flag,
                                           overflow_comment ),
                          fxp_params.Time_Sync_precision,
                          &overflow_flag,
                          overflow_comment
                        );
     }
  }
	//]

  //[ Calculate absolute correlation value
	for ( i = 0; i < EndSearch - StartSearch; i++ )
	{
	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: abs_corr[i] = fxp_complex_abs2( time_corr[%d]\n", i );
	  #endif

		abs_corr[i] = fxp_complex_abs2( time_corr[i],
 																		fxp_params.Time_Sync_precision,
																		fxp_params.Time_Sync_exp_position,
							  										&overflow_flag,
																		overflow_comment );
	}
  //]/

	//[ Find correlation peak
  max_delayed_corr = 0;
	max_index = -1;
	last_peak = -1;
	max = 0;
	for ( i = 0; i < EndSearch - StartSearch; i++ )
	{
	  #ifdef LOGGING
	  sprintf( overflow_comment, 
	  "Overflow: abs_sum[%d] = fxp_add( abs_corr[i]\n", i );
	  #endif

		if ( ( i - max_index ) % Delay == 0 )
		{
			if ( abs_corr[i] > fxp_mul( convert_flp_to_fxp( 0.6,
                                                      fxp_params.Time_Sync_precision,
                                                      fxp_params.Time_Sync_exp_position,
                                                      &overflow_flag,
                                                      overflow_comment ),
                                  max,
                                  fxp_params.Time_Sync_precision,
                                  fxp_params.Time_Sync_exp_position,
                                  &overflow_flag,
                                  overflow_comment ) )
			{
        av_delayed_corr = 0;
        for ( j = 0; j < Delay; j++ )
        {
          av_delayed_corr = fxp_add( av_delayed_corr,
                                     abs2_delay_xcorr[i-j],
                                     fxp_params.Time_Sync_precision,
                                     &overflow_flag,
                                     overflow_comment );
        }
        av_delayed_corr >>= 4;

        if ( abs2_delay_xcorr[i] > fxp_mul( convert_flp_to_fxp( 0.6,
                                                      fxp_params.Time_Sync_precision,
                                                      fxp_params.Time_Sync_exp_position,
                                                      &overflow_flag,
                                                      overflow_comment ),
                                            max_delayed_corr,
                                            fxp_params.Time_Sync_precision,
                                            fxp_params.Time_Sync_exp_position,
                                            &overflow_flag,
                                            overflow_comment ) )
        {
        	last_peak = i;
				  max_index = i;
          max = abs_corr[i];
          max_delayed_corr = av_delayed_corr;
        }
			}
		}

    if ( i <= Delay )
    {
	    if ( abs_corr[i] > max / 2 )
	    {
        last_peak = i;
		    max_index = i;
		    max = abs_corr[i];
        max_delayed_corr = 0;
        for ( j = 0; j < Delay; j++ )
        {
          max_delayed_corr = fxp_add( max_delayed_corr,
                                      abs2_delay_xcorr[i+j],
                                      fxp_params.Time_Sync_precision,
                                      &overflow_flag,
                                      overflow_comment );
        }
        max_delayed_corr >>= 4;
	    }
    }
	}
	//]
  
  #ifdef LOGGING
  logging( "fine_timing_sync_11a finished\n", -1 );
  #endif

  return last_peak + 2 * Delay + 2 * CyclicPrefixLength + RxTimingOffset;
}

#endif



void init_timing_matched_filter()
{
	unsigned i;
	fxp_complex short_training_seq[NumOfSubcarriers];
	char overflow_flag;

		//[ Generate short training sequence in time domain
  memset( short_training_seq, 0, NumOfSubcarriers * sizeof( fxp_complex ) );

  for ( i = 0; i < NumOfUsedSubcarriers; i++ )
  {
  	short_training_seq[sim_consts.UsedSubcIdx[i] - 1].re =
      find_closest_fxp_value( sim_consts.ShortTrainingSymbols[i].dat[0],
                              fxp_params.QAM_precision,
                              2.16 / ( 1 << fxp_params.QAM_precision ) );

  	short_training_seq[sim_consts.UsedSubcIdx[i] - 1].im =
      find_closest_fxp_value( sim_consts.ShortTrainingSymbols[i].dat[1],
                              fxp_params.QAM_precision,
                              2.16 / ( 1 << fxp_params.QAM_precision ) );
  }

  fft_with_mapping_11a( short_training_seq, 6, 1,
                        fxp_params.FFT_precision,
                        fxp_params.FFT_exp_position,
                        fxp_params.IFFT_shift_control );
	//]

  //[ Generate +/-1 sequence
  for ( i = 0; i < Delay; i++ )
	{
    #ifdef LOGGING
    sprintf( overflow_comment, 
		"Overflow: short_training[i] = fxp_complex_conjugate( short_training_seq[%d]\n", i  );
    #endif

		timing_matched_filter[i] = fxp_complex_conjugate( short_training_seq[i], 
																											fxp_params.FFT_precision,
																											&overflow_flag, 
																											overflow_comment );

		timing_matched_filter[i+Delay].re = timing_matched_filter[i].re;// = make2bit_value( timing_matched_filter[i].re );
		timing_matched_filter[i+Delay].im = timing_matched_filter[i].im;// = make2bit_value( timing_matched_filter[i].im );
	}
	//]
}