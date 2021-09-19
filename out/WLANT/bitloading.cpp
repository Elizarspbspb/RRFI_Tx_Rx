/* File History
 * $History: bitloading.c $
 * 
 * *****************  Version 17  *****************
 * User: Akozlov      Date: 29.03.05   Time: 19:35
 * Updated in $/WLAN
 * ABL thresholds from config
 * 
 * *****************  Version 16  *****************
 * User: Akozlov      Date: 31.01.05   Time: 14:02
 * Updated in $/WLAN
 * New thresholds
 * 
 * *****************  Version 1  *****************
 * User: Akozlov      Date: 31.01.05   Time: 13:54
 * Created in $/WLAN/WLAN
 * Reed-Solomon lib 
 * 
 * *****************  Version 15  *****************
 * User: Akozlov      Date: 24.12.04   Time: 19:43
 * Updated in $/WLAN
 * Wraper_for_BLA changed
 * 
 * *****************  Version 14  *****************
 * User: Akozlov      Date: 15.12.04   Time: 11:39
 * Updated in $/WLAN
 * 
 * *****************  Version 13  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:46
 * Updated in $/WLAN
 * ABL Thresholds changed, ABL algorithms
 * 
 */

#include <math.h>
#include "bitloading.h"
#include "sim_consts.h"
#include "model.h"
#include "sim_params.h"

#include "model.h"
#include "commonfunc.h"
#define min(a,b) (((a) < (b)) ? (a):(b))

/////////////////////////////// New Bitloading implementation

// fixed point tables
int P_SIG_ERR_DIF_LOG[BL_MAX_CONSTELLATION+1];   // thresholds for constellation changing
int A_SQ_INV_LOG[(1<<BL_INV_LOG_DIGITS)];  // A_SQ_INV_LOG[i] = log(1/(i*i))/log(2)

