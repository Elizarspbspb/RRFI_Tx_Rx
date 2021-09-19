/* File History
 * $History: sim_params.c $
 * 
 * *****************  Version 15  *****************
 * User: Akozlov      Date: 22.09.05   Time: 17:52
 * Updated in $/WLAN
 * 12 ADC
 * 
 * *****************  Version 14  *****************
 * User: Akozlov      Date: 4.03.05    Time: 15:56
 * Updated in $/WLAN
 * LDPC precision
 * 
 * *****************  Version 13  *****************
 * User: Akozlov      Date: 14.12.04   Time: 11:45
 * Updated in $/WLAN
 * 
 * *****************  Version 12  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:51
 * Updated in $/WLAN
 * Decoder accuracy precision changed
 * 
 * *****************  Version 11  *****************
 * User: Akozlov      Date: 7.12.04    Time: 12:04
 * Updated in $/WLAN
 * 
 * *****************  Version 10  *****************
 * User: Akozlov      Date: 7.12.04    Time: 11:48
 * Updated in $/WLAN
 * 
 * *****************  Version 9  *****************
 * User: Akozlov      Date: 6.12.04    Time: 20:47
 * Updated in $/WLAN
 * Demodulator input accuracy added
 * Precision are changed due to ADC precision
 * 
 * *****************  Version 8  *****************
 * User: Akozlov      Date: 3.12.04    Time: 18:19
 * Updated in $/WLAN
 * Different precision variant added
 * 
 * *****************  Version 7  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:53
 * Updated in $/WLAN
 * 
 * *****************  Version 6  *****************
 * User: Akozlov      Date: 3.12.04    Time: 16:45
 * Updated in $/WLAN
 * Decoders input accuracy added
 */
#pragma once
#include "sim_params.h"

/*struct fxpParams fxp_params =
{
  // FFT parameters
  16, // FFT_precision
  14, // FFT_exp_position
  4,  // IFFT_shift_control
  4,  // FFT_shift_control

  // QAM parameters
  10, // QAM_precision

  // Demodulator parameters
  12, // Demodulator_precision
  10, // Demodulator_exp_position

  // ADC parameters
  12, // ADC_DAC_precision
  1,  // ADC_DAC_max

  // Decoders parameters
  12, // Viterbi_precision
  12, // LDPC_precision

  // Decoders input accuracy
  0,  // first bit from demodulator output
  7,  // last bit from demodulator output

  // Frequency synchronization parameters
  16, // Freq_Sync_precision
  12, // Freq_Sync_exp_position

  // Arctan table parameters
  16, // Arctan_precision

  // Packet detection parameters
  16, // Packet detection precision
  15, // Packet detection exp_position autocorrelation

  // Time synchronization parameters
  16, // Time_Sync_precision
  11, // Time_Sync_exp_position

  // AGC parameters
  16, // AGC_precision

  // Exp and rotation parameters
  16, // Exp_precision
  14  // Exp_exp_position
};*/

struct fxpParams fxp_params =
{
  // FFT parameters
  16, // FFT_precision
  14, // FFT_exp_position
  4,  // IFFT_shift_control
  0,  // FFT_shift_control

  // QAM parameters
  10, // QAM_precision

  // Demodulator parameters
  16, // Demodulator_precision
  12, // Demodulator_exp_position

  // Demodulator input accuracy
  0,  // first bit from FFT output
  15, // last bit from FFT output

  // ADC parameters
  12, // ADC_DAC_precision
  1,  // ADC_DAC_max

  // Decoders parameters
  11, // Viterbi_precision
  11, // LDPC_precision

  // Decoders input accuracy
  0,  // first bit from demodulator output
  7,  // last bit from demodulator output

  // Frequency synchronization parameters
  16, // Freq_Sync_precision
  11,  // Freq_Sync_exp_position

  // Arctan table parameters
  16, // Arctan_precision

  // Packet detection parameters
  16, // Packet detection precision
  15, // Packet detection exp_position autocorrelation

  // Time synchronization parameters
  16, // Time_Sync_precision
  11, // Time_Sync_exp_position

  // AGC parameters
  16, // AGC_precision

  // Exp and rotation parameters
  16, // Exp_precision
  14  // Exp_exp_position
};

struct simParameters sim_params;