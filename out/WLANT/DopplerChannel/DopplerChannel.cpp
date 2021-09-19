
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                             //                                                //
//    Channel.c                                //    (c) COPYRIGHT  2003                         //
//    John S. Sadowsky                         //        Intel Corporation                       //
//                                             //                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Revision
//  11/24/03  Corrected two bugs: the "H_E bug" found by Sergey, plus a bug in the Doppler
//            offset feature.  I also added a normalization flag at the top of the *.cfg file.
//            When set, the frequency response is normalized so that to average H_E[kRx][kTx][kf]
//            is 1.
//  11/25/03  Flat channel mode now generates H_E, H_I & H_Q - exactly like other modes.
//  11/28/03  Eliminated the [channelGeneral] field in the *.cfg file.  All basic parameters
//            (like carrier frequency and antenna spacing) are now initialized through the
//            argument list of the initialization function.
//  02/15/04  Fixed bug in channel corretion - only impacted case of Tx correlation with no
//            Rx correlation (as when numRxAntenna = 0).
//  02/23/04  Added a time domain run time calling function.
//  03/01/04  Did extensive debugging of time domain calling function - it appears to work.
//            Also filled out the Fourier matrix for the AWGN model specified by TGn.
//  03/05/04  Fixed bug in square root correlation calculation - previously we were calculating
//            the Rx square root twice, rather than Rx and Tx.
//  03/07/04  Fixed bug in energy calculation in GenerateImpulseResponse() (Had NumTxAntenna
//            instead of NumRxAntenna.
//  03/10/04  Bug fix - in UpdateImpulseResponse() - doppler was applied only to the I component.
//            We now apply doppler to both I & Q.
//  03/11/04  Added LOS component on the 1st tap - the following data structures and sub-routines
//            have been modified:
//            (1) ChannelParamsStruct in Channel.h - added 7 new fields
//                boolean       lineOfSightFlag ;
//                float         RicianFactorK_dB ;
//                float         AoA_deg_LOS ;
//                float         AoD_deg_LOS ;
//                float         sigma_LOS ;
//                float       **phase_LOS_I ;
//                float       **phase_LOS_Q ;
//            (2) Added a new sub-routine CalcPhaseMatrixLOS() - it calculates the phase
//                matrix of the LOS component.
//            (3) GetVinkoParams() - added new code lines that read in or calculate the
//                2nd to 7th new fields of "channelParams" (ChannelParamsStruct).
//                The sub-routine CalcPhaseMatrixLOS() is called here.
//            (4) InitChannelState () - case Vinko: channelState->h_I and channelState->h_Q
//                are allocated space unconditionally in order to accomodate the possible
//                LOS component.
//            (5) CalcTotalImpulseResponse() - if "lineOfSightFlag == T",
//                the LOS component is included in channelState->h_I and channelState->h_Q.
//            (6) FreeChannelState() - always free up the space held by channelState->h_I (h_Q).
//            (7) FreeChannelParams() - if applicable, free up the space held by
//                channelParams->phase_LOS_I (phase_LOS_Q).
// 03/16/04   Removed antenna spacing from Initialize_Channel() argument list.  These parameters
//            are now read from the top of the ChannelModel.cfg.
// 03/16/04   The former constant DOPPLER_UPDATE_FRAC was changed to a parameter that is read
//            from the config file.
// 03/21/04   Edited LOS features.
// 04/06/04   Added the WriteToFile function
//            Removed maxSamplingError_nsec from init argument list - now read from file
// 04/10/04   Fixed minor bug in AWGN mode (did not allocate delay_ns)
// 04/24/04   Fixed minor bug in Params allocation for EandV channels - set h_LOS pointers to
//            default NULL.
// 04/27/04   Fixed MAJOR bug in Doppler time propagation.
// 05/19/04   Added simple GenerateChannelResponse() and UpdateChannelResponse() calling funcitons.
//            These may be used in either time or frequency domain simulations.
// 06/17/04   Fixed bug in sqrt{Rho_Tx} multiplication for Tx spatial correlation.
// 07/09/04   Changed the normalization proceedure to Frequency Domaion normalization, rather
//            than time domain normalization.  Now, even if only time domain impulse responses
//            are used, the numSubcarrier and subCarrierSpacing_kHz parameters are important to
//            the normalization process.
// 07/27/04   Fixed Laplacian wrapping bug in correlation calculation.  Also fixed a small bug
//            in Laplacian normalization.  Plus several other "editorial" changes.
//
//////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "DopplerChannel.h"

#define   CHANNEL_DEBUG

#define   NUM_PHI      (3600)
#define   DEL_PHI_RAD  (0.001745329)     // 2 Pi / NUM_PHI
#define   DEL_PHI_DEG  (0.1)             // 360 / NUM_PHI


// #define   DOPPLER_UPDATE_FRAC   (0.02)


///////////////////////////////////////////////////////////////////////////////////////////////////
// Local Functions                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

void GetEandV_Params(
  char                       *channelFileName ,
  char                       *fieldName ,
  ChannelParamsStruct        *channelParams
) ;