////////////////////////////////////////////////////////////////////////////////
// Name: init_NEW_bitloading_constants
//
// Purpose: Initializes constants and tables for NEW bitloading 
//
// Author: Vladislav Chernyshev, Alexandr Kozlov
//
// Parameters: (none)
//  
// Returning value: 0 - ok, -1 - error
////////////////////////////////////////////////////////////////////////////////
int init_NEW_bitloading_constants(){
  double d,d1;
  int i;

  char overflow_flag;
  char overflow_comment[100]="";

  for(i=0;i<((1<<BL_INV_LOG_DIGITS));i++){
    if(i){
      d=convert_fxp_to_flp(i,BL_EXP_POSITION);
    } else {
      d=convert_fxp_to_flp(1,BL_EXP_POSITION);
    }

    d1=log(1.0/(d*d))/log(2.0);
    A_SQ_INV_LOG[i]=convert_flp_to_fxp(d1,BL_INV_LOG_DIGITS,BL_EXP_POSITION,&overflow_flag,overflow_comment);
    if(overflow_flag){
      printf("OF in initing A_SQ_INV_LOG\n");
    }
  }

/*
  // without log-metrics!!!!!!!
  // BitsPerSymbol==2
  P_SIG_ERR_DIF_LOG[0]=2;
  !!!
  P_SIG_ERR_DIF_LOG[1]=14;
  P_SIG_ERR_DIF_LOG[2]=551;
  P_SIG_ERR_DIF_LOG[3]=5112;
  P_SIG_ERR_DIF_LOG[4]=8096;
*/
/* // for 16.10 precision 
  P_SIG_ERR_DIF_LOG[0]=3500;  // 0->QPSK    (2  bits)
  !!!
  P_SIG_ERR_DIF_LOG[1]=7500;  // 2->16QAM   (4  bits)    // <--  vld: top for BitsPerSymbol=2
  P_SIG_ERR_DIF_LOG[2]=11600; // 4->64QAM   (6  bits)    // <--  vld: top for BitsPerSymbol=4
  P_SIG_ERR_DIF_LOG[3]=15725; // 6->256QAM  (8  bits)    // <--  vld: top for BitsPerSymbol=6
  P_SIG_ERR_DIF_LOG[4]=19630; // 8->1024QAM (10 bits) 
*/

 // for 10.4 precision 
  P_SIG_ERR_DIF_LOG[0]=3550;  // 0->QPSK    (2  bits) // BPSK not implemented yet
  P_SIG_ERR_DIF_LOG[1]=0;     

  P_SIG_ERR_DIF_LOG[2]=7300;  // 2->16QAM   (4  bits)    // <--  vld: top for BitsPerSymbol=2
  P_SIG_ERR_DIF_LOG[3]=11650; // 4->64QAM   (6  bits)    // <--  vld: top for BitsPerSymbol=4

//  P_SIG_ERR_DIF_LOG[3]=15725; // 6->256QAM  (8  bits)    // <--  vld: top for BitsPerSymbol=6
//  P_SIG_ERR_DIF_LOG[4]=19630; // 8->1024QAM (10 bits) 


  // table is defined for precision x.10, convert it to new precision if needed.
  for(i=0;i<BL_MAX_CONSTELLATION;i++){
    P_SIG_ERR_DIF_LOG[i]>>=(10-BL_EXP_POSITION); 
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Name: get_constellation_coeff
//
// Purpose: calculation of constellation changing utility coefficients (fixed point)
//
// Author: Vladislav Chernyshev, Alexandr Kozlov
//
// Parameters: constellationIDs - input, current constellation id
//
//             channel_sq_amplitude - input, Channel square amplitude 
//  
// Returning value: constellation changing utility coefficients (fixed point value)
////////////////////////////////////////////////////////////////////////////////
int get_constellation_coeff(int constellationIDs,int channel_sq_amplitude){
  char overflow_flag;
  char overflow_comment[100]="";

  int r;
  
  r=fxp_add(
    P_SIG_ERR_DIF_LOG[constellationIDs],
    A_SQ_INV_LOG[channel_sq_amplitude],
    BL_INV_LOG_DIGITS,&overflow_flag,overflow_comment
  );

  if(overflow_flag){
    printf("OF in get_constellation_coeff()\n");
  }

  return r;

}

////////////////////////////////////////////////////////////////////////////////
// Name: NEW_bitloading_FXP
//
// Purpose: Implements the NEW bitloading algorithm in Fixed point
//
// Author: Vladislav Chernyshev, Alexandr Kozlov
//
// Parameters: channel_sq_amplitudes - input, Channel square amplitudes 
//
//             num_of_signal_levels - (for future use) input, Number of signal levels
//                                           in each constellation 
//
//             required_bits - input, Total number of bits to be transmitted 
//
//             max_power - (for future use) input, Power budget
//
//             bitsPerSymbol - output, number of bits to be transmitted in each subchannel 
//
//             signal_gains - (for future use) output, Channel gains for each subchannel 
//
//             min_bitsPerSymbol - (for future use) input, Do not use constellations with 
//                                          too few bits (e.g. 0) 
//  
// Returning value: 0 - ok, -1 - error
////////////////////////////////////////////////////////////////////////////////
int NEW_bitloading_FXP
(
    const int*      fx_channel_sq_amplitudes, 
    unsigned        required_bits,         
    double          max_power,             // unused
    unsigned*       bitsPerSymbol,     
    int*         signal_gains,          // unused, set to 1;
    unsigned        min_bitsPerSymbol    // unused
)
{
  int P[NumOfDataSubcarriers] ;
  int i;
  int bits_left;

  int minP_subcarrier_index;
  int minP_subcarrier_value;

  for(i=0;i<NumOfDataSubcarriers;i++){
    signal_gains[i]=1;
    bitsPerSymbol[i]=0;

    P[i]=get_constellation_coeff(0,fx_channel_sq_amplitudes[i]);
  }

  for(bits_left=required_bits;bits_left>0;bits_left-=2){
    minP_subcarrier_index=-1;
    minP_subcarrier_value=(1<<BL_INV_LOG_DIGITS)-1;
    for(i=0;i<NumOfDataSubcarriers;i++){
      if(P[i]<minP_subcarrier_value && bitsPerSymbol[i]<BL_MAX_CONSTELLATION){
        minP_subcarrier_index=i;
        minP_subcarrier_value=P[i];
      }
    }

    i=minP_subcarrier_index;
    if(0==bitsPerSymbol[i]){ // to avoid BPSK (not implemented in ABL yet)
      bitsPerSymbol[i]+=2;
    } else {
      bitsPerSymbol[i]++;
    }

    P[i]=get_constellation_coeff(bitsPerSymbol[i],fx_channel_sq_amplitudes[i]);
  }

  for(i=0;i<NumOfDataSubcarriers;i++){
    if(bitsPerSymbol[i]>2){
      bitsPerSymbol[i]=(bitsPerSymbol[i]-1)*2;
    }
  }

  return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Name: BPLA_scheme_v3_1
//
// Purpose: 
//
// Author: 
//
// Parameters:
//
// Returning value: 
////////////////////////////////////////////////////////////////////////////////
static int compare_inv_sort_str( const void *arg1, const void *arg2 )
{
  sort_str* str1 = (sort_str*)arg1;
  sort_str* str2 = (sort_str*)arg2;

  if (str1->value > str2->value)
    return(-1);
  if (str1->value < str2->value)
    return(1);
  return(0);
}

static double low_thresholds_tbl_dB = 3.65;

static double thresholds_tbl_dB[4] = {
//  3.65, 6.65, 13.15, 18.9
//  2.15, 5.15, 11.65, 17.4
  2.65, 5.65, 12.15, 17.9
};

void setUpABLThresholds( float* ABL_thresholds )
{
  int i;

  for ( i = 0; i < 4; i++ )
  {
    thresholds_tbl_dB[i] = (double)ABL_thresholds[i];
  }
}

int BPLA_scheme_v3_1(int* modulation, float* power, sort_str* sorted_ENP, float* ENP,
  float power_limit, float thresholdFCC, int N_subcarriers)
{
  float tmp_val;
  float def_tmp_val;
  float tmp_val1;
  float out_power;
  int i;
  int j;
  int Non;
  int r;
  int res = 0;


  def_tmp_val = N_subcarriers / power_limit;

  Non = N_subcarriers;
  r = -1;
  while((r == -1) && (Non >= 0)) {
    Non--;
    tmp_val = power_limit/(Non + 1.f);
    if (tmp_val > thresholdFCC) {
      tmp_val = thresholdFCC;
    }
    tmp_val1 = 10.f*(float)log10(tmp_val * def_tmp_val) + sorted_ENP[Non].value;
    for (i = 3; i >= 0; i--) {
      if ( tmp_val1 >=(thresholds_tbl_dB[i]-1.0)) {
        r = i;
        break;
      }
    }
  }
  out_power = tmp_val * (Non + 1.f);
  for (i = 0; i <= Non; i++) {
    power[sorted_ENP[i].index] = tmp_val;
    modulation[i] = 0;
  }
  for (; i < N_subcarriers; i++) {
    power[sorted_ENP[i].index] = 0.0f;
    modulation[i] = 0;
  }
  if (r != -1) {
    tmp_val1 = 10.f * (float)log10(tmp_val * def_tmp_val);
    for (i = 0; i < N_subcarriers; i+=2) {
      tmp_val = tmp_val1 +  ENP[i];
      /*we have four modulation types                */
      for(j = 3; j >=0; j--) {
        if (tmp_val >= (thresholds_tbl_dB[j] - 1.0)) {
          modulation[i] = j + 1;
          modulation[i+1] = j + 1;
          break;
        }
      }
    }
  } else {
    out_power = power_limit;
    tmp_val  = power_limit / (float)N_subcarriers;
    for (i = 0; i < N_subcarriers; i++) {
      modulation[i] = 1;
      power[i] = tmp_val;
    }
    res = 1;
  }
  return res;
}

////////////////////////////////////////////////////////////////////////////////
// Name: Wraper_for_BPLA
//
// Purpose: Wrapper for BPLA proposed by INNL team
//
// Author: Alexandr Kozlov
//
// Parameters: freq_tr_syms - input, Frequency training symbols
//
//             bits_per_symbol - output, number of bits to be transmitted in each subchannel 
//
// Returning value: 0 - ok, -1 - error
////////////////////////////////////////////////////////////////////////////////
int Wraper_for_BPLA( const fxp_complex* freq_tr_syms,
                     const fxp_complex* additional_freq_tr_syms,
                     unsigned* bits_per_symbol,
                     float* data_profile )
{
  unsigned i,j;

  float profile[NumOfDataSubcarriers];
  float power[NumOfDataSubcarriers];
  int modulation[NumOfDataSubcarriers];
  float ENP[4 * NumOfDataSubcarriers];
  sort_str* sorted_ENP;

  float noise_variance = 0;
  float re, im;

  float energy = 0;
  
  float av_SNR = 0;

  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
    re = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re - 
         freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re );

    im = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im - 
         freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im );

    noise_variance += re * re + im * im;

    if ( additional_freq_tr_syms != NULL )
    {
      re = (float)( additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re - 
           additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re );

      im = (float)( additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im - 
           additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im );

      noise_variance += re * re + im * im;
    }

    if ( sim_params.idealChannelEstimation )
    {
      //[ For ideal estimation
      re = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re + 
           freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re ) / 2;

      im = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im + 
           freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im ) / 2;

      energy += re * re + im * im;
      //]
    }
  }
  
  if ( additional_freq_tr_syms != NULL ) 
  {
    noise_variance /= 2 * NumOfDataSubcarriers;
  }
  else
  {
    noise_variance /= NumOfDataSubcarriers;
  }

  if ( sim_params.idealChannelEstimation )
  {
    //[ For ideal estimation
    energy /= NumOfDataSubcarriers;
    noise_variance = energy * exp( log(10) * -(float)sim_params.curSNR / 10 ) * 2;
    //]
  }

	for ( i = 0; i < NumOfDataSubcarriers; i++ ) 
  {
    if ( additional_freq_tr_syms != NULL ) 
    {
      re = (float)( ( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re + 
                      freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re +  
                      additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re + 
                      additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re  
                    ) ) / 4;

      im = (float)( ( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im + 
                      freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im +
                      additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im + 
                      additional_freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im  
                    ) ) / 4;
    }
    else
    {
      re = (float)( ( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re + 
                      freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re 
                    ) ) / 2;

      im = (float)( ( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im + 
                      freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im 
                    ) ) / 2;
    }

    profile[i] = ( re * re + im * im ) / ( noise_variance / 2 );
	}
	/* data preparation */

	/* Effective Noise power calculation */
	for (i = 0; i < NumOfDataSubcarriers; i++) {
		if (profile[i] > 0.0) {
			ENP[i] = 10.f*(float)log10(profile[i]);
		} else {
    /* we can make assumption about the energy level for this sub-carrier */
			ENP[i] =  -1000.;
		}
	}


  //[ UBLwSP
