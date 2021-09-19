#include "commonfunc.h"

# ifndef   SIMCONSTS_H
#   define SIMCONSTS_H

#define Pi 3.141592653589793238

//[ OFDM parameters
#define NumOfSubcarriers 64
#define NumOfUsedSubcarriers 52 
#define NumOfDataSubcarriers 48
#define NumOfPilotSubcarriers 4
#define CyclicPrefixLength 16
#define MaxBitsPerQAMSymbol 6
//]

//[ Maximum packet size and constellation parameters for memory allocation
#define MaxPacketSize ((1 << 15) + 16+288)
#define MaxCodedPacketSize (2*(MaxPacketSize+6)+288)
#define MaxNumOfSymbols (4 * MaxCodedPacketSize / NumOfDataSubcarriers)
//]

//[ Preamble length
#define  PreambleLength (16 * 10 + 2 * CyclicPrefixLength + 2 * NumOfSubcarriers)
//]

//[ Parameters for packet detection
#define Delay 16
//]

#define MaxSamplingPower 4

//[ Channel parameters
#define ExtraNoiseSamples 1000
#define ExpDecayTrms 50
#define SampFreq 20e6
#define FreqOffset 150000
//]

#include <gsl/gsl_complex.h>

//[ This struct contains basic OFDM constants
struct simConsts
{
   char PilotScramble[127]; 
   unsigned DataSubcIdx[NumOfDataSubcarriers];
	 unsigned UsedSubcIdx[NumOfUsedSubcarriers];
   unsigned PilotSubcIdx[NumOfPilotSubcarriers];
   unsigned PilotSubcPatt[NumOfPilotSubcarriers];
   unsigned DataSubcPatt[NumOfDataSubcarriers];
   char PilotSubcSymbols[NumOfPilotSubcarriers];
	 gsl_complex ShortTrainingSymbols[NumOfUsedSubcarriers];
	 char LongTrainingSymbols[NumOfUsedSubcarriers];

   int QAMSignalLevels[37][ MaxBitsPerQAMSymbol / 2 + 1 ][ 1 << ( MaxBitsPerQAMSymbol / 2 ) ];
   int QAMThresholds[37][ MaxBitsPerQAMSymbol / 2 + 1 ];

   int Arctan[16];
   fxp_complex Exp[1<<16];
};
extern struct simConsts sim_consts;
//]

enum ChannelTypes{ctAWGN=0, ctExp50, ctDoppler,     ctEnd};

//[ Non-linear ADC control block
// if defined then ADC is nonlinear and calculated using ADC_table_values
//#define USE_TABLE_ADC
//]

// Test routines compiling control
#define TESTING_ROUTINES


#endif // SIMCONSTS_H