void InitEandV_State(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void GetVinkoParams(
  char                       *channelFileName ,
  char                       *fieldName ,
  ChannelParamsStruct        *channelParams
) ;

void InitVinkoState(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void InitDopplerParams (
  ChannelParamsStruct        *channelParams
) ;

void InitDopplerState (
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void ResetDopplerState (
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void CalcTotalImpulseResponse(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void CorrelateChannel(
  int                         kDoppler ,
  int                         kTap ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void CalcCorrelationMatrix(
  int                         numAntenna ,
  float                       antennaSpacing_cm ,
  float                       lambda_dm ,
  float                     **correlation_I ,
  float                     **correlation_Q ,
  double                     *PAS
) ;

void CalcFreqResponse(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void MatSqRoot_Flt(
  int                 n ,
  float             **A_I ,
  float             **A_Q ,
  float             **S_I ,
  float             **S_Q
) ;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization Functions                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// WriteToFileChannelParams()                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////

void WriteToFileChannelParams (
  FILE                       *out ,
  ChannelParamsStruct        *channelParams
) {

  fprintf( out , "***********************************************************************\n" ) ;
  fprintf( out , "Channel Params\n" ) ;
  fprintf( out , "               Model Name:  %s\n" , channelParams->channelModelName ) ;

  switch ( channelParams->channelType ) {

    case AWGN_CH :
      fprintf( out , "             Channel Type:  AWGN\n" ) ;
      break;

    case E_AND_V :
      fprintf( out , "             Channel Type:  Exp. Delay Spread (E and V)\n" ) ;
      fprintf( out , "         RMS Delay Spread:  %6.2f n sec\n" , channelParams->rmsDelaySpread_ns ) ;
      fprintf( out , "           Sample Delta t:  %6.2f n sec\n" , channelParams->deltaTau_ns ) ;
      fprintf( out , "        Max. Delay Spread:  %6.2f n sec\n" , channelParams->maxDelay_ns ) ;
      break;

    case VINKO :
      fprintf( out , "             Channel Type:  IEEE TGn Model\n" ) ;
      fprintf( out , "             Num. Antenna:  Tx-%1d       Rx-%1d\n" ,
                                        channelParams->numTxAntenna , channelParams->numRxAntenna ) ;
      fprintf( out , "          Antenna Spacing:  Tx-%6.2f  Rx-%6.2f  (cm)\n" ,
                                        channelParams->txAntennaSpacing_cm , channelParams->rxAntennaSpacing_cm ) ;
      fprintf( out , "            Carrier Freq.:  %6.2f (GHz)\n" ,           channelParams->carrierFreq_GHz ) ;
      fprintf( out , "     Doppler Update Frac.:  %8.4f\n" ,         channelParams->dopplerUpdateFrac ) ;
      if ( channelParams->freqDomainFlag == T )
        fprintf( out , "   TD Max. Sampling Error:  %6.2f (n sec)\n" , channelParams->maxSamplingError_nsec ) ;
      if ( channelParams->pwrNormFlag == T ) {
        fprintf( out , "    Channel Normalization: YES\n" ) ;
      } else {
        fprintf( out , "    Channel Normalization: NO\n" ) ;
      }
      break;

    default:
      assert(0) ;
  }

  fprintf( out , "***********************************************************************\n" ) ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize_Channel()                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////

void Initialize_Channel (
  char                       *channelFileName ,
  char                       *channelModelName ,
  int                         numTxAntenna ,
  int                         numRxAntenna ,
  float                       sampleRate_Msps ,
  boolean                     pwrNormFlag ,
  boolean                     freqDomainFlag ,
  int                         numSubCarrier ,
  float                       subCarrierSpacing_kHz ,
  RNG_StateStruct            *chRngState ,
  RNG_StateStruct            *noiseRngState ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {


  InitChannelParams (         channelFileName ,
                              channelModelName ,
                              numTxAntenna ,
                              numRxAntenna ,
                              sampleRate_Msps ,
                              pwrNormFlag ,
                              freqDomainFlag ,
                              numSubCarrier ,
                              subCarrierSpacing_kHz ,
                              channelParams ) ;

  InitChannelState (          chRngState ,
                              noiseRngState ,
                              channelParams ,
                              channelState ) ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InitChannelParams()                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitChannelParams (
  char                       *channelFileName ,
  char                       *channelModelName ,
  int                         numTxAntenna ,
  int                         numRxAntenna ,
  float                       sampleRate_Msps ,
  boolean                     pwrNormFlag ,
  boolean                     freqDomainFlag ,
  int                         numSubCarrier ,
  float                       subCarrierSpacing_kHz ,
  ChannelParamsStruct        *channelParams
) {

  char                        fieldName[100] ;
  int                         kSC, kTau, numDelay ;
  double                      x1, x2, x3 ;
  float                      *cs, *sn ;

  strcpy( channelParams->channelModelName , channelModelName ) ;

  ReadFloat( "channelGeneral" , "carrierFreq_GHz",       channelFileName , &(channelParams->carrierFreq_GHz) ) ;
  ReadFloat( "channelGeneral" , "rxAntennaSpacing_cm" ,  channelFileName , &(channelParams->rxAntennaSpacing_cm) ) ;
  ReadFloat( "channelGeneral" , "txAntennaSpacing_cm" ,  channelFileName , &(channelParams->txAntennaSpacing_cm) ) ;

  ReadFloat( "channelGeneral" , "dopplerUpdateFrac",     channelFileName , &(channelParams->dopplerUpdateFrac) ) ;
  ReadFloat( "channelGeneral" , "maxSamplingError_nsec", channelFileName , &(channelParams->maxSamplingError_nsec) ) ;

  channelParams->sampleRate_Msps = sampleRate_Msps ;
  channelParams->pwrNormFlag     = pwrNormFlag ;

  channelParams->lambda_cm       = 30.0f / channelParams->carrierFreq_GHz ;

  // Antenna parameters

  channelParams->numTxAntenna          = numTxAntenna ;
  channelParams->numRxAntenna          = numRxAntenna ;

  assert( numTxAntenna > 0 ) ;
  assert( numRxAntenna > 0 ) ;

  channelParams->MIMO_Flag = ((numTxAntenna == 1) && (numRxAntenna == 1)) ? F : T ;
  channelParams->txCorrelationFlag = F ;
  channelParams->rxCorrelationFlag = F ;

  // Set all pointers to NULL

  channelParams->dopplerParams = NULL ;

  channelParams->sn = NULL ;
  channelParams->cs = NULL ;

  channelParams->h_LOS_I = NULL ;
  channelParams->h_LOS_Q = NULL ;

  // Default - no LOS

  channelParams->LOS_Flag         = F ;
  channelParams->RicianFactorK_dB = 0.0f ;
  channelParams->AoA_deg_LOS      = 0.0f ;
  channelParams->AoD_deg_LOS      = 0.0f ;

  // Frequency vs. Time Domain Flag

  channelParams->freqDomainFlag        = freqDomainFlag ;
  channelParams->numSubCarrier         = numSubCarrier ;
  channelParams->subCarrierSpacing_kHz = subCarrierSpacing_kHz ;


  // Find Channel Model

  FindNumberedField( "channelModel" , channelParams->channelModelName , channelFileName , fieldName ) ;

  ReadInt( fieldName , "channelType",   channelFileName , "decimal" , (int*)&(channelParams->channelType) ) ;

  switch ( channelParams->channelType ) {

    case AWGN_CH :

      channelParams->numDoppler     = 0 ;
      channelParams->dopplerFlag    = F ;
      channelParams->dopplerParams  = NULL ;
      channelParams->delays_ns      = NULL ;
      ALLOCATE_VECTOR( channelParams->delays_ns ,      1 , float ) ;
      ALLOCATE_VECTOR( channelParams->delays_samples , 1 , int ) ;
      channelParams->delays_ns[0]      = 0.0f ;
      channelParams->delays_samples[0] = 0 ;
      return ;

    case E_AND_V :

      GetEandV_Params( channelFileName , fieldName , channelParams ) ;
      break ;

    case VINKO :

      GetVinkoParams(  channelFileName , fieldName , channelParams ) ;
      break ;

    default :
      assert(0) ;
  }

  numDelay = channelParams->numDelay ;

  channelParams->maxDelay_samples = -1 ;
  if ( channelParams->freqDomainFlag == F ) {
    for ( kTau = 0 ; kTau < numDelay ; kTau++ ) {
      if ( channelParams->maxDelay_samples < channelParams->delays_samples[kTau] ) {
           channelParams->maxDelay_samples = channelParams->delays_samples[kTau] ;
    } }
  }

  // Allocate Memory

  if ( channelParams->channelType != AWGN_CH ) {

    ALLOCATE_MATRIX( channelParams->cs , numSubCarrier , numDelay , float ) ;
    ALLOCATE_MATRIX( channelParams->sn , numSubCarrier , numDelay , float ) ;

    x1 = TWO_PI * 1.0e-6 * subCarrierSpacing_kHz ;
    for ( kSC = 0 ; kSC < numSubCarrier ; kSC++ ) {
      cs = channelParams->cs[kSC] ;
      sn = channelParams->sn[kSC] ;
      x2 = (kSC - (numSubCarrier/2)) * x1 ;
      for ( kTau = 0 ; kTau < channelParams->numDelay ; kTau++ ) {
        x3 = x2 * channelParams->delays_ns[kTau] ;
        cs[kTau] = (float) cos( x3 ) ;
        sn[kTau] = (float) sin( x3 ) ;
      }
    }

  } // End of if ( channelParams->channelType != FLAT_CH ) {


  // Initialize Doppler Params

  InitDopplerParams ( channelParams ) ;

  channelParams->dopplerFlag = F ;
  if (     (channelParams->numDoppler > 1)
       || ((channelParams->numDoppler == 1)&&(channelParams->dopplerParams->dopplerOffsetFlag == T)) ) {
    channelParams->dopplerFlag = T ;
  } else {
    channelParams->dopplerFlag = F ;
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InitChannelState ()                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitChannelState (
  RNG_StateStruct            *chRngState ,
  RNG_StateStruct            *noiseRngState ,
  ChannelParamsStruct        *channelParams ,
  ChannelStateStruct         *channelState
) {

  int                         kRx, kTx, kSC, numTxAntenna, numRxAntenna ;
  double                      phi_0 ;
  FreqDomainChannelStruct    *freqDomainChannel ;

  channelState->chRngState    = chRngState ;
  channelState->noiseRngState = noiseRngState ;

  numTxAntenna = channelParams->numTxAntenna ;
  numRxAntenna = channelParams->numRxAntenna ;

  channelState->h_I = NULL ;
  channelState->h_Q = NULL ;

  channelState->xBuffer_I = NULL ;
  channelState->xBuffer_Q = NULL ;

  channelState->freqDomainChannel.H_I = NULL ;
  channelState->freqDomainChannel.H_Q = NULL ;
  channelState->freqDomainChannel.H_E = NULL ;

  switch ( channelParams->channelType ) {

    case AWGN_CH :

      channelParams->numDelay         = 1 ;
      channelParams->maxDelay_samples = 0 ;
      channelState->dopplerState = NULL ;
      break;

    case E_AND_V :

      InitEandV_State( channelState , channelParams ) ;
      break ;

    case VINKO :

      InitVinkoState(  channelState , channelParams ) ;
      break ;

    default :
      assert(0) ;
  }

  // Allocate Memory
  switch ( channelParams->channelType ) {

    case AWGN_CH :

      ALLOCATE_VECTOR( channelState->xBuffer_I , numTxAntenna , float* ) ;
      ALLOCATE_VECTOR( channelState->xBuffer_Q , numTxAntenna , float* ) ;
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        channelState->xBuffer_I[kTx] = NULL ;
        channelState->xBuffer_Q[kTx] = NULL ;
      }
      ALLOCATE_3D_MATRIX( channelState->h_I , numRxAntenna , numTxAntenna , 1 , float ) ;
      ALLOCATE_3D_MATRIX( channelState->h_Q , numRxAntenna , numTxAntenna , 1 , float ) ;
      ALLOCATE_MATRIX(    channelState->h_E , numRxAntenna , numTxAntenna ,     float ) ;

      phi_0 = ( numRxAntenna > numTxAntenna ) ? TWO_PI / ((double) numRxAntenna)
                                              : TWO_PI / ((double) numTxAntenna) ;

      for ( kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
        for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
          channelState->h_I[kRx][kTx][0] = (float)   cos( kRx*kTx*phi_0 ) ;
          channelState->h_Q[kRx][kTx][0] = (float) (-sin( kRx*kTx*phi_0 )) ;
        }
        channelState->h_E[kRx][kRx]    = 1.0f ;
      }
      break ;

    case E_AND_V :
    case VINKO :

      ALLOCATE_MATRIX(      channelState->h_E , numRxAntenna , numTxAntenna , float ) ;
      if ( channelParams->dopplerFlag == T ) {
        ALLOCATE_3D_MATRIX( channelState->h_I , numRxAntenna , numTxAntenna , channelParams->numDelay , float ) ;
        ALLOCATE_3D_MATRIX( channelState->h_Q , numRxAntenna , numTxAntenna , channelParams->numDelay , float ) ;
      } else {
        channelState->h_I = channelState->dopplerState->h_I ;
        channelState->h_Q = channelState->dopplerState->h_Q ;
      }
      break ;

    default :
      assert(0) ;
  }


  freqDomainChannel = &(channelState->freqDomainChannel) ;

  if ( channelParams->freqDomainFlag == T ) {

    ALLOCATE_3D_MATRIX( freqDomainChannel->H_I ,  numRxAntenna , numTxAntenna , channelParams->numSubCarrier , float ) ;
    ALLOCATE_3D_MATRIX( freqDomainChannel->H_Q ,  numRxAntenna , numTxAntenna , channelParams->numSubCarrier , float ) ;
    ALLOCATE_3D_MATRIX( freqDomainChannel->H_E ,  numRxAntenna , numTxAntenna , channelParams->numSubCarrier , float ) ;
    ALLOCATE_MATRIX(    freqDomainChannel->AvgE , numRxAntenna , numTxAntenna , float ) ;

    channelState->xBuffer_I = NULL ;
    channelState->xBuffer_Q = NULL ;

    if ( channelParams->channelType == AWGN_CH ) {
      for (     kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
        for (   kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
          freqDomainChannel->AvgE[kRx][kTx] = 1.0f ;
          for ( kSC = 0 ; kSC < channelParams->numSubCarrier ; kSC++ ) {
            freqDomainChannel->H_I[kRx][kTx][kSC] = channelState->h_I[kRx][kTx][0] ;
            freqDomainChannel->H_Q[kRx][kTx][kSC] = channelState->h_Q[kRx][kTx][0] ;
            freqDomainChannel->H_E[kRx][kTx][kSC] = 1.0f ;
      } } }
    }

  } else {  // channelParams->freqDomainFlag == F

    if ( channelParams->maxDelay_samples > 0 ) {
      ALLOCATE_MATRIX( channelState->xBuffer_I , numTxAntenna , channelParams->maxDelay_samples , float ) ;
      ALLOCATE_MATRIX( channelState->xBuffer_Q , numTxAntenna , channelParams->maxDelay_samples , float ) ;
    }

    freqDomainChannel->H_I  = NULL ;
    freqDomainChannel->H_Q  = NULL ;
    freqDomainChannel->H_E  = NULL ;
    freqDomainChannel->AvgE = NULL ;

  }

  InitDopplerState ( channelState , channelParams ) ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InitDopplerParams()                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitDopplerParams (
  ChannelParamsStruct        *channelParams
) {

  int                         kDoppler ;
  DopplerParamsStruct        *dopplerParams ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;

    dopplerParams->beta = (float) ( TWO_PI * dopplerParams->dopplerBW_Hz / 3.0e+6 ) ;
    dopplerParams->updateTime0_muSec = (float) (- log( 1.0 - channelParams->dopplerUpdateFrac ) / dopplerParams->beta ) ;

    if ( dopplerParams->dopplerOffset_Hz == 0.0f ) {
      dopplerParams->dopplerOffsetFlag = F ;
      dopplerParams->updateTime1_muSec = 1.0e+20f ;
      dopplerParams->omega             = 0.0f ;
    } else {
      dopplerParams->dopplerOffsetFlag = T ;
      dopplerParams->updateTime1_muSec = (float) (1.0e+6 * channelParams->dopplerUpdateFrac / (TWO_PI * dopplerParams->dopplerOffset_Hz)) ;
      dopplerParams->omega             = (float) (1.0e-6 * TWO_PI * dopplerParams->dopplerOffset_Hz ) ;
    }
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InitDopplerState()                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitDopplerState (
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kDoppler ;
  DopplerParamsStruct        *dopplerParams ;
  DopplerStateStruct         *dopplerState ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    dopplerState  = &(channelState->dopplerState[kDoppler]) ;

    ALLOCATE_VECTOR( dopplerState->nextUpdateTime0_muSec , channelParams->numDelay , float ) ;

    if ( dopplerParams->dopplerOffsetFlag == F ) {
      dopplerState->nextUpdateTime1_muSec = NULL ;
      dopplerState->cs = NULL ;
      dopplerState->sn = NULL ;
    } else {
      ALLOCATE_VECTOR( dopplerState->nextUpdateTime1_muSec , channelParams->numDelay , float ) ;
      ALLOCATE_VECTOR( dopplerState->cs ,                    channelParams->numDelay , float ) ;
      ALLOCATE_VECTOR( dopplerState->sn ,                    channelParams->numDelay , float ) ;
    }

  }  // End of kDoppler state

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetEandV_Params()                                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

void GetEandV_Params(
  char                       *channelFileName ,
  char                       *fieldName ,
  ChannelParamsStruct        *channelParams
) {

  double                      a , z, p, x1, x2 ;
  float                       dTau_ns ;
  int                         delSamp, kTap, numDelay ;
  DopplerParamsStruct        *dopplerParams ;

  ReadFloat( fieldName , "rmsDelaySpread_ns",   channelFileName , &(channelParams->rmsDelaySpread_ns) ) ;
  ReadFloat( fieldName , "deltaTau_ns",         channelFileName , &(channelParams->deltaTau_ns) ) ;
  ReadFloat( fieldName , "maxDelay_ns",         channelFileName , &(channelParams->maxDelay_ns) ) ;

  // For Time Domain sims, align delays with integral samples

  if ( channelParams->freqDomainFlag == T ) {
    delSamp = 0 ;
  } else {
    dTau_ns = 1000.0f / channelParams->sampleRate_Msps ;
    delSamp = (int) (0.5f + (channelParams->deltaTau_ns / dTau_ns)) ;
    if (    (dTau_ns > (channelParams->deltaTau_ns + channelParams->maxSamplingError_nsec))
         || (fabs(channelParams->deltaTau_ns - delSamp*dTau_ns) > channelParams->maxSamplingError_nsec) ) {
      printf("\n") ;
      printf("************************************************************************\n") ;
      printf("E and V Channel Parameter Error: analog Sample Rate - deltaTau missmatch\n") ;
      printf("************************************************************************\n") ;
      exit(0) ;
    }
    channelParams->deltaTau_ns = delSamp * dTau_ns ;
  }

  numDelay                = 1 + (int) (channelParams->maxDelay_ns / channelParams->deltaTau_ns) ;
  channelParams->numDelay = numDelay ;

  ALLOCATE_VECTOR( channelParams->delays_ns ,      numDelay , float ) ;
  ALLOCATE_VECTOR( channelParams->delays_samples , numDelay , int ) ;

  channelParams->numDoppler = 1 ;
  dopplerParams = (DopplerParamsStruct*)malloc( sizeof( DopplerParamsStruct ) ) ;
  channelParams->dopplerParams = dopplerParams ;

  ALLOCATE_VECTOR( dopplerParams->tapEnergy ,       numDelay , float ) ;
  ALLOCATE_VECTOR( dopplerParams->sigmaChannelTap , numDelay , float ) ;
  ALLOCATE_VECTOR( dopplerParams->tapFlag ,         numDelay , boolean ) ;

  dopplerParams->numCluster = 0 ;
  dopplerParams->clusterParams = NULL ;

  ReadFloat( fieldName , "dopplerBW_Hz",     channelFileName , &(dopplerParams->dopplerBW_Hz) ) ;
  ReadFloat( fieldName , "dopplerOffset_Hz", channelFileName , &(dopplerParams->dopplerOffset_Hz) ) ;

  if ( numDelay == 1 ) {

    channelParams->delays_ns[0]        = 0.0f ;
    channelParams->delays_samples[0]   = 0 ;
    dopplerParams->tapEnergy[0]        = 1.0f ;
    dopplerParams->sigmaChannelTap[0]  = (float) sqrt( 0.5 ) ;
    dopplerParams->tapFlag[0]          = T ;

  } else {

    z = (channelParams->rmsDelaySpread_ns * channelParams->rmsDelaySpread_ns)
            / (channelParams->deltaTau_ns * channelParams->deltaTau_ns) ;
    p                 = (2.0*z + 1.0 + sqrt(8.0*z + 1)) / (2.0*(z - 1.0)) ;
    if ( p >= 1.0 ) p = (2.0*z + 1.0 - sqrt(8.0*z + 1)) / (2.0*(z - 1.0)) ;
    assert( (p > 0.0) && (p < 1.0) ) ;
    x1 = (1.0 - p) / (1.0 - pow( p , channelParams->numDelay )) ;
    for ( kTap = 0 ; kTap < channelParams->numDelay ; kTap++ ) {
      channelParams->delays_ns[kTap]        = kTap * channelParams->deltaTau_ns ;
      channelParams->delays_samples[kTap]   = kTap * delSamp ;
      dopplerParams->tapEnergy[kTap]        = (float) x1 ;
      dopplerParams->sigmaChannelTap[kTap]  = (float) sqrt( 0.5*x1 ) ;
      x1                                   *= p ;
      dopplerParams->tapFlag[kTap]          = T ;
    }

  }

  // Test Code
  x1 = x2 = 0.0 ;
  for ( kTap = 0 ; kTap < channelParams->numDelay ; kTap++ ) {
    a = 2.0 * dopplerParams->sigmaChannelTap[kTap] * dopplerParams->sigmaChannelTap[kTap] ;
    x1 += a ;
    x2 += channelParams->delays_ns[kTap] * channelParams->delays_ns[kTap] * a ;
  }
  x2 = sqrt(x2) ;

  dopplerParams->rxSqrtRho_I = NULL ;
  dopplerParams->rxSqrtRho_Q = NULL ;

  dopplerParams->txSqrtRho_I = NULL ;
  dopplerParams->txSqrtRho_Q = NULL ;

  dopplerParams->txRho_I     = NULL ;
  dopplerParams->txRho_Q     = NULL ;
  dopplerParams->rxRho_I     = NULL ;
  dopplerParams->rxRho_Q     = NULL ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InitEandV_State()                                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitEandV_State(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {


  DopplerStateStruct         *dopplerState ;
  DopplerParamsStruct        *dopplerParams ;

  assert( channelParams->numDoppler == 1 ) ;

  dopplerParams = channelParams->dopplerParams ;

  dopplerState  = (DopplerStateStruct*)malloc( sizeof( DopplerStateStruct ) ) ;
  channelState->dopplerState = dopplerState ;

  ALLOCATE_3D_MATRIX( dopplerState->h_iid_I , channelParams->numRxAntenna ,
                                              channelParams->numTxAntenna ,
                                              channelParams->numDelay , float ) ;
  ALLOCATE_3D_MATRIX( dopplerState->h_iid_Q , channelParams->numRxAntenna ,
                                              channelParams->numTxAntenna ,
                                              channelParams->numDelay , float ) ;

  dopplerState->h_I = dopplerState->h_iid_I ;
  dopplerState->h_Q = dopplerState->h_iid_Q ;

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// GetVinkoParams()                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////

void GetVinkoParams(
  char                       *channelFileName ,
  char                       *fieldName ,
  ChannelParamsStruct        *channelParams
) {

  char                        fieldNameX[100], lineName[100], xChar[2] ;
  int                         numTxAntenna, numRxAntenna, numDelay ;
  int                         kx, kTap, kDoppler, kCluster, numCluster ;
  float                      *relEnergy_dB, *energy ;
  float                       dTau_ns ;
  double                      txPAS[NUM_PHI], rxPAS[NUM_PHI] ;
  double                      phi, xx, xz, zA, zD, xA, xD, AoA, AoD, sA, sD, ex ;
  double                      firstTapEnergy, totalEnergy, energy_LOS, RicianFactorK ;
  boolean                     txCorrelationFlag, rxCorrelationFlag ;
  DopplerParamsStruct        *dopplerParams ;
  ClusterParamsStruct        *clusterParams ;

  assert( channelParams->channelType == VINKO ) ;

  // Get local copy of parameters

  numTxAntenna = channelParams->numTxAntenna ;
  numRxAntenna = channelParams->numRxAntenna ;

  channelParams->txCorrelationFlag = txCorrelationFlag = ( numTxAntenna > 1 ) ? T : F ;
  channelParams->rxCorrelationFlag = rxCorrelationFlag = ( numRxAntenna > 1 ) ? T : F ;

  // Read Delay Vector

  ReadInt(        fieldName , "numDelay",   channelFileName , "decimal" , &numDelay ) ;
  channelParams->numDelay = numDelay ;

  ALLOCATE_VECTOR( channelParams->delays_ns ,      numDelay , float ) ;

  ReadMultiFloat( fieldName , "delays_ns" , channelFileName , numDelay , channelParams->delays_ns ) ;

  // Read Rician K-factor in dB value and AoD_deg_LOS (AoA_deg_LOS) in degree
  ReadInt(   fieldName , "LOS_Flag", channelFileName , "decimal" , (int*)&(channelParams->LOS_Flag) ) ;
  if ( channelParams->LOS_Flag == T ) {
    ReadFloat( fieldName , "RicianFactorK_dB", channelFileName , &(channelParams->RicianFactorK_dB) ) ;
    ReadFloat( fieldName , "AoA_deg_LOS", channelFileName , &(channelParams->AoA_deg_LOS) ) ;
    ReadFloat( fieldName , "AoD_deg_LOS", channelFileName , &(channelParams->AoD_deg_LOS) ) ;
  }

  // For time domain channels, we align the delays with analog samples

  if ( channelParams->freqDomainFlag == T ) {
    channelParams->maxSamplingError_nsec = 0.0f ;
    channelParams->delays_samples        = NULL ;
  } else {
    ALLOCATE_VECTOR( channelParams->delays_samples , numDelay , int ) ;
    dTau_ns = 1000.0f / channelParams->sampleRate_Msps ;
    for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
      kx     = (int) (0.5f + (channelParams->delays_ns[kTap] / dTau_ns) ) ;
      if ( fabs(channelParams->delays_ns[kTap] - kx * dTau_ns) > channelParams->maxSamplingError_nsec ) {
        printf("\n") ;
        printf("****************************************************************\n") ;
        printf("Channel Parameter Error: analog Sample Rate - deltaTau missmatch\n") ;
        printf("****************************************************************\n") ;
        exit(0) ;
      }
      channelParams->delays_samples[kTap] = kx ;
      channelParams->delays_ns[kTap]      = kx * dTau_ns ;
    }
  }

  // Read Cluster Params for different Doppler models

  ReadInt( fieldName , "numDoppler",  channelFileName , "decimal" , &(channelParams->numDoppler) ) ;

  ALLOCATE_VECTOR( channelParams->dopplerParams , channelParams->numDoppler , DopplerParamsStruct ) ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;

    xChar[0] = (char) (kDoppler + 48) ; xChar[1] = '\0' ;
    strcpy( fieldNameX , fieldName ) ;
    strcat( fieldNameX , "_" ) ;
    strcat( fieldNameX , xChar ) ;

    ReadFloat( fieldNameX , "dopplerBW_Hz",     channelFileName , &(dopplerParams->dopplerBW_Hz) ) ;
    ReadFloat( fieldNameX , "dopplerOffset_Hz", channelFileName , &(dopplerParams->dopplerOffset_Hz) ) ;

    ReadInt(       fieldNameX , "numCluster",  channelFileName , "decimal" , &numCluster ) ;
    dopplerParams->numCluster = numCluster ;

    ALLOCATE_VECTOR( dopplerParams->clusterParams , dopplerParams->numCluster , ClusterParamsStruct ) ;

    for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {

      clusterParams = &(dopplerParams->clusterParams[kCluster]) ;
      xChar[0] = (char) (kCluster + 48) ; xChar[1] = '\0' ;

      ALLOCATE_VECTOR( clusterParams->relEnergy_dB , numDelay , float ) ;
      ALLOCATE_VECTOR( clusterParams->AoA_deg ,      numDelay , float ) ;
      ALLOCATE_VECTOR( clusterParams->AoD_deg ,      numDelay , float ) ;
      ALLOCATE_VECTOR( clusterParams->energy ,       numDelay , float ) ;

      strcpy( lineName , "minDelayIndex_") ; strcat( lineName , xChar ) ;
      ReadInt( fieldNameX , lineName ,  channelFileName , "decimal" , &(clusterParams->minDelayIndex) ) ;
      strcpy( lineName , "maxDelayIndex_") ; strcat( lineName , xChar ) ;
      ReadInt( fieldNameX , lineName ,  channelFileName , "decimal" , &(clusterParams->maxDelayIndex) ) ;
      assert( clusterParams->minDelayIndex <= clusterParams->maxDelayIndex ) ;
      assert( clusterParams->maxDelayIndex <= channelParams->numDelay ) ;

      strcpy( lineName , "relEnergy_dB_") ; strcat( lineName , xChar ) ;
      ReadMultiFloat( fieldNameX , lineName , channelFileName ,
                      clusterParams->maxDelayIndex - clusterParams->minDelayIndex + 1 ,
                    &(clusterParams->relEnergy_dB[clusterParams->minDelayIndex]) ) ;

      strcpy( lineName , "AoA_AS_deg_") ; strcat( lineName , xChar ) ;
      ReadFloat( fieldNameX , lineName , channelFileName , &(clusterParams->AoA_AS_deg) ) ;
      strcpy( lineName , "AoD_AS_deg_") ; strcat( lineName , xChar ) ;
      ReadFloat( fieldNameX , lineName , channelFileName , &(clusterParams->AoD_AS_deg) ) ;

      strcpy( lineName , "AoA_deg_") ; strcat( lineName , xChar ) ;
      ReadMultiFloat( fieldName , lineName , channelFileName ,
                      clusterParams->maxDelayIndex - clusterParams->minDelayIndex + 1 ,
                    &(clusterParams->AoA_deg[clusterParams->minDelayIndex]) ) ;

      strcpy( lineName , "AoD_deg_") ; strcat( lineName , xChar ) ;
      ReadMultiFloat( fieldName , lineName , channelFileName ,
                      clusterParams->maxDelayIndex - clusterParams->minDelayIndex + 1 ,
                    &(clusterParams->AoD_deg[clusterParams->minDelayIndex]) ) ;

    } // End of Cluster loop

  }   // End of Doppler loop

  // Calculate LOS energy if LOS is true

  RicianFactorK =( channelParams->LOS_Flag == T ) ? pow( 10.0 , 0.1* channelParams->RicianFactorK_dB ) : 0.0 ;

  // Calculate Per Cluster Tap Energies

  totalEnergy = firstTapEnergy = energy_LOS = 0.0 ;
  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    numCluster    = dopplerParams->numCluster ;

    ALLOCATE_VECTOR( dopplerParams->tapFlag ,         numDelay , boolean ) ;
    ALLOCATE_VECTOR( dopplerParams->tapEnergy ,       numDelay , float ) ;
    ALLOCATE_VECTOR( dopplerParams->sigmaChannelTap , numDelay , float ) ;

    for ( kTap = 0 ; kTap < numDelay ; kTap++ ) dopplerParams->tapFlag[kTap] = F ;

    for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {

      clusterParams = &(dopplerParams->clusterParams[kCluster]) ;
      relEnergy_dB  = clusterParams->relEnergy_dB ;
      energy        = clusterParams->energy ;
      for ( kTap = 0 ; kTap < numDelay ; kTap++ ) energy[kTap] = 0.0f ;

      if ( clusterParams->minDelayIndex == 0 ) {
        kx = 1 ;
        dopplerParams->tapFlag[0] = T ;
        xz            =  pow( 10.0 , 0.1*relEnergy_dB[0] ) ;
        totalEnergy  +=  xz ;
        energy_LOS   += RicianFactorK * xz ;
        energy[0]     = (float) xz ;
      } else {
        kx = clusterParams->minDelayIndex ;
      }

      for ( kTap = kx ; kTap <= clusterParams->maxDelayIndex ; kTap++ ) {
        dopplerParams->tapFlag[kTap] = T ;
        xz            = pow( 10.0 , 0.1*relEnergy_dB[kTap] ) ;
        energy[kTap]  = (float) xz ;
        totalEnergy  += xz ;
      }

      firstTapEnergy += (double) energy[0] ;

    }   // End of Cluster loop

  }   // End of Doppler loop

  // Normalization all the taps

  ex = 1.0 / (totalEnergy + energy_LOS) ;
  energy_LOS *= ex ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {
    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {
      clusterParams = &(dopplerParams->clusterParams[kCluster]) ;
      energy        = clusterParams->energy ;
      for ( kTap = clusterParams->minDelayIndex ; kTap <= clusterParams->maxDelayIndex ; kTap++ ) {
        energy[kTap] *= (float) ex ;
  } } }

  // Calculate Tap StDev - aggregate over Clusters

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {
    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
      xx = 0.0 ;
      for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {
        clusterParams = &(dopplerParams->clusterParams[kCluster]) ;
        if (    (kTap >= clusterParams->minDelayIndex)
             && (kTap <= clusterParams->maxDelayIndex) ) {
          xx += clusterParams->energy[kTap] ;
        }
      }
      dopplerParams->tapEnergy[kTap]       = (float) xx ;
      dopplerParams->sigmaChannelTap[kTap] = (float) sqrt( 0.5*xx ) ;
    }
  }

  channelParams->sigma_LOS = (float) sqrt( 0.5*energy_LOS ) ;

  // Caluculate LOS phase matrix if LOS is true

  if ( channelParams->LOS_Flag == T ) {

    double  delTxD, delRxD, delDeg ;
    int     kRx, kTx ;

    ALLOCATE_MATRIX( channelParams->h_LOS_I , numRxAntenna , numTxAntenna , float ) ;
    ALLOCATE_MATRIX( channelParams->h_LOS_Q , numRxAntenna , numTxAntenna , float ) ;

    for ( kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        delTxD = (double) (kTx*channelParams->txAntennaSpacing_cm)*cos(TWO_PI*channelParams->AoD_deg_LOS/360.0) ;  // in the unit of cm
        delRxD = (double) (kRx*channelParams->rxAntennaSpacing_cm)*cos(TWO_PI*channelParams->AoA_deg_LOS/360.0) ;
        delDeg = (double) TWO_PI*(delRxD-delTxD)/channelParams->lambda_cm ;
        channelParams->h_LOS_I[kRx][kTx] = channelParams->sigma_LOS * (float) cos(delDeg) ;
        channelParams->h_LOS_Q[kRx][kTx] = channelParams->sigma_LOS * (float) sin(delDeg) ;
      }
    }

  }

  // For SISO, we are done!

  if ( (txCorrelationFlag == F) && (rxCorrelationFlag == F) ) {
    for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {
      dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
      dopplerParams->txSqrtRho_I = NULL ;
      dopplerParams->txSqrtRho_Q = NULL ;
      dopplerParams->rxSqrtRho_I = NULL ;
      dopplerParams->rxSqrtRho_Q = NULL ;
    }
    return ;
  }

  ////////////////
  //  Not SISO  //
  ////////////////

  // Allocate correlation matrices

  kx = (numRxAntenna > numTxAntenna) ? numRxAntenna : numTxAntenna ;

  // Calculate Correlation Matrices

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;

    if ( txCorrelationFlag == T ) {
      ALLOCATE_3D_MATRIX( dopplerParams->txRho_I ,     numDelay , numTxAntenna , numTxAntenna , float ) ;
      ALLOCATE_3D_MATRIX( dopplerParams->txRho_Q ,     numDelay , numTxAntenna , numTxAntenna , float ) ;
      ALLOCATE_3D_MATRIX( dopplerParams->txSqrtRho_I , numDelay , numTxAntenna , numTxAntenna , float ) ;
      ALLOCATE_3D_MATRIX( dopplerParams->txSqrtRho_Q , numDelay , numTxAntenna , numTxAntenna , float ) ;
    } else {
      dopplerParams->txRho_I     = NULL ;
      dopplerParams->txRho_Q     = NULL ;
      dopplerParams->txSqrtRho_I = NULL ;
      dopplerParams->txSqrtRho_Q = NULL ;
    }

    if ( rxCorrelationFlag == T ) {
      ALLOCATE_3D_MATRIX( dopplerParams->rxRho_I ,     numDelay , numRxAntenna , numRxAntenna , float ) ;
      ALLOCATE_3D_MATRIX( dopplerParams->rxRho_Q ,     numDelay , numRxAntenna , numRxAntenna , float ) ;
      ALLOCATE_3D_MATRIX( dopplerParams->rxSqrtRho_I , numDelay , numRxAntenna , numRxAntenna , float ) ;
      ALLOCATE_3D_MATRIX( dopplerParams->rxSqrtRho_Q , numDelay , numRxAntenna , numRxAntenna , float ) ;
    } else {
      dopplerParams->rxRho_I     = NULL ;
      dopplerParams->rxRho_Q     = NULL ;
      dopplerParams->rxSqrtRho_I = NULL ;
      dopplerParams->rxSqrtRho_Q = NULL ;
    }

    for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {

      if ( dopplerParams->tapFlag[kTap] == T ) {

        for ( kx = 0 ; kx < NUM_PHI ; kx++ ) txPAS[kx] = rxPAS[kx] = 0.0 ;
        sA = sD = 0.0 ;

        for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {
          clusterParams = &(dopplerParams->clusterParams[kCluster]) ;
          if (    (kTap >= clusterParams->minDelayIndex)
               && (kTap <= clusterParams->maxDelayIndex) ) {

            zA  = clusterParams->energy[kTap] / ((double) clusterParams->AoA_AS_deg) ;
            zD  = clusterParams->energy[kTap] / ((double) clusterParams->AoD_AS_deg) ;
            AoA = clusterParams->AoA_deg[kTap] ;
            AoD = clusterParams->AoD_deg[kTap] ;

            xA  = -1.414213562 / ((double) clusterParams->AoA_AS_deg) ;
            xD  = -1.414213562 / ((double) clusterParams->AoD_AS_deg) ;

            for ( kx = 0 ; kx < NUM_PHI ; kx++ ) {

              phi = DEL_PHI_DEG*kx - AoD ;
              if ( phi >  180.0 ) phi -= 360.0 ;
              if ( phi < -180.0 ) phi += 360.0 ;
              xz         = zD * exp( xD * fabs( phi ) ) ;
              sD        += xz ;
              txPAS[kx] += xz ;

              phi = DEL_PHI_DEG*kx - AoA ;
              if ( phi >  180.0 ) phi -= 360.0 ;
              if ( phi < -180.0 ) phi += 360.0 ;
              xz         = zA * exp( xA * fabs( phi ) ) ;
              sA        += xz ;
              rxPAS[kx] += xz ;

            }  // End of k loop

        } }    // End of kCluster loop

        sA = 1.0 / sA ;
        sD = 1.0 / sD ;
        for ( kx = 0 ; kx < NUM_PHI ; kx++ ) {
          txPAS[kx] *= sD ;
          rxPAS[kx] *= sA ;
        }

        if ( rxCorrelationFlag == T ) {

          CalcCorrelationMatrix( numRxAntenna ,
                                 channelParams->rxAntennaSpacing_cm ,
                                 channelParams->lambda_cm ,
                                 dopplerParams->rxRho_I[kTap] ,
                                 dopplerParams->rxRho_Q[kTap] ,
                                 rxPAS ) ;

          MatSqRoot_Flt(         numRxAntenna ,
                                 dopplerParams->rxRho_I[kTap] ,
                                 dopplerParams->rxRho_Q[kTap] ,
                                 dopplerParams->rxSqrtRho_I[kTap] ,
                                 dopplerParams->rxSqrtRho_Q[kTap] ) ;

        }  // End of if ( channelParams->rxCorrelationFlag == T ) {

        if ( channelParams->txCorrelationFlag == T ) {

          CalcCorrelationMatrix( numTxAntenna ,
                                 channelParams->txAntennaSpacing_cm ,
                                 channelParams->lambda_cm ,
                                 dopplerParams->txRho_I[kTap] ,
                                 dopplerParams->txRho_Q[kTap] ,
                                 txPAS ) ;

          MatSqRoot_Flt(         numTxAntenna ,
                                 dopplerParams->txRho_I[kTap] ,
                                 dopplerParams->txRho_Q[kTap] ,
                                 dopplerParams->txSqrtRho_I[kTap] ,
                                 dopplerParams->txSqrtRho_Q[kTap] ) ;

        }  // End of if ( channelParams->txCorrelationFlag == T ) {

    } }  // End of kTap Loop

  }  // End of kDoppler loop


}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CalcRMS_DelaySpread()                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////

float CalcRMS_DelaySpread(
  ChannelParamsStruct        *channelParams
) {

  int                         kTap, kDoppler, kCluster ;
  float                      *delays_ns ;
  double                      x0, x1, x2, xx ;
  DopplerParamsStruct        *dopplerParams ;
  ClusterParamsStruct        *clusterParams ;

  delays_ns = channelParams->delays_ns ;

  x0 = x1 = x2 = 0.0 ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;

    for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {

      clusterParams = &(dopplerParams->clusterParams[kCluster]) ;

      for ( kTap = clusterParams->minDelayIndex ; kTap <= clusterParams->maxDelayIndex ; kTap++ ) {
        xx  = clusterParams->energy[kTap] ;
        x0 += xx ;
        x1 +=                 delays_ns[kTap]*xx ;
        x2 += delays_ns[kTap]*delays_ns[kTap]*xx ;
      }

    }

  } // End of Doppler Loop

  x2 = sqrt( x2 - x1*x1 ) ;

  return (float) x2 ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CalcChannel_DelaySpread()                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////

void CalcChannel_DelaySpreads(
  float                     **delaySpreads ,
  float                     **energies ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kRx, kTx, kx ;
  float                      *h_I, *h_Q, *delays_ns ;
  double                      x0, x1, x2, xx ;

  delays_ns = channelParams->delays_ns ;

  for (   kRx = 0 ; kRx < channelParams->numRxAntenna ; kRx++ ) {
    for ( kTx = 0 ; kTx < channelParams->numRxAntenna ; kTx++ ) {
      h_I = channelState->h_I[kRx][kTx] ;
      h_Q = channelState->h_Q[kRx][kTx] ;
      x0 = x1 = x2 = 0.0 ;
      for ( kx = 0 ; kx < channelParams->numDelay ; kx++ ) {
        xx  = h_I[kx]*h_I[kx] + h_Q[kx]*h_Q[kx] ;
        x0 += xx ;
        x1 += delays_ns[kx]*xx ;
        x2 += delays_ns[kx]*delays_ns[kx]*xx ;
      }
      x1 /= x0 ;
      energies[kRx][kTx]     = (float) x0 ;
      delaySpreads[kRx][kTx] = (float) sqrt( x2/x0 - x1*x1 ) ;
    }
  }

  return ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CalcCorrelationMatrix()                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

void CalcCorrelationMatrix(
  int                         numAntenna ,
  float                       antennaSpacing_cm ,
  float                       lambda_cm ,
  float                     **correlation_I ,
  float                     **correlation_Q ,
  double                     *PAS
) {

  int                         k1, k2, kx, k ;
  double                      rho_I, rho_Q, phi, Dx, Dxx ;

  for ( k1 = 0 ; k1 < numAntenna ; k1++ ) {
    correlation_I[k1][k1] = 1.0f ;
    correlation_Q[k1][k1] = 0.0f ;
  }

  Dx = TWO_PI * antennaSpacing_cm / lambda_cm ;
  for ( kx = 1 ; kx < numAntenna ; kx++ ) {
    Dxx = kx*Dx ;
    rho_I = rho_Q = 0.0 ;
    for ( k = 0 , phi = 0.0 ; k < NUM_PHI ; k++, phi += DEL_PHI_RAD ) {
      rho_I += cos( Dxx * sin( phi ) ) * PAS[k] ;
      rho_Q += sin( Dxx * sin( phi ) ) * PAS[k] ;
    }
    for ( k1 = 0 ; k1 < numAntenna - kx ; k1++ ) {
      k2 = k1 + kx ;
      correlation_I[k1][k2] = (float)   rho_I ;
      correlation_Q[k1][k2] = (float)   rho_Q ;
      correlation_I[k2][k1] = (float)   rho_I ;
      correlation_Q[k2][k1] = (float) (-rho_Q) ;
    }
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InitVinkoState()                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////

void InitVinkoState(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         numTxAntenna, numRxAntenna, numDelay ;
  int                         kDoppler ;
  DopplerParamsStruct        *dopplerParams ;
  DopplerStateStruct         *dopplerState ;

  assert( channelParams->channelType == VINKO ) ;

  // Get local copy of parameters

  numTxAntenna = channelParams->numTxAntenna ;
  numRxAntenna = channelParams->numRxAntenna ;
  numDelay     = channelParams->numDelay ;

  assert( numTxAntenna > 0 ) ;
  assert( numRxAntenna > 0 ) ;



  // Allocate Doppler State Memory

  ALLOCATE_VECTOR( channelState->dopplerState , channelParams->numDoppler , DopplerStateStruct ) ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    dopplerState  = &(channelState->dopplerState[kDoppler]) ;

    ALLOCATE_3D_MATRIX( dopplerState->h_iid_I , numRxAntenna , numTxAntenna , numDelay , float ) ;
    ALLOCATE_3D_MATRIX( dopplerState->h_iid_Q , numRxAntenna , numTxAntenna , numDelay , float ) ;

    if ( channelParams->MIMO_Flag == T ) {
      ALLOCATE_3D_MATRIX( dopplerState->h_I ,   numRxAntenna , numTxAntenna , numDelay , float ) ;
      ALLOCATE_3D_MATRIX( dopplerState->h_Q ,   numRxAntenna , numTxAntenna , numDelay , float ) ;
    } else {
      dopplerState->h_I = dopplerState->h_iid_I ;
      dopplerState->h_Q = dopplerState->h_iid_Q ;
    }

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Wrapup Code                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Wrapup_ChannelTD()                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void Wrapup_Channel (
  ChannelStateStruct               *channelState ,
  ChannelParamsStruct              *channelParams
) {

  FreeChannelState ( channelState , channelParams ) ;
  FreeChannelParams (               channelParams ) ;

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FreeChannelState()                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void FreeChannelState (
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kDoppler ;
  DopplerStateStruct         *dopplerState ;


  FREE_3D_MATRIX( channelState->freqDomainChannel.H_I ) ;
  FREE_3D_MATRIX( channelState->freqDomainChannel.H_Q ) ;
  FREE_3D_MATRIX( channelState->freqDomainChannel.H_E ) ;
  FREE_MATRIX(    channelState->freqDomainChannel.AvgE ) ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerState = &(channelState->dopplerState[kDoppler]) ;

    // Due to pointer copying, these channel pointers must be freed in this order
    FREE_3D_MATRIX( dopplerState->h_iid_I ) ;
    FREE_3D_MATRIX( dopplerState->h_iid_Q ) ;
    if ( (channelParams->channelType == VINKO) && (channelParams->MIMO_Flag == T) ) {
      FREE_3D_MATRIX( dopplerState->h_I ) ;
      FREE_3D_MATRIX( dopplerState->h_Q ) ;
    } else {
      dopplerState->h_I = NULL ;
      dopplerState->h_Q = NULL ;
    }

  }

  switch ( channelParams->channelType ) {
    case AWGN_CH :
      FREE_3D_MATRIX( channelState->h_I ) ;
      FREE_3D_MATRIX( channelState->h_Q ) ;
      break ;
    case E_AND_V :
    case VINKO :
      if ( channelParams->dopplerFlag == T ) {
        FREE_3D_MATRIX( channelState->h_I ) ;
        FREE_3D_MATRIX( channelState->h_Q ) ;
      }
      break ;
    default :
      assert(0) ;
  }

  FREE_MATRIX( channelState->h_E ) ;

  FREE_VECTOR( channelState->dopplerState ) ;

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FreeChannelParams()                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void FreeChannelParams (
  ChannelParamsStruct       *channelParams
) {

  int                         kDoppler, kCluster ;
  DopplerParamsStruct        *dopplerParams ;
  ClusterParamsStruct        *clusterParams ;


  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;

    for ( kCluster = 0 ; kCluster < dopplerParams->numCluster ; kCluster++ ) {
      clusterParams = &(dopplerParams->clusterParams[kCluster]) ;
      FREE_VECTOR( clusterParams->relEnergy_dB ) ;
      FREE_VECTOR( clusterParams->AoA_deg ) ;
      FREE_VECTOR( clusterParams->AoD_deg ) ;
      FREE_VECTOR( clusterParams->energy ) ;
    }

    FREE_VECTOR(    dopplerParams->clusterParams ) ;
    FREE_VECTOR(    dopplerParams->tapFlag ) ;
    FREE_VECTOR(    dopplerParams->tapEnergy ) ;
    FREE_VECTOR(    dopplerParams->sigmaChannelTap ) ;

    FREE_3D_MATRIX( dopplerParams->txRho_I ) ;
    FREE_3D_MATRIX( dopplerParams->txRho_Q ) ;
    FREE_3D_MATRIX( dopplerParams->rxRho_I ) ;
    FREE_3D_MATRIX( dopplerParams->rxRho_Q ) ;

    FREE_3D_MATRIX( dopplerParams->txSqrtRho_I ) ;
    FREE_3D_MATRIX( dopplerParams->txSqrtRho_Q ) ;
    FREE_3D_MATRIX( dopplerParams->rxSqrtRho_I ) ;
    FREE_3D_MATRIX( dopplerParams->rxSqrtRho_Q ) ;

  }

  FREE_VECTOR( channelParams->dopplerParams ) ;

  FREE_VECTOR( channelParams->delays_ns ) ;
  FREE_VECTOR( channelParams->delays_samples ) ;

  FREE_MATRIX( channelParams->cs ) ;
  FREE_MATRIX( channelParams->sn ) ;

  FREE_MATRIX( channelParams->h_LOS_I ) ;
  FREE_MATRIX( channelParams->h_LOS_Q ) ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Generate Impulse Response                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// GenerateImpulseResponse()                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////

void GenerateImpulseResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kTx, kRx, kDoppler, kx, kTap, numDelay ;
  int                         numRxAntenna, numTxAntenna ;
  float                      *h_I, *h_Q, *sigmaChTap, xx ;
  boolean                    *tapFlag ;
  DopplerParamsStruct        *dopplerParams ;
  DopplerStateStruct         *dopplerState ;


  switch ( channelParams->channelType ) {

    case AWGN_CH :
      return ;

    case E_AND_V :
    case VINKO :
      break;

    default :
      assert(0) ;
  }

  numRxAntenna      = channelParams->numRxAntenna ;
  numTxAntenna      = channelParams->numTxAntenna ;
  numDelay          = channelParams->numDelay ;

  if ( channelParams->freqDomainFlag == F ) {
    for ( kTx = 0 ; kTx < numTxAntenna ; kTx++  ) {
      for ( kx = 0 ; kx < channelParams->maxDelay_samples ; kx++ ) {
        channelState->xBuffer_I[kTx][kx] = 0.0f ;
        channelState->xBuffer_Q[kTx][kx] = 0.0f ;
      }
    }
  }

  ResetDopplerState ( time_muSec , channelState , channelParams ) ;

  // Doppler loop - Generate iid channels
  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    dopplerState  = &(channelState->dopplerState[kDoppler]) ;
    sigmaChTap    = dopplerParams->sigmaChannelTap ;
    tapFlag       = dopplerParams->tapFlag ;

    for (   kRx = 0 ; kRx < channelParams->numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < channelParams->numTxAntenna ; kTx++ ) {
        h_I    = dopplerState->h_iid_I[kRx][kTx] ;
        h_Q    = dopplerState->h_iid_Q[kRx][kTx] ;
        GaussVectorRNG( numDelay , h_I , channelState->chRngState ) ;
        GaussVectorRNG( numDelay , h_Q , channelState->chRngState ) ;
        for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
          h_I[kTap] *= sigmaChTap[kTap] ;
          h_Q[kTap] *= sigmaChTap[kTap] ;
        }
    } }

    // For Vinko, calculate correlated channel from iid channel
    if ( channelParams->channelType == VINKO ) {
      for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
        if ( tapFlag[kTap] == T ) {
          CorrelateChannel( kDoppler , kTap , channelState , channelParams ) ;
    } } }

  } // End Doppler loop

  CalcTotalImpulseResponse( channelState , channelParams ) ;

  // Zero Buffer
  for ( kTx = 0 ; kTx < channelParams->numTxAntenna ; kTx++ ) {
    for ( kx = 0 ; kx < channelParams->maxDelay_samples ; kx++ ) {
      channelState->xBuffer_I[kTx][kx] = 0.0f ;
      channelState->xBuffer_Q[kTx][kx] = 0.0f ;
  } }

  // Power Normalization

  if ( channelParams->pwrNormFlag == T ) {

    int     kf ;
    double  xE ;
    float  *cs, *sn, xI, xQ ;

    xE = 0.0 ;
    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        h_I = channelState->h_I[kRx][kTx] ;
        h_Q = channelState->h_Q[kRx][kTx] ;
        for ( kf = 0 ; kf < channelParams->numSubCarrier ; kf++ ) {
          cs = channelParams->cs[kf] ;
          sn = channelParams->sn[kf] ;
          xI = xQ = 0.0f ;
          for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
            xI += cs[kTap]*h_I[kTap] + sn[kTap]*h_Q[kTap] ;
            xQ += cs[kTap]*h_Q[kTap] - sn[kTap]*h_I[kTap] ;
          }
          xE += xI*xI + xQ*xQ ;
        } // End of kf loop

    } }  // End of kRx-kTx loops

    xx = (float) sqrt( ((double) (numRxAntenna*numTxAntenna*channelParams->numSubCarrier)) / xE ) ;

    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        h_I = channelState->h_I[kRx][kTx] ;
        h_Q = channelState->h_Q[kRx][kTx] ;
        for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
          h_I[kTap] *= xx ;
          h_Q[kTap] *= xx ;
        }
    } }  // End of kRx-kTx loops

  }    // End of  if ( channelParams->pwrNormFlag == T ) {


  // Calculate Channel Energy
  for ( kRx = 0 ; kRx < numRxAntenna ; kRx++  ) {
    for ( kTx = 0 ; kTx < numTxAntenna ; kTx++  ) {
      h_I = channelState->h_I[kRx][kTx] ;
      h_Q = channelState->h_Q[kRx][kTx] ;
      xx = 0.0f ;
      for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
        xx += h_I[kTap]*h_I[kTap] + h_Q[kTap]*h_Q[kTap] ;
      }
      channelState->h_E[kRx][kTx] = xx ;
    }
  }


}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CalcTotalImpulseResponse()                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////

void CalcTotalImpulseResponse(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kTx, kRx, kDoppler, kTap, numDelay, numTxAntenna, numRxAntenna ;
  float                      *hTap_I, *hTap_Q, *hTapX_I, *hTapX_Q, cs, sn, xx ;
  boolean                    *tapFlag, dopplerOffsetFlag ;
  DopplerParamsStruct        *dopplerParams ;
  DopplerStateStruct         *dopplerState ;

  numDelay     = channelParams->numDelay ;
  numTxAntenna = channelParams->numTxAntenna ;
  numRxAntenna = channelParams->numRxAntenna ;

  // dopplerFlag == T when there are multiple Dopplers, or if there is a Doppler offset.
  // Otherwise, the channel is automatically passed through by equating pointers.

  if ( channelParams->dopplerFlag == T ) {

    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {

        hTap_I  = channelState->h_I[kRx][kTx] ;
        hTap_Q  = channelState->h_Q[kRx][kTx] ;

        for ( kTap = 0 ; kTap < numDelay ; kTap++ )
          hTap_I[kTap] = hTap_Q[kTap] = 0.0f ;

        for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

          dopplerState      = &(channelState->dopplerState[kDoppler]) ;
          dopplerParams     = &(channelParams->dopplerParams[kDoppler]) ;
          dopplerOffsetFlag =  dopplerParams->dopplerOffsetFlag ;
          tapFlag           =  dopplerParams->tapFlag ;

          hTapX_I = dopplerState->h_I[kRx][kTx] ;
          hTapX_Q = dopplerState->h_Q[kRx][kTx] ;

          for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {
            if ( tapFlag[kTap] == T ) {
              if ( dopplerParams->dopplerOffsetFlag == T ) {
                cs = dopplerState->cs[kTap] ; sn = dopplerState->sn[kTap] ;
                hTap_I[kTap] += cs*hTapX_I[kTap] - sn*hTapX_Q[kTap] ;
                hTap_Q[kTap] += cs*hTapX_Q[kTap] + sn*hTapX_I[kTap] ;
              } else {
                hTap_I[kTap] += hTapX_I[kTap] ;
                hTap_Q[kTap] += hTapX_Q[kTap] ;
              }
            }
          }    // End of kTap loop

        }      // End of kDoppler loop

    } }        // End of kRx-kTx loops

  }            // End of if ( channelParams->dopplerFlag == T ) {

  // The 1st tap could have LOS component
  if ( channelParams->LOS_Flag == T ) {
    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        channelState->h_I[kRx][kTx][0] += channelParams->h_LOS_I[kRx][kTx];
        channelState->h_Q[kRx][kTx][0] += channelParams->h_LOS_Q[kRx][kTx];
      }
    }
  }

  // Calculate Total Energy
  for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
    for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {

      hTap_I  = channelState->h_I[kRx][kTx] ;
      hTap_Q  = channelState->h_Q[kRx][kTx] ;

      xx = 0.0 ;
      for ( kTap = 0 ; kTap < numDelay ; kTap++ )
        xx += hTap_I[kTap]*hTap_I[kTap] + hTap_Q[kTap]*hTap_Q[kTap] ;

      channelState->h_E[kRx][kTx] = xx ;

  } }  // End of kRx-kTx loops

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CorrelateChannel()                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void CorrelateChannel(
  int                         kDoppler ,
  int                         kTap ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kRx, kTx, kx, numRxAntenna, numTxAntenna ;
  float                    ***hxx_I, ***hxx_Q, **sqrtRho_I , **sqrtRho_Q ;
  float                       hz_I[10][10], hz_Q[10][10] ;
  float                       xI, xQ ;
  DopplerStateStruct         *dopplerState ;
  DopplerParamsStruct        *dopplerParams ;

  if ( channelParams->channelType != VINKO ) return ;

  if ( (channelParams->txCorrelationFlag == F) && (channelParams->rxCorrelationFlag == F) ) return ;

  dopplerState  = &(channelState->dopplerState[kDoppler]) ;
  dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;

  hxx_I = dopplerState->h_iid_I ;
  hxx_Q = dopplerState->h_iid_Q ;

  numRxAntenna = channelParams->numRxAntenna ;
  numTxAntenna = channelParams->numTxAntenna ;

  if ( channelParams->txCorrelationFlag == T ) {

    sqrtRho_I = dopplerParams->txSqrtRho_I[kTap] ;
    sqrtRho_Q = dopplerParams->txSqrtRho_Q[kTap] ;

    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        xI = xQ = 0.0 ;
        for ( kx = kTx ; kx < numTxAntenna ; kx++) {
          xI += hxx_I[kRx][kx][kTap]*sqrtRho_I[kTx][kx] - hxx_Q[kRx][kx][kTap]*sqrtRho_Q[kTx][kx] ;
          xQ += hxx_I[kRx][kx][kTap]*sqrtRho_Q[kTx][kx] + hxx_Q[kRx][kx][kTap]*sqrtRho_I[kTx][kx] ;
        }
        hz_I[kRx][kTx] = xI ;
        hz_Q[kRx][kTx] = xQ ;
    } }

  } else {

    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      hz_I[kRx][0] = hxx_I[kRx][0][kTap] ;
      hz_Q[kRx][0] = hxx_Q[kRx][0][kTap] ;
    }

  }  // end of  if ( txCorrelationFlag == T ) { ... } else { ...

  hxx_I = dopplerState->h_I ;
  hxx_Q = dopplerState->h_Q ;

  if ( channelParams->rxCorrelationFlag == T ) {

    sqrtRho_I = dopplerParams->rxSqrtRho_I[kTap] ;
    sqrtRho_Q = dopplerParams->rxSqrtRho_Q[kTap] ;

    for (   kRx = 0 ; kRx < numRxAntenna ; kRx++ ) {
      for ( kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
        xI = xQ = 0.0f ;
        for ( kx = kRx ; kx < numRxAntenna ; kx++) {
          xI += sqrtRho_I[kRx][kx]*hz_I[kx][kTx] - sqrtRho_Q[kRx][kx]*hz_Q[kx][kTx] ;
          xQ += sqrtRho_I[kRx][kx]*hz_Q[kx][kTx] + sqrtRho_Q[kRx][kx]*hz_I[kx][kTx] ;
        }
        hxx_I[kRx][kTx][kTap] = xI ;
        hxx_Q[kRx][kTx][kTap] = xQ ;
    } }

  } else {

    for (   kTx = 0 ; kTx < numTxAntenna ; kTx++ ) {
      hxx_I[0][kTx][kTap] = hz_I[0][kTx] ;
      hxx_Q[0][kTx][kTap] = hz_Q[0][kTx] ;
    }

  } // end of  if ( txCorrelationFlag == T ) { ... } else { ...

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Doppler Update functions                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// ResetDopplerState()                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ResetDopplerState (
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kDoppler, kTap ;
  float                       xx, minT ;
  DopplerParamsStruct        *dopplerParams ;
  DopplerStateStruct         *dopplerState ;

  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    dopplerState  = &(channelState->dopplerState[kDoppler]) ;

    minT = 1.0e+20f ;

    for ( kTap = 0 ; kTap < channelParams->numDelay ; kTap++ ) {
      if ( dopplerParams->tapFlag[kTap] == T ) {
        xx = time_muSec + dopplerParams->updateTime0_muSec * UniformRNG( channelState->chRngState ) ;
        dopplerState->nextUpdateTime0_muSec[kTap] = xx ;
        if ( xx < minT ) minT = xx ;
    } }

    if ( dopplerParams->dopplerOffsetFlag == T ) {
      for ( kTap = 0 ; kTap < channelParams->numDelay ; kTap++ ) {
        if ( dopplerParams->tapFlag[kTap] == T ) {
          dopplerState->cs[kTap] = (float) cos( dopplerParams->omega * time_muSec ) ;
          dopplerState->sn[kTap] = (float) sin( dopplerParams->omega * time_muSec ) ;
          xx = time_muSec + dopplerParams->updateTime1_muSec * UniformRNG( channelState->chRngState ) ;
          dopplerState->nextUpdateTime1_muSec[kTap] = xx ;
          if ( xx < minT ) minT = xx ;
      } }
    }

    dopplerState->minNextUpdateTime_muSec = minT ;

  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UpdateImpulseResponse()                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