/*  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
    av_SNR += ENP[i];
  }
  av_SNR /= NumOfDataSubcarriers;

	/*for( j = 0; j < 4; j++)
	{
		if( av_SNR < thresholds_tbl_dB[j] )
			break;
	}

  if ( j == 0 ) j = 1;

  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
    if ( ENP[i] < low_thresholds_tbl_dB )
    {
      bits_per_symbol[i] = 0;
    }
    else
    {
      switch ( j )
      {
        case 1:  bits_per_symbol[i] = 1; break;

        case 2:  bits_per_symbol[i] = 2; break;

        case 3:  bits_per_symbol[i] = 4; break;

        case 4:  bits_per_symbol[i] = 6; 
      }
    }
  }/**/
  //]

	/* sorted_ENP = (sort_str*) &ENP[NumOfDataSubcarriers];
	for (i = 0; i < NumOfDataSubcarriers; i++) {
		sorted_ENP[i].index = i;
		sorted_ENP[i].value = ENP[i];
	}

	qsort( sorted_ENP, NumOfDataSubcarriers, sizeof(sort_str), compare_inv_sort_str );

  for ( i = 0; i < 10; i++ )
  {
    av_SNR += sorted_ENP[i].value;
  }
  av_SNR /= 10;/**/ 
 
  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
		for( j = 0; j < 4; j++)
		{
			if( ENP[i] < thresholds_tbl_dB[j] )
				break;
		}

    switch ( j )
    {
      case 0:  bits_per_symbol[i] = 0; break;

      case 1:  bits_per_symbol[i] = 1; break;

      case 2:  bits_per_symbol[i] = 2; break;

      case 3:  bits_per_symbol[i] = 4; break;

      case 4:  bits_per_symbol[i] = 6; 
    }

    // check average SNR if its OK for LDPC code use 64-QAM in each subcarrier
    /*if ( av_SNR > 23.0 ) 
    {
      bits_per_symbol[i] = 6;
    }/**/
  }

  return 0;
}


