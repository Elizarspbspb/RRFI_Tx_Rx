
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                             //                                                //
//    Channel.h                                //   (c) COPYRIGHT  2003                          //
//    John S. Sadowsky                         //   Intel Corporation                            //
//                                             //                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Revision Date:  11/28/03                                                                      //
// See Channel.c for details on revisions                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef  _NEWCHANNEL_H_
#define  _NEWCHANNEL_H_

#include "FileUtils.h"
#include "GeneralUtils.h"
#include "RNG.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Data Types
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
  AWGN_CH              = 0 ,
  E_AND_V              = 1 ,
  VINKO                = 2
} ChannelTypeType ;


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Internal Structures
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {

  float                    ***H_I ;
  float                    ***H_Q ;
  float                    ***H_E ;
  float                     **AvgE ;

} FreqDomainChannelStruct ;

typedef struct {

  int                         minDelayIndex ;
  int                         maxDelayIndex ;
  float                      *relEnergy_dB ;
  float                       AoA_AS_deg ;
  float                       AoD_AS_deg ;
  float                      *AoA_deg ;
  float                      *AoD_deg ;

  float                      *energy ;

} ClusterParamsStruct ;

typedef struct {

  float                       dopplerBW_Hz ;
  float                       dopplerOffset_Hz ;

  boolean                     dopplerOffsetFlag ;
  float                       beta ;
  float                       omega ;
  float                       updateTime0_muSec ;
  float                       updateTime1_muSec ;

  boolean                    *tapFlag ;

  int                         numCluster ;
  ClusterParamsStruct        *clusterParams ;

  float                      *tapEnergy ;
  float                      *sigmaChannelTap ;

  float                    ***rxRho_I ;
  float                    ***rxRho_Q ;

  float                    ***txRho_I ;
  float                    ***txRho_Q ;

  float                    ***rxSqrtRho_I ;
  float                    ***rxSqrtRho_Q ;

  float                    ***txSqrtRho_I ;
  float                    ***txSqrtRho_Q ;

} DopplerParamsStruct ;

typedef struct {

  float                      *nextUpdateTime0_muSec ;
  float                      *nextUpdateTime1_muSec ;

  float                       minNextUpdateTime_muSec ;

  float                      *cs ;
  float                      *sn ;

  float                    ***h_iid_I ;
  float                    ***h_iid_Q ;
  float                    ***h_I ;
  float                    ***h_Q ;

} DopplerStateStruct ;



///////////////////////////////////////////////////////////////////////////////////////////////////
//  Params and State Structures
///////////////////////////////////////////////////////////////////////////////////////////////////


typedef struct {

  char                        channelModelName[MAX_FILE_NAME_LEN] ;

  boolean                     pwrNormFlag ;

  int                         numTxAntenna ;
  int                         numRxAntenna ;

  float                       txAntennaSpacing_cm ;
  float                       rxAntennaSpacing_cm ;

  boolean                     MIMO_Flag ;
  boolean                     rxCorrelationFlag ;
  boolean                     txCorrelationFlag ;

  float                       carrierFreq_GHz ;
  float                       lambda_cm ;

  ChannelTypeType             channelType ;

  boolean                     freqDomainFlag ;          // Frequency domain parameters
  int                         numSubCarrier ;
  float                       subCarrierSpacing_kHz ;

  float                       sampleRate_Msps ;
  float                       maxSamplingError_nsec ;   // For time domain channels, delays will be aligned with
                                                        // integral samples.  This can introduce a delay error relative
                                                        // to the model specification - and this parameter specifices
                                                        // the maximum tollerable error.

  boolean                     LOS_Flag ;                // Used only in LOS conditions.
  float                       RicianFactorK_dB ;        // Applied only on the 1st tap.
  float                       AoA_deg_LOS ;
  float                       AoD_deg_LOS ;

  float                       sigma_LOS ;
  float                     **h_LOS_I ;
  float                     **h_LOS_Q ;

  int                         numDelay ;                // Delay params - common
  float                      *delays_ns ;
  int                        *delays_samples ;

  float                       rmsDelaySpread_ns ;       // E_AND_V parameters
  float                       deltaTau_ns ;
  float                       maxDelay_ns ;
  int                         maxDelay_samples ;

  float                       dopplerUpdateFrac ;

  int                         numDoppler ;              // Vinko Params
  DopplerParamsStruct        *dopplerParams ;
  boolean                     dopplerFlag ;

  float                     **cs ;                      // cos/sin used for time domain => freq domain
  float                     **sn ;

} ChannelParamsStruct ;

typedef struct {

  DopplerStateStruct         *dopplerState ;

  float                    ***h_I ;
  float                    ***h_Q ;
  float                     **h_E ;

  float                     **xBuffer_I ;
  float                     **xBuffer_Q ;

  FreqDomainChannelStruct     freqDomainChannel ;

  RNG_StateStruct            *chRngState ;
  RNG_StateStruct            *noiseRngState ;

} ChannelStateStruct ;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Function Prototypes
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
//  The first two are Wrapper functions that initialize both Params and State.  These
//  can be used in BCubed Stars
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
) ;

void Wrapup_Channel (
  ChannelStateStruct          *channel_State ,
  ChannelParamsStruct         *channel_Params
) ;


///////////////////////////////////////////////////////////////////////////////////////////////////
//  The following Init and Free functions operate on State and Params separately.  These
//  can be used when on Params struct is used to service many state structs.
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
) ;

void InitChannelState (
  RNG_StateStruct            *chRngState ,
  RNG_StateStruct            *noiseRngState ,
  ChannelParamsStruct        *channelParams ,
  ChannelStateStruct         *channelState
) ;


void FreeChannelState (
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void FreeChannelParams (
  ChannelParamsStruct        *channelParams
) ;

void WriteToFileChannelParams (
  FILE                       *out ,
  ChannelParamsStruct        *channelParams
) ;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  The following functions are run time functions
///////////////////////////////////////////////////////////////////////////////////////////////////

void GenerateChannelResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void UpdateChannelResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void GenerateImpulseResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

boolean UpdateImpulseResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

void GenerateFreqResponse(
  float                        time_muSec ,
  ChannelStateStruct          *channel_State ,
  ChannelParamsStruct         *channel_Params
) ;

void UpdateFreqResponse(
  float                       time_muSec ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  The following functions are run time functions
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
) ;

void TimeDomainChannel(
  int                          numSample ,
  float                      **xI ,     // Inputs
  float                      **xQ ,
  float                      **yI ,     // Outputs
  float                      **yQ ,
  float                        noiseStDev ,
  ChannelStateStruct          *channelState ,
  ChannelParamsStruct         *channelParams
) ;

void FreqDomainChannel(
  float                      **xI ,
  float                      **xQ ,
  float                      **yI ,
  float                      **yQ ,
  float                        noiseStDev ,
  ChannelStateStruct          *channel_State ,
  ChannelParamsStruct         *channel_Params
) ;


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Debug Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

float CalcRMS_DelaySpread(
  ChannelParamsStruct        *channelParams
) ;

void CalcChannel_DelaySpreads(
  float                     **delaySpreads ,
  float                     **energies ,
  ChannelStateStruct         *channelState ,
  ChannelParamsStruct        *channelParams
) ;

#endif // _NEWCHANNEL_H_