boolean UpdateImpulseResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kTx, kRx, kDoppler, kTap, numDelay ;
  float                      *sigmaChTap, minT ;
  float                      *nextUpdateTime0_muSec, *nextUpdateTime1_muSec ;
  float                       surplusTime, updateTime0_muSec, beta ;
  double                      xx, x0, x1 ;
  boolean                    *tapFlag, flag ;
  DopplerParamsStruct        *dopplerParams ;
  DopplerStateStruct         *dopplerState ;
  RNG_StateStruct            *rngState ;

  flag = F ;

  switch ( channelParams->channelType ) {

    case AWGN_CH :
      return F ;

    case E_AND_V :
    case VINKO :
      break;

    default :
      assert(0) ;
  }

  numDelay = channelParams->numDelay ;
  rngState = channelState->chRngState ;

  // Doppler loop
  for ( kDoppler = 0 ; kDoppler < channelParams->numDoppler ; kDoppler++ ) {

    dopplerParams = &(channelParams->dopplerParams[kDoppler]) ;
    dopplerState  = &(channelState->dopplerState[kDoppler]) ;

    if ( time_muSec >= dopplerState->minNextUpdateTime_muSec ) {

      flag                  = T ;
      tapFlag               = dopplerParams->tapFlag ;
      sigmaChTap            = dopplerParams->sigmaChannelTap ;
      nextUpdateTime0_muSec = dopplerState->nextUpdateTime0_muSec ;
      nextUpdateTime1_muSec = dopplerState->nextUpdateTime1_muSec ;
      updateTime0_muSec     = dopplerParams->updateTime0_muSec ;
      beta                  = dopplerParams->beta ;
      minT                  = 1.0e+20f ;

      for ( kTap = 0 ; kTap < numDelay ; kTap++ ) {

        if ( tapFlag[kTap] == T ) {

          surplusTime = time_muSec - nextUpdateTime0_muSec[kTap] ;

          if ( surplusTime >= 0 ) {

            x0 = exp( - beta * (surplusTime + updateTime0_muSec) ) ;
            x1 = sqrt(1.0 - x0*x0) * sigmaChTap[kTap] ;

            for ( kRx = 0 ; kRx < channelParams->numRxAntenna ; kRx++ ) {
              for ( kTx = 0 ; kTx < channelParams->numTxAntenna ; kTx++ ) {
                dopplerState->h_iid_I[kRx][kTx][kTap]
                   = (float) (x0 * dopplerState->h_iid_I[kRx][kTx][kTap] + x1 * GaussianRNG( rngState )) ;
                dopplerState->h_iid_Q[kRx][kTx][kTap]
                   = (float) (x0 * dopplerState->h_iid_Q[kRx][kTx][kTap] + x1 * GaussianRNG( rngState )) ;
            } }

            nextUpdateTime0_muSec[kTap] = time_muSec + updateTime0_muSec ;

            CorrelateChannel( kDoppler , kTap , channelState , channelParams ) ;

          }  // End of  if ( (surplusTime = (time_muSec - nextUpdateTime0_muSec[kTap]) >= 0 ) {

          if ( minT > nextUpdateTime0_muSec[kTap] ) minT = nextUpdateTime0_muSec[kTap] ;

          if ( dopplerParams->dopplerOffsetFlag == T ) {

            if ( time_muSec >= nextUpdateTime1_muSec[kTap] ) {
              flag = T ;
              xx =  dopplerParams->omega * time_muSec ;
              dopplerState->cs[kTap] = (float) cos( xx ) ;
              dopplerState->sn[kTap] = (float) sin( xx ) ;
              nextUpdateTime1_muSec[kTap] = time_muSec + dopplerParams->updateTime1_muSec ;
            }

            if ( minT > nextUpdateTime1_muSec[kTap] ) minT = nextUpdateTime1_muSec[kTap] ;

          }  // end of if ( dopplerParams->dopplerOffsetFlag == T ) { ...

        }    // end of if ( tapFlag[kTap] == T ) {

      }      // end of kTap loop

    }        // end of if ( time_muSec >= dopplerState->minNextUpdateTime_muSec ) { ..

  } // End Doppler loop

  if ( flag == T ) CalcTotalImpulseResponse( channelState , channelParams ) ;

  return flag ;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Frequency Domain Channel                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// GenerateFreqResponse()                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

void GenerateFreqResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         numSubCarrier, numRxAntenna, numTxAntenna ;
  RNG_StateStruct            *rngState ;

  rngState = channelState->chRngState ;

  numSubCarrier = channelParams->numSubCarrier ;
  numRxAntenna  = channelParams->numRxAntenna ;
  numTxAntenna  = channelParams->numTxAntenna ;

  switch ( channelParams->channelType ) {

    case AWGN_CH :
      break ;

    case E_AND_V :
    case VINKO :

      GenerateImpulseResponse( time_muSec , channelState , channelParams ) ;
      CalcFreqResponse(                     channelState , channelParams ) ;

      break;

    default:
      assert(0) ;
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CalcFreqResponse()                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

void CalcFreqResponse(
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  int                         kTx, kRx, kf, k, numDelay, numSubCarrier ;
  float                      *h_I, *h_Q, *H_I, *H_Q, *H_E, xI, xQ, xx ;
  float                      *cs, *sn, AvgE ;
  FreqDomainChannelStruct    *freqDomainChannel ;


  numSubCarrier     =   channelParams->numSubCarrier ;
  freqDomainChannel = &(channelState->freqDomainChannel) ;

  numDelay = channelParams->numDelay ;

  for (   kRx = 0 ; kRx < channelParams->numRxAntenna ; kRx++ ) {
    for ( kTx = 0 ; kTx < channelParams->numTxAntenna ; kTx++ ) {

      H_I  = freqDomainChannel->H_I[kRx][kTx] ;
      H_Q  = freqDomainChannel->H_Q[kRx][kTx] ;
      H_E  = freqDomainChannel->H_E[kRx][kTx] ;
      AvgE = 0.0f ;

      h_I = channelState->h_I[kRx][kTx] ;
      h_Q = channelState->h_Q[kRx][kTx] ;

      for ( kf = 0 ; kf < numSubCarrier ; kf++ ) {

        cs = channelParams->cs[kf] ;
        sn = channelParams->sn[kf] ;

        xI = xQ = 0.0f ;
        for ( k = 0 ; k < numDelay ; k++ ) {
          xI += cs[k]*h_I[k] + sn[k]*h_Q[k] ;
          xQ += cs[k]*h_Q[k] - sn[k]*h_I[k] ;
        }

        H_I[kf]  = xI ;
        H_Q[kf]  = xQ ;
        xx       = xI*xI + xQ*xQ ;
        H_E[kf]  = xx ;
        AvgE    += xx ;

      } // End of kf loop

      freqDomainChannel->AvgE[kRx][kTx] = AvgE / (float) numSubCarrier ;

  } }  // End of kRx-kTx loops

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UpdateFreqResponse()                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateFreqResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  switch ( channelParams->channelType ) {
    case AWGN_CH :
      return ;
    case E_AND_V :
    case VINKO :
      break;
    default:
      assert(0) ;
  }

  if ( T == UpdateImpulseResponse( time_muSec , channelState , channelParams ) ) {

    CalcFreqResponse( channelState , channelParams ) ;

  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Generic Channel Functions                                                                     //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void GenerateChannelResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  if ( channelParams->freqDomainFlag == T ) {

    GenerateFreqResponse(    time_muSec , channelState , channelParams ) ;

  } else {

    GenerateImpulseResponse( time_muSec , channelState , channelParams ) ;

  }

}

void UpdateChannelResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) {

  if ( channelParams->freqDomainFlag == T ) {

    UpdateFreqResponse(      time_muSec , channelState , channelParams ) ;

  } else {

    UpdateImpulseResponse(   time_muSec , channelState , channelParams ) ;

  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Channel Input -> Output Functions                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// FreqDomainChannel()                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

void Channel(
  int                          numSample ,  // Only used for Time Domain
  float                      **xI ,         // Inputs
  float                      **xQ ,
  float                      **yI ,         // Outputs
  float                      **yQ ,
  float                        noiseStDev ,
  ChannelStateStruct          *channelState ,
  ChannelParamsStruct         *channelParams
) {

  if ( channelParams->freqDomainFlag == T ) {

    FreqDomainChannel(        xI , xQ ,
                              yI , yQ ,
                              noiseStDev ,
                              channelState ,
                              channelParams ) ;
  } else {

    TimeDomainChannel(        numSample ,
                              xI , xQ ,
                              yI , yQ ,
                              noiseStDev ,
                              channelState ,
                              channelParams ) ;
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FreqDomainChannel()                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

void FreqDomainChannel(
  float                      **xI ,     // Inputs
  float                      **xQ ,
  float                      **yI ,     // Outputs
  float                      **yQ ,
  float                        noiseStDev ,
  ChannelStateStruct          *channelState ,
  ChannelParamsStruct         *channelParams
) {

  int                          kf, kTx, kRx ;
  int                          numTxAnt , numRxAnt ;
  float                     ***H_I, ***H_Q, *HxI, *HxQ, *xxI, *xxQ, *yyI, *yyQ ;
  RNG_StateStruct             *rngState ;


  rngState = channelState->noiseRngState ;

  numTxAnt = channelParams->numTxAntenna ;
  numRxAnt = channelParams->numRxAntenna ;

  H_I = channelState->freqDomainChannel.H_I ;
  H_Q = channelState->freqDomainChannel.H_Q ;

  for ( kRx = 0 ; kRx < numRxAnt ; kRx++ ) {

    yyI = yI[kRx] ;  yyQ = yQ[kRx] ;

    GaussVectorRNG( channelParams->numSubCarrier , yyI , rngState ) ;
    GaussVectorRNG( channelParams->numSubCarrier , yyQ , rngState ) ;

    for ( kf = 0 ; kf < channelParams->numSubCarrier ; kf++ ) {
      yyI[kf] *= noiseStDev ;
      yyQ[kf] *= noiseStDev ;
    }

    for ( kTx = 0 ; kTx < numTxAnt ; kTx++ ) {
      HxI = H_I[kRx][kTx] ; HxQ = H_Q[kRx][kTx] ;
      xxI =       xI[kTx] ; xxQ =       xQ[kTx] ;
      for ( kf = 0 ; kf < channelParams->numSubCarrier ; kf++ ) {
        yyI[kf] += HxI[kf]*xxI[kf] - HxQ[kf]*xxQ[kf] ;
        yyQ[kf] += HxI[kf]*xxQ[kf] + HxQ[kf]*xxI[kf] ;
      }
    }

  }  // End of kRx loop

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// TimeDomainChannel()                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

void TimeDomainChannel(
  int                          numSample ,
  float                      **xI ,     // Inputs
  float                      **xQ ,
  float                      **yI ,     // Outputs
  float                      **yQ ,
  float                        noiseStDev ,
  ChannelStateStruct          *channelState ,
  ChannelParamsStruct         *channelParams
) {

  int                          kt, kx, kz, kTx, kRx ;
  int                          numDelay, *delays_samples, maxDelay_samples ;
  int                          numTxAnt , numRxAnt, n1 ;
  float                       *xBuffI, *xBuffQ ;
  float                       *yyI, *yyQ, *xxI, *xxQ ;
  float                     ***h_I, ***h_Q, *hxI, *hxQ ;
  float                        yxI, yxQ ;
  RNG_StateStruct             *rngState ;


  rngState         = channelState->noiseRngState ;

  numTxAnt         = channelParams->numTxAntenna ;
  numRxAnt         = channelParams->numRxAntenna ;

  numDelay         = channelParams->numDelay ;
  delays_samples   = channelParams->delays_samples ;
  maxDelay_samples = channelParams->maxDelay_samples ;

  h_I              = channelState->h_I ;
  h_Q              = channelState->h_Q ;

  n1  = (numSample < maxDelay_samples) ? numSample : maxDelay_samples ;


  for ( kRx = 0 ; kRx < numRxAnt ; kRx++ ) {

    yyI = yI[kRx] ;
    yyQ = yQ[kRx] ;

    GaussVectorRNG( numSample , yyI , rngState ) ;
    GaussVectorRNG( numSample , yyQ , rngState ) ;
    for ( kt = 0 ; kt < numSample ; kt++ ) {
      yyI[kt] *= noiseStDev ;
      yyQ[kt] *= noiseStDev ;
    }

    for ( kTx = 0 ; kTx < numTxAnt ; kTx++ ) {

      xBuffI = channelState->xBuffer_I[kTx] ;
      xBuffQ = channelState->xBuffer_Q[kTx] ;
      xxI    = xI[kTx] ;
      xxQ    = xQ[kTx] ;
      hxI    = h_I[kRx][kTx] ;
      hxQ    = h_Q[kRx][kTx] ;

      for ( kt = 0 ; kt < n1 ; kt++ ) {
        yxI  = 0.0f ;
        yxQ  = 0.0f ;
        for ( kx = 0 ; kx < numDelay ; kx++ ) {
          kz = kt - delays_samples[kx] ;
          if ( kz < 0 ) {
            kz += maxDelay_samples ;
            yxI += hxI[kx]*xBuffI[kz] - hxQ[kx]*xBuffQ[kz] ;
            yxQ += hxI[kx]*xBuffQ[kz] + hxQ[kx]*xBuffI[kz] ;
          } else {
            yxI += hxI[kx]*xxI[kz] - hxQ[kx]*xxQ[kz] ;
            yxQ += hxI[kx]*xxQ[kz] + hxQ[kx]*xxI[kz] ;
          }
        }
        yyI[kt] += yxI ;
        yyQ[kt] += yxQ ;
      }

      for ( kt = n1 ; kt < numSample ; kt++ ) {
        yxI  = 0.0f ;
        yxQ  = 0.0f ;
        for ( kx = 0 ; kx < numDelay ; kx++ ) {
          kz = kt - delays_samples[kx] ;
          yxI += hxI[kx]*xxI[kz] - hxQ[kx]*xxQ[kz] ;
          yxQ += hxI[kx]*xxQ[kz] + hxQ[kx]*xxI[kz] ;
        }
        yyI[kt] += yxI ;
        yyQ[kt] += yxQ ;
      }

    }  // End of kTx loop

  }    // End of kRx loop

  // Refill Buffer

  if ( maxDelay_samples > numSample ) {

    n1 = maxDelay_samples - numSample ;

    for ( kTx = 0 ; kTx < numTxAnt ; kTx++ ) {

      xBuffI = channelState->xBuffer_I[kTx] ;
      xBuffQ = channelState->xBuffer_Q[kTx] ;
      xxI    = xI[kTx] ;
      xxQ    = xQ[kTx] ;

      for ( kt = 0 ; kt < n1 ; kt++ ) {
        kz = kt + numSample ;
        xBuffI[kt] = xBuffI[kz] ;
        xBuffQ[kt] = xBuffQ[kz] ;
      }
      for ( kt = n1 ; kt < maxDelay_samples ; kt++ ) {
        kz = kt - n1 ;
        xBuffI[kt] = xxI[kz] ;
        xBuffQ[kt] = xxQ[kz] ;
      }

    }

  } else {

    n1 = numSample - maxDelay_samples ;

    for ( kTx = 0 ; kTx < numTxAnt ; kTx++ ) {

      xBuffI = channelState->xBuffer_I[kTx] ;
      xBuffQ = channelState->xBuffer_Q[kTx] ;
      xxI    = xI[kTx] ;
      xxQ    = xQ[kTx] ;

      for ( kt = 0 ; kt < maxDelay_samples ; kt++ ) {
        kz = kt + n1  ;
        xBuffI[kt] = xxI[kz] ;
        xBuffQ[kt] = xxQ[kz] ;
      }

    }

  }

}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// Utility Functions                                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//  MatSqRoot_Flt()                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////

void MatSqRoot_Flt(
  int                 n ,
  float             **A_I ,
  float             **A_Q ,
  float             **S_I ,
  float             **S_Q
) {

  int                 ki, kj, k ;
  double              Sd_I[8][8], Sd_Q[8][8], xI, xQ, xx ;

  for ( kj = n-1 ; kj >= 0 ; kj-- ) {

    xx = A_I[kj][kj] ;
    for ( k = kj+1 ; k < n ; k++ ) {
      xx -= Sd_I[kj][k]*Sd_I[kj][k] + Sd_Q[kj][k]*Sd_Q[kj][k] ;
    }
    xx = sqrt( xx ) ;
    Sd_I[kj][kj] = xx ;
    Sd_Q[kj][kj] = 0.0 ;

    xx = 1.0 / xx ;
    for ( ki = 0 ; ki < kj ; ki++ ) {
      xI = A_I[ki][kj] ; xQ = A_Q[ki][kj] ;
      for ( k = kj+1 ; k < n ; k++ ) {
        xI -= Sd_I[ki][k]*Sd_I[kj][k] + Sd_Q[ki][k]*Sd_Q[kj][k] ;
        xQ -= Sd_Q[ki][k]*Sd_I[kj][k] - Sd_I[ki][k]*Sd_Q[kj][k] ;
      }
      Sd_I[ki][kj] = xx*xI ;
      Sd_Q[ki][kj] = xx*xQ ;
    }

  }

  for (   ki = 0 ; ki < n ; ki++ ) {
    for ( kj = 0 ; kj < ki ; kj++)
      S_I[ki][kj] = S_Q[ki][kj] = 0.0f ;
    for ( kj = ki ; kj < n ; kj++) {
      S_I[ki][kj] = (float) Sd_I[ki][kj] ;
      S_Q[ki][kj] = (float) Sd_Q[ki][kj] ;
    }
  }

#ifdef CHANNEL_DEBUG
  for (   ki = 0  ; ki < n ; ki++ ) {
    for ( kj = ki ; kj < n ; kj++ ) {
      xI = xQ = 0.0 ;
      for ( k = ((ki < kj) ? kj : ki) ; k < n ; k++ ) {
        xI += S_I[ki][k]*S_I[kj][k] + S_Q[ki][k]*S_Q[kj][k] ;
        xQ += S_Q[ki][k]*S_I[kj][k] - S_I[ki][k]*S_Q[kj][k] ;
      }
      xx = sqrt( A_I[ki][ki]*A_I[kj][kj] ) ;
      if ( fabs( A_I[ki][kj] - xI ) > 0.1*xx ) {
        assert(0) ;
      }
      if ( fabs( A_Q[ki][kj] - xQ ) > 0.1*xx ) {
        assert(0) ;
      }
    }
  }
#endif

}