/*******************************************************************
 *
 *  Name: BPLA_fxp
 *
 *  Purpose: fixed-point bitloading algorithm from ITPC
 *
 *  Author:  Mikhail Lyakh
 *
 *  Parameters: freq_tr_syms - input, Frequency training symbols
 *				bits_per_symbol - output, number of bits to be transmitted in each subchannel
 *
 *  Return value: 0 - ok, -1 - error
 *
 *******************************************************************
 */

//const double SNR_thresholds[4] = { 5.62, 8.22, 11.18, 12.76 };
// const double SNR_thresholds[4] = {  3.65, 6.65, 13.15, 18.9 };
//const double SNR_thresholds[4] = {  4.95, 7.83, 13.81, 19.26 };
const double SNR_thresholds[4] = {  5.95, 8.83, 14.81, 20.26 };

int BPLA_fxp ( const fxp_complex* freq_tr_syms,
                     unsigned* bits_per_symbol,
					 unsigned data_precision,
					 unsigned thresh_precision)
{
	char overflow_flag;
	char overflow_comment[100]="";
	unsigned i,j, data_subc_ind;
	
	int fxp_SNR_thresholds[4];

	int freq_tr_syms_bl_re[2 * NumOfDataSubcarriers], freq_tr_syms_bl_im[2 * NumOfDataSubcarriers];
	int subc_power[NumOfDataSubcarriers];

	int noise_power=0;
	int re, im;
	
	int sub_prec=data_precision, 
		mul_prec=2*data_precision, 
		mul_pos=0, 
		add_prec=mul_prec+7;

	
	
	//Preparing data for BL block precisions of data and thresholds
	for (i=0;i<4; i++)
		fxp_SNR_thresholds[i]=convert_flp_to_fxp(exp(0.1*log(10)*SNR_thresholds[i]),
												 thresh_precision,
												 BL_EXP_POSITION,
												 &overflow_flag,
												 overflow_comment);

	for (i=0;i<2*NumOfDataSubcarriers;i++)
	{
		data_subc_ind=sim_consts.DataSubcPatt[i%NumOfDataSubcarriers] + NumOfUsedSubcarriers*(i>=NumOfDataSubcarriers)- 1;
		freq_tr_syms_bl_re[i]=convert_precision(freq_tr_syms[ data_subc_ind ].re,
											   fxp_params.FFT_precision,
											   fxp_params.FFT_precision-data_precision,
											   data_precision,
											   0,
											   &overflow_flag, overflow_comment);
		freq_tr_syms_bl_im[i]=convert_precision(freq_tr_syms[ data_subc_ind ].im,
											   fxp_params.FFT_precision,
											   fxp_params.FFT_precision-data_precision,
											   data_precision,
											   0,
											   &overflow_flag, overflow_comment);
	}


	//calculate noise power and subcarrier powers
	

	for (i=0;i<NumOfDataSubcarriers;i++)
	{

		re=fxp_sub(freq_tr_syms_bl_re[i],
				   freq_tr_syms_bl_re[ i + NumOfDataSubcarriers],
				   sub_prec,
				   &overflow_flag,overflow_comment);


		re=fxp_mul(re,re,mul_prec,mul_pos,&overflow_flag,overflow_comment);

		noise_power=fxp_add(noise_power,re,add_prec,&overflow_flag,overflow_comment);

		im=fxp_sub(freq_tr_syms_bl_im[i],
				   freq_tr_syms_bl_im[ i + NumOfDataSubcarriers],
				   sub_prec,
				   &overflow_flag,overflow_comment);
		im=fxp_mul(im,im,mul_prec,mul_pos,&overflow_flag,overflow_comment);

		noise_power=fxp_add(noise_power,im,add_prec,&overflow_flag,overflow_comment);


		re=fxp_add(freq_tr_syms_bl_re[i],
				   freq_tr_syms_bl_re[i+NumOfDataSubcarriers],
				   data_precision+1,
				   &overflow_flag,overflow_comment);
		re=fxp_mul(re,re,2*data_precision+2,0,
				   &overflow_flag,overflow_comment);
		
		im=fxp_add(freq_tr_syms_bl_im[i],
				   freq_tr_syms_bl_im[i+NumOfDataSubcarriers],
				   data_precision+1,
				   &overflow_flag,overflow_comment);
		im=fxp_mul(im,im,2*data_precision+2,0,
				   &overflow_flag,overflow_comment);
		
		subc_power[i]=fxp_add(re,im,
							  2*data_precision+3,&overflow_flag, overflow_comment);
		subc_power[i]=fxp_mul(subc_power[i],NumOfDataSubcarriers>>1,
							  2*data_precision-mul_pos,
							  3+mul_pos,
							  &overflow_flag, overflow_comment);

		
	}


	noise_power=convert_precision(noise_power,add_prec,3,
											  add_prec-3,0,
											  &overflow_flag, overflow_comment);
	
	for(j=0;j<4;j++)
		fxp_SNR_thresholds[j]=fxp_mul(fxp_SNR_thresholds[j],noise_power,
								      2*data_precision-mul_pos,
									  BL_EXP_POSITION,
									  &overflow_flag, overflow_comment);

	
	for(i=0;i<NumOfDataSubcarriers;i++)
	{
		for(j=0;j<4;j++)
		{
			if(subc_power[i]<fxp_SNR_thresholds[j])
				break;
		}

		switch(j)
		{
		  case 0: bits_per_symbol[i]=0;
				  break;
		  case 1: bits_per_symbol[i]=1;
				  break;
		  case 2: bits_per_symbol[i]=2;
				  break;
		  case 3: bits_per_symbol[i]=4;
				  break;
		  case 4: bits_per_symbol[i]=6;
		}
	}



	return 0;			   

}


