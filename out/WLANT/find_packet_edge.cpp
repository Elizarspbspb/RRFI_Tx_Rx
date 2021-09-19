/* File History
 * $History: find_packet_edge.c $
 * 
 * *****************  Version 6  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:51
 * Updated in $/WLAN
 * 
 * *****************  Version 5  *****************
 * User: Akozlov      Date: 2.12.04    Time: 12:24
 * Updated in $/WLAN
 * Algorithm changed
 * 
 */

#include "find_packet_edge.h"
#include "sim_consts.h"
#include "model.h"
#include "sim_params.h"
#include "assert.h"


////////////////////////////////////////////////////////////////////////////////
// Name: find_packet_edge_11a
//
// Purpose: Packet detection
//
// Author: Alexandr Kozlov, modified by K.Zakharchenko, 17.nov.2004
//
// Parameters: time_signal - input, Time domain signal 
//  
// Returning value: Sample number or 0 if not detected 
////////////////////////////////////////////////////////////////////////////////
int find_packet_edge_11a( fxp_complex* time_signal )
{
  unsigned i;
  fxp_complex delay_xcorr;
  fxp_complex RMS;

  int decision_var = 0;
  int det_fresh = 0;

  double xcorr_re;
  double xcorr_im;
  double abs2_xcorr;
  double fRMS;
  double DES;

  char overflow_flag;
	char overflow_warning = 0;
	unsigned num_of_clippings = 0;
  
  #ifdef LOGGING
  logging( "find_packet_edge_11a...\n", 1 );
  #endif

  assert(Delay==16);

  xcorr_re = 0;
  xcorr_im = 0;
  fRMS = 0;
  for ( i = 0; i < Delay; i++ )
  {
    xcorr_re += (double)time_signal[i].re *
                         (double) fxp_complex_conjugate(
                           time_signal[i+Delay],
                           fxp_params.AGC_precision,  
                           &overflow_flag,
                           overflow_comment ).re -
                (double)time_signal[i].im *
                         (double) fxp_complex_conjugate(
                           time_signal[i+Delay],
                           fxp_params.AGC_precision,  
                           &overflow_flag,
                           overflow_comment ).im;

    xcorr_im += (double)time_signal[i].re *
                          (double)fxp_complex_conjugate(
                           time_signal[i+Delay],
                           fxp_params.AGC_precision,  
                           &overflow_flag,
                           overflow_comment ).im +
                (double)time_signal[i].im *
                          (double)fxp_complex_conjugate(
                           time_signal[i+Delay],
                           fxp_params.AGC_precision,  
                           &overflow_flag,
                           overflow_comment ).re;

    fRMS += (double)time_signal[i].re * (double)time_signal[i].re + (double)time_signal[i].im * (double)time_signal[i].im;
  }

  abs2_xcorr = xcorr_re * xcorr_re + xcorr_im * xcorr_im;


  DES = abs2_xcorr / ( fRMS * fRMS );

  //[ Calculate the delayed correlation
  delay_xcorr.re = 0;
  delay_xcorr.im = 0;
  RMS.re=0;
  RMS.im=0;
/*
	for( i = 0; i < Delay; i++ )
	{
	  #ifdef LOGGING
 	  sprintf( overflow_comment, 
    "Overflow: delay_xcorr =, i = %d\n", i );
    #endif    

    delay_xcorr =
      fxp_complex_add( delay_xcorr,
        fxp_complex_mul( time_signal[i],
                         fxp_complex_conjugate(
                           time_signal[i+Delay],
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

		if ( overflow_flag )
		{
			overflow_warning = 1;
		}

    RMS = fxp_complex_add( RMS,
                           fxp_complex_mul( time_signal[i],
                                            fxp_complex_conjugate(
                                                time_signal[i],
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

		if ( overflow_flag )
		{
			overflow_warning = 1;
		}
	}
  //]
	#ifdef LOGGING
 	sprintf( overflow_comment, 
	"Overflow: decision_var = fxp_complex_abs2\n" );
	#endif    
/*
  decision_var = fxp_complex_abs2( delay_xcorr, 
                                   fxp_params.Packet_detection_precision, 
                                   fxp_params.Packet_detection_exp_position,
                                   &overflow_flag,
                                   overflow_comment );
*/	
// Modified by K.Zakharchenko according to yellow book
//[ Detection statistic beta  calculation
  /*decision_var = fxp_complex_abs2( delay_xcorr, 
                                   fxp_params.Packet_detection_precision, 
																	 fxp_params.ADC_DAC_precision - 1,
                                   &overflow_flag,
                                   overflow_comment );
	
	if ( overflow_flag )
	{
		overflow_warning = 1;
	}

  //if ( !overflow_warning )
    DES = (double)decision_var / (double)RMS.re;
 // else
   // DES = 0;

  decision_var=decision_var/Delay;
  RMS.re=RMS.re/Delay;

//]
//[ Detection threshold calculation, reference - yellow book
 /* det_fresh = fxp_mul( RMS.re,
                       convert_flp_to_fxp( 0.0807, 
                                           fxp_params.ADC_DAC_precision,
                                           fxp_params.ADC_DAC_precision - 1,
                                           &overflow_flag,
                                           overflow_comment );
                       fxp_params.AGC_precision,
                       fxp_params.ADC_DAC_precision - 1,
                       &overflow_flag,
                       overflow_comment );*/
                       
//]
//[Noise variance calcalation

//]
  #ifdef LOGGING
  logging( "find_packet_edge_11a finished\n", -1 );
  #endif
#ifdef AQ_LOG
  aq_log=fopen("aq_log.txt","a");
  fprintf(aq_log,"%d\t%d\t%d\t%d\t%f\n",decision_var, det_fresh, RMS.re / 2, (unsigned)overflow_warning, DES);
  fclose(aq_log);
#endif

	//printf( "decision_var=%d ", decision_var);




  //if ( (decision_var > det_fresh ) && (decision_var > RMS.re / 2 ) && (!overflow_warning) )//reference - yellow book
  //if ( (decision_var > det_fresh ) && (!overflow_warning) )

  if ( DES > 0.6 )
  {
    return 1;
  }
  else
  {
    return 0;
  }

//  return 1;
}
