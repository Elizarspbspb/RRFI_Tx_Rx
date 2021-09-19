//#pragma once
# ifndef   SIMPARAMS_H
#   define SIMPARAMS_H

struct simParameters
{
  unsigned data_rate;
  unsigned bits_per_QAM_symbol;
  unsigned data_bits_per_OFDM_symbol;
  double code_rate;

  unsigned sampling_power;

  char useLDPC;

  char useGolay;

  char useABL;

  int useJitter;

  int packets_count;

  int channel_type;
  
  char idealFrequencySync;

  char idealChannelEstimation;
  
  char subcarriersPower;
  
  char idealTiming;
  
  char advTraining;

  double curSNR;
  
  unsigned packet_size;

  // if Doppler effect is simulated then provide model filename and name
  char DOPPLER_MODEL_FILENAME[100];
  char DOPPLER_MODEL_NAME[100];
};

extern struct simParameters sim_params;

struct fxpParams
{
  unsigned FFT_precision;
  unsigned FFT_exp_position;
  unsigned IFFT_shift_control;
  unsigned FFT_shift_control;

  unsigned QAM_precision;

  unsigned Demodulator_precision;
  unsigned Demodulator_exp_position;

  unsigned first_bit_for_demodulator;
  unsigned last_bit_for_demodulator;

  unsigned ADC_DAC_precision;
  double   ADC_DAC_max;

  unsigned Viterbi_precision;
  unsigned LDPC_precision;
  
  unsigned first_bit_for_decoder;
  unsigned last_bit_for_decoder;

  unsigned Freq_Sync_precision;
  unsigned Freq_Sync_exp_position;

  unsigned Arctan_precision;

  unsigned Packet_detection_precision;
  unsigned Packet_detection_exp_position;

  unsigned Time_Sync_precision;
  unsigned Time_Sync_exp_position;

  unsigned AGC_precision;

  unsigned Exp_precision;
  unsigned Exp_exp_position;
};

extern struct fxpParams fxp_params;

#endif // SIMPARAMS_H