/**********************************************************************
 *
 *	Name: BPLA_flp
 *
 *	Purpose: floating-point algorithm of BPLA_fxp
 *
 **********************************************************************/

int BPLA_flp( const fxp_complex* freq_tr_syms,
                     unsigned* bits_per_symbol )
{
  unsigned i,j;

  unsigned min_N_on=12,N_on=0;

  double subc_power[NumOfDataSubcarriers],subc_power1[NumOfDataSubcarriers], 
	  subc_power2[NumOfDataSubcarriers],subc_power3[NumOfDataSubcarriers];

  
	double flp_SNR_thresholds[4];

	unsigned scs_on[NumOfDataSubcarriers];

  double noise_power = 0;
  double re, im;

  for(j=0;j<4;j++)
	  flp_SNR_thresholds[j]=exp(0.1*log(10)*SNR_thresholds[j]);


  for ( i = 0; i < NumOfDataSubcarriers; i++ )
  {
    re = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re - 
         freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re );

    im = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im - 
         freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im );

    noise_power += re * re + im * im;

	re = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].re + 
         freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].re );

    im = (float)( freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 ].im + 
         freq_tr_syms[ sim_consts.DataSubcPatt[i] - 1 + NumOfUsedSubcarriers ].im );


	subc_power[i]=(re*re+im*im)*.5*NumOfDataSubcarriers;
  }
