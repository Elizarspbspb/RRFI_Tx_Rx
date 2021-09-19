# ifndef   AGC_H
#   define AGC_H

#define AGCLocked 1
#define AGCStateUnLock 2
#define AGCGainUpdate 3 
#define AGCFreeze 4

#include "commonfunc.h"

typedef struct AGC_events
{
  unsigned sample;
  int AGCStatus;
  int gain;
  int RMS;
} AGCEvents;


int calculateRMS( fxp_complex* time_signal, unsigned num_of_samples );

void gain_change_via_table( int* gain, int RMS );

//int AGC_11a( fxp_complex* time_signal, int* cur_gain, int AGC_status );

//void init_AGC_table();

void init_AGCAddrModLUT();

void AGCPacketDetectionBlock( gsl_complex* time_signal, // 40 MHz signal
														  fxp_complex* fxp_signal, // need for temp
															unsigned num_of_samples, 
															unsigned* packet_start_sample,
															unsigned* AGCFreeze_sample,
															unsigned* VGA_sample );

unsigned getAGCStatus_in_sample( unsigned sample_number );

#endif // AGC_H