//  noise_variance /= NumOfDataSubcarriers;



	
//  Smoothing response. Method 2
	subc_power2[0]=subc_power[0];
	for(i=1;i<NumOfDataSubcarriers-1;i++)
		subc_power2[i]=.5*(subc_power[i]+ .5*(subc_power[i-1]+subc_power[i+1]));
	subc_power2[NumOfDataSubcarriers-1]=subc_power[NumOfDataSubcarriers-1];

//	Smoothing response. Method 1
	for(i=0; i<NumOfDataSubcarriers/2; i++)
	{
		subc_power1[i*2]=(subc_power2[i*2]+subc_power2[i*2+1])/2;
		subc_power1[i*2+1]=subc_power1[i*2];
	}

//	Smoothing response. Metod 3 (minimum)

	subc_power3[0]=min(subc_power[0],subc_power[1]);

	for (i=1; i<NumOfDataSubcarriers-1;i++)
		subc_power3[i]=min(min(subc_power[i-1],subc_power[i]),subc_power[i+1]);
	subc_power3[i]=min(subc_power[i-1],subc_power[i]);

	
//	for(i=0; i< NumOfDataSubcarriers;i++)
//		if(subc_power1[i]/noise_power<flp_SNR_thresholds[0])
//			scs_on[i]=0;
//		else
//		{
//			scs_on[i]=1;
//			N_on++;
//		}


	
	for(i=0;i<NumOfDataSubcarriers;i++)
	{

		subc_power1[i]/=noise_power;
		
		subc_power3[i]/=noise_power;
		subc_power[i]/=noise_power;

		for(j=0;j<4;j++)
		{
			if(subc_power1[i]<flp_SNR_thresholds[j])
				break;
		}

		switch(j)
		{
		  case 0: bits_per_symbol[i]=0;
				  break;
		  case 1: bits_per_symbol[i]=1;
				  break;
		  case 2: bits_per_symbol[i]=2;
				  break;
		  case 3: bits_per_symbol[i]=4;
				  break;
		  case 4: bits_per_symbol[i]=6;
		}
	}

  return 0;
}