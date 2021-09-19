/* File History
 * $History: main.c $
 * 
 * *****************  Version 12  *****************
 * User: Akozlov      Date: 28.07.05   Time: 19:05
 * Updated in $/WLAN
 * use_Golay option added
 * 
 * *****************  Version 11  *****************
 * User: Akozlov      Date: 26.07.05   Time: 14:29
 * Updated in $/WLAN
 * Feedback channel stat
 * 
 * *****************  Version 10  *****************
 * User: Akozlov      Date: 21.04.05   Time: 13:18
 * Updated in $/WLAN
 * setUpABLThesholds added
 * 
 * *****************  Version 9  *****************
 * User: Akozlov      Date: 29.03.05   Time: 19:35
 * Updated in $/WLAN
 * ABL thresholds from config
 * 
 * *****************  Version 8  *****************
 * User: Akozlov      Date: 24.12.04   Time: 19:41
 * Updated in $/WLAN
 * Advanced training option added
 * 
 * *****************  Version 7  *****************
 * User: Akozlov      Date: 10.12.04   Time: 14:48
 * Updated in $/WLAN
 *
 * *****************  Version 8  *****************
 * User: Eegorov      Date: 10.11.20   Time: 14:48
 * Updated in $/WLAN
 * 
 */
/*TRANSMITTER*/
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#include <cmath>
#include <memory.h>//transm
#include <string.h>//transm
//#include <cstring>//transm
//#include <conio.h> // for getch()
#include "model.h"//transm
#include "sim_consts.h"//transm
#include "sim_params.h"//transm
#include "transmitter.h"//transm
#include "commonfunc.h"//transm
#include "channel.hpp"
#include "receiver.h"
#include "Interpolation.h"
#include "bitloading.h"
#include "DopplerChannel/FileUtils.h"
#include "fft.h"
#include "scrambler.h"
#include "conv_encoder.h"
#include "interleaver.h"
#include "modulator.h"
#include "puncturer.h"
#include "pilots_insertion.h"
#include "add_preamble.h"
#include "add_cyclic_prefix.h"
#include "DAC.h"
#include "ldpc_encode.h"
#include "modulatorABL.h"
#include "golay_wrap.h"
#include <stdint.h> // int64_t
//#include <cstdint> // int64_t
#include <inttypes.h> // int64_t  printf
#include "ADC.h"
#include "fxp_to_gsl.cpp"
#include "gsl_to_fxp.cpp"
#define FILE_PATH "data.txt"
#define BUF_SIZE 256
using namespace std;


//[ Test message from 802.11a standard
static unsigned char test_message[100] = 
{	0x04, 0x02, 0x00, 0x2e, 0x00,
	0x60, 0x08, 0xcd, 0x37, 0xa6,
	0x00, 0x20, 0xd6, 0x01, 0x3c,
	0xf1, 0x00, 0x60, 0x08, 0xad,
	0x3b, 0xaf, 0x00, 0x00, 0x4a,
	0x6f, 0x79, 0x2c, 0x20, 0x62,
	0x72, 0x69, 0x67, 0x68, 0x74,
	0x20, 0x73, 0x70, 0x61, 0x72,
	0x6b, 0x20, 0x6f, 0x66, 0x20,
	0x64, 0x69, 0x76, 0x69, 0x6e,
	0x69, 0x74, 0x79, 0x2c, 0x0a,
	0x44, 0x61, 0x75, 0x67, 0x68,
	0x74, 0x65, 0x72, 0x20, 0x6f,
	0x66, 0x20, 0x45, 0x6c, 0x79,
	0x73, 0x69, 0x75, 0x6d, 0x2c,
	0x0a, 0x46, 0x69, 0x72, 0x65,
	0x2d, 0x69, 0x6e, 0x73, 0x69,
	0x72, 0x65, 0x64, 0x20, 0x77,
	0x65, 0x20, 0x74, 0x72, 0x65,
	0x61, 0xda, 0x57, 0x99, 0xed };
//]

//[Test ABL
#ifdef ABL_TEST
// ABL Test 1
	  int ABL_bits_per_QAM_symbol_1[48]={1, 2, 6, 2, 4, 1, 0, 1, 2, 0, 6, 4, 4, 6, 1, 1, 2, 4, 6, 1, 0, 0, 1, 6, 4, 0, 4, 2, 0, 1, 1, 4, 2, 0, 6, 6, 6, 0, 0, 4, 1, 1, 2, 4, 2, 2, 2, 6};
// ABL Test 2
	  int ABL_bits_per_QAM_symbol_2[48]={0, 0, 0, 0, 1, 2, 6, 2, 4, 1, 1, 2, 6, 4, 4, 6, 1, 1, 2, 4, 6, 1, 0, 0, 1, 6, 4, 0, 4, 2, 0, 1, 1, 4, 2, 0, 6, 6, 6, 4, 1, 1, 2, 4, 2, 2, 2, 6};
// ABL Test 3
	  int ABL_bits_per_QAM_symbol_3[48]={0, 0, 0, 1, 2, 6, 2, 4, 1, 0, 1, 2, 6, 4, 4, 6, 1, 1, 2, 4, 6, 1, 1, 6, 4, 0, 4, 2, 0, 1, 1, 4, 2, 6, 6, 6, 4, 1, 1, 2, 4, 2, 2, 2, 6, 0, 0, 0};
// ABL Test 4
	  int ABL_bits_per_QAM_symbol_4[48]={1, 1, 1, 0, 0, 0, 2, 4, 6, 2, 0, 2, 6, 4, 4, 6, 1, 1, 2, 4, 6, 1, 1, 6, 4, 0, 4, 2, 0, 1, 1, 4, 2, 6, 6, 6, 4, 1, 1, 2, 4, 2, 2, 2, 6, 0, 0, 0};
#endif
//]


// Устаревшая функция без Тестового Объекта. static std::vector<std::complex<float>> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, int *num_of_samples2);
//static std::vector<short> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, int* num_of_samples2);
static std::vector<std::complex<float>> simulate1(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, int *num_of_samples2, char buf[BUF_SIZE]);



typedef unsigned (*TRoutine_get_channel_ir)( gsl_complex* cir );
typedef unsigned (*TRoutine_get_channel_ir_len)();
typedef unsigned (*TRoutine_channel)( gsl_complex* received_signal, 
                  gsl_complex* time_signal,
                  gsl_complex* cir, 

                  unsigned num_of_samples,
                  unsigned ir_duration,  
                  double SNR );

 static TRoutine_get_channel_ir Routine_get_channel_ir[ctEnd]={
  get_AWGN_channel_ir,
  get_channel_ir,
  get_dopplerChannel_ir
};

static TRoutine_get_channel_ir_len Routine_get_channel_ir_len[ctEnd]={
  get_AWGN_channel_ir_len,
  get_channel_ir_len,
  get_dopplerChannel_ir_len
};
static TRoutine_channel Routine_channel[ctEnd]={
  channel,
  channel,
  dopplerChannel
};
static TRoutine_channel Routine_channel2[ctEnd] = {
  channel,
  channel,
  dopplerChannel
};

// ---------- My function //
static int64_t getFileSize(const char* file_name)
{
    int64_t _file_size = 0;
    FILE* fd = fopen(file_name, "rb");
    if (fd == NULL) {
        _file_size = -1;
    }
    else {
        while (getc(fd) != EOF)
            _file_size++;
        fclose(fd);
    }
    return _file_size;
}

static std::vector<std::complex<float>> MyTransmitter(char* param, int xx, int *num_of_samples2, char buf[BUF_SIZE]){ // было
    printf("Transmitter");
  std::vector<std::complex<float>> buff(MaxPacketSize);
  int SNR_count;
  float* SNRs;
  float ABL_thresholds[4];
  int packet_size;
  int data_rate;
  int use_ldpc;
  int use_ABL;
  int use_jitter;
  int channel_type;
  int packet_count;
  int idealFrequencySync;
  int idealChannelEstimation;
  int subcarriersPower;
  int idealTiming;
  int advTraining;
  int useGolay;

  int i;

  ReadInt("General", "SNR_count", param, "decimal", &SNR_count);
  SNRs = (float*)malloc(SNR_count * sizeof(float));
  ReadMultiFloat("General", "SNRs", param, SNR_count, SNRs);
  ReadInt("General", "packet_size", param, "decimal", &packet_size);
  ReadInt("General", "data_rate", param, "decimal", &data_rate);
  ReadInt("General", "use_ldpc", param, "decimal", &use_ldpc);
  ReadInt("General", "use_Golay", param, "decimal", &useGolay);
  ReadInt("General", "use_ABL", param, "decimal", &use_ABL);
  ReadInt("General", "use_jitter", param, "decimal", &use_jitter);
  ReadInt("General", "channel_type", param, "decimal", &channel_type);
  ReadInt("General", "packet_count", param, "decimal", &packet_count);
  ReadInt("General", "sampling_power", param, "decimal", (int*)&sim_params.sampling_power);
  ReadInt("General", "idealFrequencySync", param, "decimal", &idealFrequencySync);
  ReadInt("General", "idealChannelEstimation", param, "decimal", &idealChannelEstimation);
  ReadInt("General", "idealTiming", param, "decimal", &idealTiming);
  ReadInt("General", "subcarriersPower", param, "decimal", &subcarriersPower);
  ReadInt("General", "advTraining", param, "decimal", &advTraining);
  ReadMultiFloat("General", "ABLThresholds", param, 4, ABL_thresholds);


  if (ctDoppler == channel_type) {
      ReadString("DopplerChannel", "DOPPLER_MODEL_FILENAME", param, sim_params.DOPPLER_MODEL_FILENAME);
      ReadString("DopplerChannel", "DOPPLER_MODEL_NAME", param, sim_params.DOPPLER_MODEL_NAME);
  }
  printf(" Channel_type = %d \n", channel_type);


  sim_params.useABL = use_ABL;
  sim_params.useJitter=use_jitter;
  sim_params.idealFrequencySync = idealFrequencySync;
  sim_params.idealChannelEstimation = idealChannelEstimation;
  sim_params.subcarriersPower = subcarriersPower;
  sim_params.idealTiming = idealTiming;
  sim_params.advTraining = advTraining;
  setUpABLThresholds( ABL_thresholds );
  

  sim_params.useGolay = useGolay;

	#ifdef AQ_LOG
	aq_log=fopen("aq_log.txt","w");
	fprintf(aq_log,"ACF\tThreshold\tRMS\n");
	fclose(aq_log);
  #endif

    //  #ifdef AGC_TEST
//	agc_log=fopen("agc_log.log","w");
//	fprintf(agc_log,"RMS\tGain\tGainChange\tNumOfClipping\n");
//	fclose(agc_log);
//  #endif

	#ifdef TEST_SEQ
	packet_size = 100;
    //packet_size = 1;
	#endif

    if (xx==1)
    {
        for (int i = 0; i < SNR_count; i++) {
            sim_params.packet_size = packet_size;
            sim_params.curSNR = SNRs[0];
            //simulate(SNRs[0], packet_size, data_rate, channel_type, use_ldpc, packet_count);
            //buff = simulate(SNRs[0], packet_size, data_rate, channel_type, use_ldpc, packet_count, num_of_samples2); // было
            buff = simulate1(SNRs[0], packet_size, data_rate, channel_type, use_ldpc, packet_count, num_of_samples2, buf);
        }
    }
    else if (xx == 2)
    {
        printf("Start Priem\n");
    }
  return buff;
}

static std::vector<std::complex<float>> simulate1(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, int *num_of_samples2, char buf[BUF_SIZE])
{

    unsigned int i;
    char log_filename[30];
    char instant_log_filename[30];
    double time_last_dump = 0;

    unsigned char inf_bits[MaxPacketSize];
    unsigned char out_bits[MaxPacketSize];
    //std::complex<float> time_signal[(1 << MaxSamplingPower) *
    //                                (PreambleLength + MaxNumOfSymbols *
    //                                                  (CyclicPrefixLength + NumOfSubcarriers))];
    //std::complex<float> time_signal[7092480];

    std::complex<float> time_signal[10000];

    gsl_complex* cir = NULL;
    unsigned ir_duration;

    unsigned num_of_samples;

    gsl_complex* received_signal;
    unsigned num_of_received_samples;
    unsigned iter;
    unsigned bits_transmitted = 0;
    unsigned packets_transmitted = 0;
    unsigned error_bits = 0;
    unsigned error_packets = 0;
    unsigned char packet_error;
    unsigned bits_per_OFDM_symbol;
    double throughput;
    double data_throughput;
    double totalMps = 0;

    unsigned bits_transmitted_Sync = 0;
    unsigned error_bits_Sync = 0;

    double total_packets_time = 0;
    unsigned packet_detection_errors = 0;

    int ABL_bits_per_QAM_symbol[NumOfDataSubcarriers];
    char syncError;

    unsigned feedBackNumOfOFDMSyms;
    unsigned averNumOfOFDMSyms = 0;
    unsigned allNumOfOFDMSyms = 0;
    unsigned maxNumOfOFDMSyms = 0;

    FILE* file=NULL;
    std::vector<std::complex<float>> Mybuff(MaxPacketSize); //���
    std::vector<std::complex<float>> Mybuff2(MaxPacketSize); //���
    //std::vector<short> Mybuff(MaxPacketSize); //���

    //[ Initialize simulation parameters
    sim_params.data_rate = data_rate;
    sim_params.useLDPC = use_ldpc;
    sim_params.channel_type = channel_type;
    sim_params.packets_count = packet_count;
    sprintf( log_filename, "log%f.log", SNR );
    sprintf( instant_log_filename, "__I_log%f.log", SNR );
    initialize_model(); //error
    //]

    //[ Allocate channel signal and IR storage
    ir_duration = (Routine_get_channel_ir_len[channel_type])();
    cir = (gsl_complex*)malloc(ir_duration * sizeof(gsl_complex));
    received_signal = (gsl_complex*)malloc((ExtraNoiseSamples +
                                            (1 << MaxSamplingPower) * (
                                                    (PreambleLength + MaxNumOfSymbols *
                                                                      (CyclicPrefixLength + NumOfSubcarriers)) +
                                                    ir_duration - 1))
                                           * sizeof(gsl_complex));
    //]

    //[ Initialize ABL parameters
    for ( i = 0; i < NumOfDataSubcarriers; i++ )
    {
        ABL_bits_per_QAM_symbol[i] = sim_params.bits_per_QAM_symbol;
    }
    bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
    //]

    //[ABL Test
#ifdef ABL_TEST
    for ( i = 0; i < 48; i++ )
	{
      ABL_bits_per_QAM_symbol[i] = ABL_bits_per_QAM_symbol_4[i];
	}

    bits_per_OFDM_symbol = 0;
    for ( i = 0; i < 48; i++ )
	{
	  bits_per_OFDM_symbol += ABL_bits_per_QAM_symbol[i];
	}
#endif
    //]


    for ( iter = 0; iter < sim_params.packets_count; iter++ )
    {
#ifdef LOGGING
        sprintf( overflow_comment, "Iteration=%d\n", iter + 1 );
        logging( overflow_comment, 0 );
#endif

#ifdef TEST_SEQ
        for ( i = 0; i < 8 * packet_size; i++ )
		{
			inf_bits[i] = ( test_message[ i / 8 ] >> ( i % 8 ) ) & 1;
		}
#else
        FILE* data;
        data = fopen(FILE_PATH, "r+");
        unsigned char mas1[MaxPacketSize];
        char cc;
        int k = 1; // 00000001
        int kk = 128; // 10000000
        int chet = 0;
        int and1;
        int ii = 8;
        int i = 0;
        static int64_t file_size = getFileSize(FILE_PATH);
        //printf("File size: %d \n", file_size);
        //for (int i = 0; i < file_size; i++)
        //{
            //while ((cc = fgetc(data)) != EOF)
            while (ii < MaxPacketSize)
            {
                cc = buf[i];
                mas1[i] = cc;
                printf(" %u ", mas1[i]);
                printf(" - ");
                while(chet<ii)
                {
                    and1 = (mas1[i] & k);
                    mas1[i] >>= k;
                    inf_bits[chet] = and1;
                    printf(" %u ", inf_bits[chet]);
                    chet++;
                }
                printf("\n");
                if(buf[i] == '\0')
                    break;
                ii = ii+8;
                i++;
            }
        //}
        fclose(data);

#endif
        if ( sim_params.useLDPC )
        {
            totalMps += bits_per_OFDM_symbol * 0.75 *
                        (double) SampFreq / ( CyclicPrefixLength + NumOfSubcarriers ) / 10e5;
        }
        else
        {
            totalMps += bits_per_OFDM_symbol * sim_params.code_rate *
                        (double) SampFreq / ( CyclicPrefixLength + NumOfSubcarriers ) / 10e5;
        }

        if ( !sim_params.useABL )
        {
            bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
        }

        //[ Modulate data and prepare it for transmission

    ////////////////////////////////////////////////////////////////////////////////
    // Name: transmitter
    //
    // Purpose: Implements OFDM transmitter scheme: FEC coding, modulation,
    //					adding preamble and training symbols, generating time domain signal
    //
    // Author: Alexandr Kozlov
    //
    // Parameters: time_signal - output, Time domain signal stores
    //
    //						 inf_bits - input, data bits to be transmitted
    //
    //             packet_size - input, Packet size in bytes
    //
    //             ABL_bits_per_QAM_symbol - input, Subcarriers modulation for ABL
    //
    //             bits_per_OFDM_symbol - input, Coded bits per OFDM symbol
    //
    // Returning value: Number of time domain samples
    ////////////////////////////////////////////////////////////////////////////////
        unsigned j;
        unsigned char data_bits[MaxPacketSize + LDPCMessageLength];
        unsigned char coded_bits[MaxCodedPacketSize];
        unsigned char interl_bits[MaxCodedPacketSize];
        unsigned char punctured_bits[MaxCodedPacketSize];

        unsigned num_of_coded_bits;
        unsigned num_of_OFDM_symbols;
        unsigned num_of_pad_bits = 0;
        unsigned num_of_samples;
        //unsigned num_of_samples2;
        unsigned num_of_LDPC_codewords;
        unsigned num_of_LDPC_zerobits;

        fxp_complex mod_symbols[48 + MaxCodedPacketSize]; // PLCP symbol + data symbols
        fxp_complex ofdm_symbols[MaxNumOfSymbols * NumOfSubcarriers];

        fxp_complex fxp_time_signal[PreambleLength + MaxNumOfSymbols *
                                                     (CyclicPrefixLength + NumOfSubcarriers)];

        unsigned num_of_total_bits;

#ifdef LOGGING
        logging("Transmitter...\n", 1);
#endif

        //[ Prepare PLCP header
#ifdef LOGGING
        logging("Prepare PLCP header...\n", 1);
#endif
        memset(data_bits, 0, 24);
        get_PLCP_header(data_bits, packet_size);
#ifdef TEST_SEQ
        dump_char_array(data_bits, 24, "PLCP_header.log", "PLCP header");
#endif

        if (sim_params.useGolay)
        {
            GolayEncode(golay_handle, data_bits, coded_bits);
            GolayEncode(golay_handle, data_bits + 12, coded_bits + 24);
        }
        else
            conv_encoder_11a(data_bits, 24, coded_bits);

#ifdef TEST_SEQ
        dump_char_array(coded_bits, 48, "PLCP_header_coded.log", "PLCP header after encoding");
#endif

        interleaver_11a(coded_bits, interl_bits, 48, 48);

#ifdef TEST_SEQ
        dump_char_array(interl_bits, 48, "PLCP_header_interl.log", "PLCP header after interleaving");
#endif

        modulator_11a(mod_symbols, interl_bits, 48, 1);   // вот тут должна быть ошибка какая-та

#ifdef LOGGING
        logging("Prepare PLCP header finished\n", -1);
#endif
        //]

        //[ Add SERVICE field to data bits and scramble its
        memset(data_bits, 0, 16);
        memcpy(data_bits + 16, inf_bits, 8 * packet_size);
        //]

        //[ Add tail bits
        memset(data_bits + 16 + 8 * packet_size, 0, 6);
        //]

        //[ Padding
        if (!sim_params.useLDPC && !sim_params.useGolay)
        {
            printf("packet_size = %d \n", packet_size);
            //[ Add padding bits to the information bits in convolutional code case
            num_of_OFDM_symbols = (unsigned)ceil((double)(16 + 8 * packet_size + 6) /
                                                 (double)(bits_per_OFDM_symbol * sim_params.code_rate));

            num_of_pad_bits = (unsigned)(num_of_OFDM_symbols * bits_per_OFDM_symbol * sim_params.code_rate -
                                         (16 + 8 * packet_size + 6));
            for (i = 0; i < num_of_pad_bits; i++)
            {
                data_bits[16 + 8 * packet_size + 6 + i] = 0;
            }

            num_of_total_bits = 16 + 8 * packet_size + 6 + num_of_pad_bits;
            //]
        }
        else
        if (sim_params.useGolay)
        {
            //[ Add padding bits to the information bits in Golay code case
            num_of_OFDM_symbols = (unsigned)ceil((double)(16 + 8 * packet_size) /
                                                 (double)(bits_per_OFDM_symbol * 0.5));

            num_of_pad_bits = (unsigned)(num_of_OFDM_symbols * bits_per_OFDM_symbol * 0.5 -
                                         (16 + 8 * packet_size));

            for (i = 0; i < num_of_pad_bits; i++)
            {
                data_bits[16 + 8 * packet_size + i] = 0;
            }

            num_of_total_bits = 16 + 8 * packet_size + num_of_pad_bits;
            //]
        }
        else
        {
            //[ Add padding bits to the information bits in LDPC code case
            num_of_LDPC_zerobits = LDPCMessageLength - (8 * packet_size + 16) % LDPCMessageLength;

            if (num_of_LDPC_zerobits == LDPCMessageLength)
            {
                num_of_LDPC_codewords = (8 * packet_size + 16) / LDPCMessageLength;
            }
            else
            {
                num_of_LDPC_codewords = (8 * packet_size + 16) / LDPCMessageLength + 1;
            }

            //[ Add padding bits for last message
            for (i = 0; i < num_of_LDPC_zerobits; i++)
            {
                data_bits[num_of_LDPC_codewords * LDPCMessageLength -
                          num_of_LDPC_zerobits + i] = 0;
            }
            //]

            //[ Add pad bits for LDPC case due to LDPC codeword length may not divide by NumOfDataSubcarriers
            num_of_coded_bits = num_of_LDPC_codewords * LDPCCodewordLength;

            num_of_OFDM_symbols = (unsigned)ceil((double)num_of_coded_bits /
                                                 bits_per_OFDM_symbol);

            num_of_pad_bits = num_of_OFDM_symbols * bits_per_OFDM_symbol - num_of_coded_bits;

            for (i = 0; i < num_of_pad_bits; i++)
            {
                data_bits[num_of_LDPC_codewords * LDPCMessageLength + i] = 0;
            }
            //]

            num_of_total_bits = num_of_LDPC_codewords * LDPCMessageLength + num_of_pad_bits;
            //]
        }
        //]
#ifdef TEST_SEQ
        dump_char_array(data_bits,
        num_of_total_bits,
        "data_bits.log",
        "DATA bits");
#endif
        //[ Scrambling
        scrambler_11a(data_bits, num_of_total_bits, &tx_scrambler_state, 0);
        //]
        if (!sim_params.useLDPC && !sim_params.useGolay)
        {
            //[ Set tail bits to zero
            memset(data_bits + 16 + 8 * packet_size, 0, 6);
            //]
        }

        //]

#ifdef TEST_SEQ
        dump_char_array(data_bits,
        num_of_total_bits,
        "data_bits_scrambled.log",
        "DATA bits after scrambling");
#endif

        //[ Encode data
        if (!sim_params.useLDPC && !sim_params.useGolay)
        {
            //[ Encode data using convolutional code

            num_of_coded_bits =
                    conv_encoder_11a(data_bits, 16 + 8 * packet_size + 6 + num_of_pad_bits, coded_bits);
            num_of_coded_bits =
                    puncturer_11a(coded_bits,
                                  num_of_coded_bits,
                                  punctured_bits,
                                  sim_params.code_rate);

#ifdef TEST_SEQ
            dump_char_array(punctured_bits,
            num_of_coded_bits,
            "data_bits_coded.log",
            "Coded DATA bits");
#endif
            //]
        }
        else
        if (sim_params.useGolay)
        {
            // Golay encode
            num_of_coded_bits = 2 * num_of_total_bits;
            for (i = 0; i < 2 * num_of_OFDM_symbols; i++)
            {
                GolayEncode(golay_handle, data_bits + 12 * i, punctured_bits + 24 * i);
            }
        }
        else
        {
            //[ Encode data using  LDPC code
            for (i = 0; i < num_of_LDPC_codewords; i++)
            {
                ldpc_encode((const char*)data_bits + i * LDPCMessageLength,
                            (char*)punctured_bits + i * LDPCCodewordLength);
            }

            memcpy(punctured_bits + num_of_LDPC_codewords * LDPCCodewordLength,
                   data_bits + num_of_LDPC_codewords * LDPCMessageLength, num_of_pad_bits);

#ifdef TEST_SEQ
            dump_char_array(punctured_bits,
                num_of_coded_bits + num_of_pad_bits,
                "data_bits_coded_LDPC.log",
                "LDPC Coded DATA bits");
#endif
            //]
        }
        //]

        if (!sim_params.useLDPC)
        {
            //[ Interleave bits
            interleaver_11a(punctured_bits,
                            interl_bits,
                            num_of_OFDM_symbols * bits_per_OFDM_symbol,
                            bits_per_OFDM_symbol);

#ifdef TEST_SEQ
            dump_char_array(interl_bits,
            num_of_coded_bits,
            "data_bits_inter.log",
            "DATA bits after interleaving");
#endif
            //]
        }

        num_of_OFDM_symbols++; // due to PLCP header symbol

        //[ Modulate data bits to the mod_symbols array, mod_symbols precision is equal to fxp_params.QAM_precision
        if (!sim_params.useABL)
        {
            //printf("\nmodulator_11a\n");
            modulator_11a(mod_symbols + NumOfDataSubcarriers,
                          sim_params.useLDPC ? punctured_bits : interl_bits,
                          sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits,
                          sim_params.bits_per_QAM_symbol);  // errors
        }
        else
        {
#ifdef ABL_TEST
            dump_char_array(sim_params.useLDPC ? punctured_bits : interl_bits, 2 * bits_per_OFDM_symbol, "Data_before_ABL_modulator.log", "Data_before_ABL_modulator");
#endif

            modulator_for_ABL(mod_symbols + NumOfDataSubcarriers,
                              sim_params.useLDPC ? punctured_bits : interl_bits,
                              sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits,
                              (unsigned int*)ABL_bits_per_QAM_symbol,
                              num_of_OFDM_symbols - 1);

#ifdef ABL_TEST
            dump_fxp_complex_array(mod_symbols, 3 * NumOfSubcarriers, "Data_after_ABL_modulator.log", "Data_after_ABL_modulator");
#endif
        }
        //]

        //[ Insert pilots symbols, ofdm_symbols precision is equal to fxp_params.QAM_precision
        insert_pilot_symbols_11a(ofdm_symbols, mod_symbols, num_of_OFDM_symbols);
        //]

        if (sim_params.advTraining)
        {
            memmove(ofdm_symbols + 3 * NumOfSubcarriers,
                    ofdm_symbols + NumOfSubcarriers,
                    (num_of_OFDM_symbols - 1) * NumOfSubcarriers * sizeof(fxp_complex));

            memset(ofdm_symbols + NumOfSubcarriers, 0, 2 * NumOfSubcarriers * sizeof(fxp_complex));

            for (j = 0; j < 2; j++)
                for (i = 0; i < NumOfUsedSubcarriers; i++)
                {
                    if (sim_consts.LongTrainingSymbols[i] > 0)
                        ofdm_symbols[(j + 1) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].re =
                                sim_consts.QAMSignalLevels[0][0][1];
                    else
                        ofdm_symbols[(j + 1) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].re =
                                sim_consts.QAMSignalLevels[0][0][0];

                    ofdm_symbols[(j + 1) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].im = 0;
                }

            num_of_OFDM_symbols += 2;
        }


#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols,
        NumOfSubcarriers,
        "freq_PLCP_header.log",
        "PLCP header in frequency domain");
#endif

#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols + NumOfSubcarriers,
        (num_of_OFDM_symbols - 1) * NumOfSubcarriers,
        "freq_DATA.log",
        "DATA in frequency domain");
#endif

        //[ Convert frequency domain symbols to time domain signal
        for (i = 0; i < num_of_OFDM_symbols; i++)
        {
            // Set ofdm_symbols precision to fxp_params.FFT_precision
            // and exp. position to fxp_params.FFT_exp_position to avoid any fxp convertions
            fft_with_mapping_11a(ofdm_symbols + i * NumOfSubcarriers, 6, 1,
                                 fxp_params.FFT_precision,
                                 fxp_params.FFT_exp_position,
                                 fxp_params.IFFT_shift_control);
            // After IFFT ofdm_symbols have (fxp_params.FFT_precision - fxp_params.IFFT_shift_control) precision,
            // after IFFT exp. position doesn't matter for it
        }
        //]

#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols,
        NumOfSubcarriers,
        "time_PLCP.log",
        "PLCP header in time domain");
#endif

#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols + NumOfSubcarriers,
        (num_of_OFDM_symbols - 1) * NumOfSubcarriers,
        "time_DATA.log",
        "DATA in time domain");
#endif

        //[ Add preamble
        add_preamble_11a(fxp_time_signal);

        //]

        //[ Add cyclic prefix
        add_cyclic_prefix_11a(fxp_time_signal + PreambleLength,
                              ofdm_symbols,
                              num_of_OFDM_symbols);
        //]

        num_of_samples = PreambleLength + num_of_OFDM_symbols *
                                          (CyclicPrefixLength + NumOfSubcarriers);

#ifdef TEST_SEQ
        dump_fxp_complex_array(fxp_time_signal,
        num_of_samples,
        "entire_packet.log",
        "DATA in time domain");
#endif
        *num_of_samples2 = num_of_samples;
            //printf(" num_of_samples = %d \n", num_of_samples);
        fxp_to_gsl((gsl_complex*)time_signal, fxp_time_signal, num_of_samples);

#ifdef LOGGING
        logging("Transmitter finished\n", -1);
#endif

        total_packets_time += num_of_samples / ( SampFreq * ( 1 << sim_params.sampling_power ) );
        //]

        FILE* filets = NULL;
        //filets = fopen("/home/elizar/RRFI/out/WLANT/timesignal.cfg", "w+");
        filets = fopen("timesignal.cfg", "w+");

        float col_tmp;
        float col_tmp2;


        std::vector<double> a{ 1.0, 2.0, 3.0 };
        std::vector<double> b{ 4.0, 5.0, 6.0 };
        std::vector<std::complex<double>> cvec(a.size());
        std::transform(a.begin(), a.end(), b.begin(), cvec.begin(), [](double da, double db)
        {
            return std::complex<double>(da, db);
        });

        float and12 = 0;
        for (int i = 0; i < *num_of_samples2 * 2; i++)
        {
            fprintf(filets, "%2.2f ", time_signal[i]); //timesignal
            Mybuff[i] = time_signal[i]; //
        }


        fprintf(filets, "\nnum_of_samples  %u\n", num_of_samples);//num of samples
        fprintf(filets, "ABL_bits_per_QAM_symbol");
        for (i = 0; i < 48; i++) {
            fprintf(filets, " %2.2f", ABL_bits_per_QAM_symbol[i]);//ABL_bits_per_QAM_symbol
        }
        fprintf(filets, "\nbits_per_OFDM_symbol  %u\n", bits_per_OFDM_symbol);//bits_per_OFDM_symbol
        fclose(filets);
    }

    //[ Free allocated buffers
    free( received_signal );
    free( cir );
    //]
    destroy_model();
    //printf("\nFile written");

    printf("\nMybuff\n");
    //for (int i = 300; i < 500; i++) {
    //    printf(" %f; ", Mybuff[i]);
    //}
    return Mybuff;
}

//static void MyReceiver1(fxp_complex fxp_time_signal[PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers)], int num_of_samples2, char* param)
//static void MyReceiver1(std::vector<std::complex<float>> fxp_time_signal, int sizebuff3, char* param)

//static void MyReceiver1(std::vector<std::complex<float>> fxp_time_signal, int sizebuff3, char* param, char &bufout)
static void MyReceiver1(std::vector<std::complex<float>> fxp_time_signal, int sizebuff3, char* param, char bufout[])
{
    printf("    Work MyReceiver...\n");
    //for (int i = 0; i < sizebuff3; i++) {
    //    printf(" %f; ", fxp_time_signal[i]);
    //}

    int SNR_count;
    float* SNRs;
    int channel_type;
    ReadInt("General", "SNR_count", param, "decimal", &SNR_count);
    SNRs = (float*)malloc(SNR_count * sizeof(float));
    ReadMultiFloat("General", "SNRs", param, SNR_count, SNRs);
    ReadInt("General", "channel_type", param, "decimal", &channel_type);


    //std::complex<float> Mybuff5[(1 << MaxSamplingPower) *
    //    (PreambleLength + MaxNumOfSymbols *
    //        (CyclicPrefixLength + NumOfSubcarriers))];
    std::complex<float> Mybuff5[10000];
    //std::complex<float> Mybuff5[5000];
    unsigned num_of_samples;
    //std::complex<float> Mybuff4[(ExtraNoiseSamples + (1 << MaxSamplingPower) * ((PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers)) + 1 - 1))];
    std::complex<float> Mybuff4[10000];

    fxp_complex fxp_time_signal2[PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers)];

    for (int i = 0; i < sizebuff3*2; i++) // очень долго
    {
        Mybuff5[i] = fxp_time_signal[i];
    }

    gsl_to_fxp((gsl_complex*)Mybuff5, fxp_time_signal2, sizebuff3 * 2);

    //printf("\ngsl_to_fxp\n");

        unsigned i;

        char log_filename[30];
        char instant_log_filename[30];
        double time_last_dump = 0;

        unsigned char inf_bits[MaxPacketSize];
        unsigned char out_bits[MaxPacketSize];

        gsl_complex* cir = NULL;
        unsigned ir_duration;

        gsl_complex* received_signal2;
        ir_duration = (Routine_get_channel_ir_len[channel_type])();
        cir = (gsl_complex*)malloc(ir_duration * sizeof(gsl_complex));
        received_signal2 = (gsl_complex*)malloc((ExtraNoiseSamples +
            (1 << MaxSamplingPower) * (
                (PreambleLength + MaxNumOfSymbols *
                    (CyclicPrefixLength + NumOfSubcarriers)) +
                ir_duration - 1))
            * sizeof(gsl_complex));

        unsigned num_of_received_samples;

        unsigned iter;
        unsigned bits_transmitted = 0;
        unsigned packets_transmitted = 0;
        unsigned error_bits = 0;
        unsigned error_packets = 0;
        unsigned char packet_error;
        unsigned bits_per_OFDM_symbol;

        double throughput;
        double data_throughput;
        double totalMps = 0;

        unsigned bits_transmitted_Sync = 0;
        unsigned error_bits_Sync = 0;

        double total_packets_time = 0;
        unsigned packet_detection_errors = 0;


        int ABL_bits_per_symbol[NumOfDataSubcarriers];
        char syncError;

        unsigned feedBackNumOfOFDMSyms;
        unsigned averNumOfOFDMSyms = 0;
        unsigned allNumOfOFDMSyms = 0;
        unsigned maxNumOfOFDMSyms = 0;

        FILE* file = NULL;

        //[ Initialize ABL parameters
        for (i = 0; i < NumOfDataSubcarriers; i++)
        {
            ABL_bits_per_symbol[i] = sim_params.bits_per_QAM_symbol;
        }

        bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
        //]

//
//            if (sim_params.useLDPC)
//            {
//                totalMps += bits_per_OFDM_symbol * 0.75 *
//                    (double)SampFreq / (CyclicPrefixLength + NumOfSubcarriers) / 10e5;
//            }
//            else
//            {
//                totalMps += bits_per_OFDM_symbol * sim_params.code_rate *
//                    (double)SampFreq / (CyclicPrefixLength + NumOfSubcarriers) / 10e5;
//            }
//
//            if (!sim_params.useABL)
//            {
//                bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
//            }
//            printf("P5\n");
//
//
    printf("sizebuff3 = %d\n", sizebuff3);
            receiver((fxp_complex*)fxp_time_signal2,
                sizebuff3,
                out_bits,
                (unsigned int*)ABL_bits_per_symbol,
                &bits_per_OFDM_symbol,                // MaxBitsPerQAMSymbol
                &syncError,
                &feedBackNumOfOFDMSyms);

            int ii = 8;
            int chet = 0;
            int chet1 = 0;
            int and1 = 0;
            int k = 1;
            int per = 0;
            int per1 = 0;
            i = 0;
            unsigned char mas1[MaxPacketSize];
            while ((ii < MaxPacketSize))
            {
                while(chet<ii)
                {
                    per1 = pow(2, chet1);
                    per1 = per1*out_bits[chet];
                    printf(" (%d) ", per1);
                    per = per + per1;
                    printf(" %u ", out_bits[chet]);
                    chet++;
                    chet1++;
                }
                printf(" -- %u ", per);
                printf("\n");
                //mas1[i] = per;
                bufout[i] = per;
                if(per == '\0')
                    break;
                per = 0;
                chet1 = 0;
                ii = ii+8;
                i++;
            }
            printf("\n");

            //]
            FILE* recevier11 = NULL;
            recevier11 = fopen("recevier11.bin", "wb");
            for (int i = 0; i < MaxPacketSize; i++)
            {
                if ((i != 0) && (i % 8 == 0))
                {
                    //fprintf(recevier11, "\n");
                }
                fprintf(recevier11, "%d ", out_bits[i]);//ttimesignal
            }


         /*   if (syncError)
        //    {
        //        packet_detection_errors++;
        //    }

        //    packet_error = 0;

          //  for (i = 0; i < 8 * packet_size; i++)
          //  {
          //      if (inf_bits[i] != out_bits[i])
          //      {
           //         error_bits++;

           //         if (!syncError)
           //         {
           //             error_bits_Sync++;
           //         }

             //       packet_error = 1;
              //  }
            //}
            //printf("P7\n");
            //if (packet_error)
            {
                error_packets++;
            }

            bits_transmitted += 8 * packet_size;
            packets_transmitted++;

            if (!syncError)
            {
                bits_transmitted_Sync += 8 * packet_size;
            }

            printf("P8\n");
            if (sim_params.useABL)
            {
                throughput = totalMps / packets_transmitted;

                if (maxNumOfOFDMSyms < feedBackNumOfOFDMSyms)
                {
                    maxNumOfOFDMSyms = feedBackNumOfOFDMSyms;
                }
                allNumOfOFDMSyms += feedBackNumOfOFDMSyms;
            }
            else
            {
                throughput = sim_params.data_rate;
            }

            data_throughput = (packets_transmitted - error_packets) *
                8 * packet_size / total_packets_time / 10e5;
            printf("P9\n");
            if ((time(NULL) - time_last_dump >= 2) || (iter == sim_params.packets_count - 1))
            {
                time_last_dump = time(NULL);

                printf("Iteration=%d, TransmittedBits=%d, ErrorBits=%d, TransmittedFrames=%d,"
                    "ErrorFrames=%d, SNR=%f, BER=%e, BERNoSyncError=%e, FER=%e, Throughput=%2.2f, DataThroughput=%2.2f, PacketDetectionErrors=%d\n",
                    iter + 1, bits_transmitted, error_bits, packets_transmitted, error_packets,
                    SNR, (double)error_bits / (double)bits_transmitted, (double)error_bits_Sync / (double)bits_transmitted_Sync,
                    (double)error_packets / (double)packets_transmitted, throughput, data_throughput, packet_detection_errors);

                if (sim_params.useABL)
                    printf("AverageFeedBack=%2.2f, MaxFeedBack=%d\n", (double)allNumOfOFDMSyms / (double)packets_transmitted, maxNumOfOFDMSyms);
                printf("P10\n");
                file = fopen(instant_log_filename, "w");
                printf("P11111\n");
                fprintf(file, "Iteration=%d\n TransmittedBits=%d\n ErrorBits=%d\n"
                    "TransmittedFrames=%d\n ErrorFrames=%d\n SNR=%f\n BER=%e\n BERNoSyncError=%e\n FER=%e\n"
                    " Throughput=%2.2f\n DataThroughput=%2.2f\n PacketDetectionErrors=%d\n",
                    iter + 1, bits_transmitted, error_bits, packets_transmitted, error_packets,
                    SNR, (double)error_bits / (double)bits_transmitted, (double)error_bits_Sync / (double)bits_transmitted_Sync,
                    (double)error_packets / (double)packets_transmitted, throughput, data_throughput, packet_detection_errors);
                printf("P2222222\n");
                if (sim_params.useABL)
                    fprintf(file, " AverageFeedBack=%2.2f\n MaxFeedBack=%d\n", (double)allNumOfOFDMSyms / (double)packets_transmitted, maxNumOfOFDMSyms);
                fclose(file);
                printf("P333333\n");
                file = NULL;
            }
        }

        printf("P11\n");
        //return received_signal;
        FILE* received_sig;
        printf("P123\n");
        received_sig = fopen("received_signal.txt", "w");
        printf("P12\n");
        //for (int i = 0; i < num_of_samples; i++) {
        for (int i = 0; i < num_of_samples; i++) {
            fprintf(received_sig, "%2.2f ", received_signal[i]);
            printf("%2.2f ", received_signal[i]);//ttimesignal
        }
        printf("\n time_signal \n");
        fprintf(received_sig, "\n time_signal \n");
        for (int i = 0; i < num_of_samples; i++) {
            fprintf(received_sig, "%2.2f ", time_signal[i]);
            printf("%2.2f ", time_signal[i]);//ttimesignal
        }
        printf("P13\n");
        //[ Free allocated buffers
        free(received_signal);
        printf("P14\n");
        free(cir);
        //]


        file = fopen(log_filename, "w");
        fprintf(file, "Iteration=%d\n TransmittedBits=%d\n ErrorBits=%d\n"
            "TransmittedFrames=%d\n ErrorFrames=%d\n SNR=%f\n BER=%e\n BERNoSyncError=%e\n FER=%e\n"
            " Throughput=%2.2f\n DataThroughput=%2.2f\n PacketDetectionErrors=%d\n",
            iter + 1, bits_transmitted, error_bits, packets_transmitted, error_packets,
            SNR, (double)error_bits / (double)bits_transmitted, (double)error_bits_Sync / (double)bits_transmitted_Sync,
            (double)error_packets / (double)packets_transmitted, throughput, data_throughput, packet_detection_errors);
        fclose(file);
        file = NULL;

        destroy_model();*/
}
































/*
//static std::vector<short> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count)
static std::vector<std::complex<float>> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, int *num_of_samples2)
//static std::vector<short> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, int* num_of_samples2)
{
    unsigned int i;
    char log_filename[30];
    char instant_log_filename[30];
    double time_last_dump = 0;

    unsigned char inf_bits[MaxPacketSize];
    unsigned char out_bits[MaxPacketSize];
    std::complex<float> time_signal[(1 << MaxSamplingPower) *
                                    (PreambleLength + MaxNumOfSymbols *
                                                      (CyclicPrefixLength + NumOfSubcarriers))];

    //gsl_complex *time_signal[(1 << MaxSamplingPower) *
    //                      ( PreambleLength + MaxNumOfSymbols *
    //                      ( CyclicPrefixLength + NumOfSubcarriers ) )];

    gsl_complex* cir = NULL;
    unsigned ir_duration;

    unsigned num_of_samples;

    gsl_complex* received_signal;
    unsigned num_of_received_samples;
    unsigned iter;
    unsigned bits_transmitted = 0;
    unsigned packets_transmitted = 0;
    unsigned error_bits = 0;
    unsigned error_packets = 0;
    unsigned char packet_error;
    unsigned bits_per_OFDM_symbol;
    double throughput;
    double data_throughput;
    double totalMps = 0;

    unsigned bits_transmitted_Sync = 0;
    unsigned error_bits_Sync = 0;

    double total_packets_time = 0;
    unsigned packet_detection_errors = 0;

    int ABL_bits_per_QAM_symbol[NumOfDataSubcarriers];
    char syncError;

    unsigned feedBackNumOfOFDMSyms;
    unsigned averNumOfOFDMSyms = 0;
    unsigned allNumOfOFDMSyms = 0;
    unsigned maxNumOfOFDMSyms = 0;

    FILE* file=NULL;
    std::vector<std::complex<float>> Mybuff(MaxPacketSize); //���
    std::vector<std::complex<float>> Mybuff2(MaxPacketSize); //���
    //std::vector<short> Mybuff(MaxPacketSize); //���

    //[ Initialize simulation parameters
    sim_params.data_rate = data_rate;
    sim_params.useLDPC = use_ldpc;
    sim_params.channel_type = channel_type;
    sim_params.packets_count = packet_count;
    sprintf( log_filename, "log%f.log", SNR );
    sprintf( instant_log_filename, "__I_log%f.log", SNR );
    initialize_model();
    //]

    //[ Allocate channel signal and IR storage
    ir_duration = (Routine_get_channel_ir_len[channel_type])();
    cir = (gsl_complex*)malloc(ir_duration * sizeof(gsl_complex));
    received_signal = (gsl_complex*)malloc((ExtraNoiseSamples +
                                            (1 << MaxSamplingPower) * (
                                                    (PreambleLength + MaxNumOfSymbols *
                                                                      (CyclicPrefixLength + NumOfSubcarriers)) +
                                                    ir_duration - 1))
                                           * sizeof(gsl_complex));
    //]

    //[ Initialize ABL parameters
    for ( i = 0; i < NumOfDataSubcarriers; i++ )
    {
        ABL_bits_per_QAM_symbol[i] = sim_params.bits_per_QAM_symbol;
    }

    bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
    //]
    printf(" sim_params.bits_per_QAM_symbol = %d \n", sim_params.bits_per_QAM_symbol);
    printf(" bits_per_OFDM_symbol = %u \n", bits_per_OFDM_symbol);
    printf(" NumOfDataSubcarriers = %d \n", NumOfDataSubcarriers);
    //[ABL Test
#ifdef ABL_TEST
    for ( i = 0; i < 48; i++ )
	{
      ABL_bits_per_QAM_symbol[i] = ABL_bits_per_QAM_symbol_4[i];
	}

    bits_per_OFDM_symbol = 0;
    for ( i = 0; i < 48; i++ )
	{
	  bits_per_OFDM_symbol += ABL_bits_per_QAM_symbol[i];
	}
#endif
    //]


    for ( iter = 0; iter < sim_params.packets_count; iter++ )
    {
#ifdef LOGGING
        sprintf( overflow_comment, "Iteration=%d\n", iter + 1 );
        logging( overflow_comment, 0 );
#endif

#ifdef TEST_SEQ
        for ( i = 0; i < 8 * packet_size; i++ )
		{
			inf_bits[i] = ( test_message[ i / 8 ] >> ( i % 8 ) ) & 1;
		}
#else
        FILE* data;
        data = fopen(FILE_PATH, "r+");
        unsigned char mas1[MaxPacketSize];
        char cc;
        int k = 1; // 00000001
        int kk = 128; // 10000000
        int chet = 0;
        int and1;
        static int64_t file_size = getFileSize(FILE_PATH);
        printf("File size: %d \n", file_size);
        for (int i = 0; i < file_size; i++) // i = ���������� ����
        {
            while ((cc = fgetc(data)) != EOF)
            {
                mas1[i] = cc;
                printf(" %u ", mas1[i]);
                printf(" - ");
                for (int ii = 0; ii < 8; ii++)
                {
                    and1 = (mas1[i] & k);
                    //and1 = (mas1[i] & kk);
                    mas1[i] >>= k;
                    //mas1[i] <<= kk;
                    inf_bits[chet] = and1;
                    printf(" %u ", inf_bits[chet]);
                    chet++;
                }
                printf("\n");
            }
        }
        fclose(data);

        //[ Generate random data
        //	for ( i = 0; i < 8 * packet_size; i++ )
        //    {
        //        inf_bits[i] = gsl_ran_bernoulli(RNG,0.5);
        //        printf(" %u ", inf_bits[i]);
         //   }
        //]
#endif
        if ( sim_params.useLDPC )
        {
            totalMps += bits_per_OFDM_symbol * 0.75 *
                        (double) SampFreq / ( CyclicPrefixLength + NumOfSubcarriers ) / 10e5;
        }
        else
        {
            totalMps += bits_per_OFDM_symbol * sim_params.code_rate *
                        (double) SampFreq / ( CyclicPrefixLength + NumOfSubcarriers ) / 10e5;
        }

        if ( !sim_params.useABL )
        {
            bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
        }

        //[ Modulate data and prepare it for transmission

    ////////////////////////////////////////////////////////////////////////////////
    // Name: transmitter
    //
    // Purpose: Implements OFDM transmitter scheme: FEC coding, modulation,
    //					adding preamble and training symbols, generating time domain signal
    //
    // Author: Alexandr Kozlov
    //
    // Parameters: time_signal - output, Time domain signal stores
    //
    //						 inf_bits - input, data bits to be transmitted
    //
    //             packet_size - input, Packet size in bytes
    //
    //             ABL_bits_per_QAM_symbol - input, Subcarriers modulation for ABL
    //
    //             bits_per_OFDM_symbol - input, Coded bits per OFDM symbol
    //
    // Returning value: Number of time domain samples
    ////////////////////////////////////////////////////////////////////////////////
        unsigned j;
        unsigned char data_bits[MaxPacketSize + LDPCMessageLength];
        unsigned char coded_bits[MaxCodedPacketSize];
        unsigned char interl_bits[MaxCodedPacketSize];
        unsigned char punctured_bits[MaxCodedPacketSize];

        unsigned num_of_coded_bits;
        unsigned num_of_OFDM_symbols;
        unsigned num_of_pad_bits = 0;
        unsigned num_of_samples;
        //unsigned num_of_samples2;
        unsigned num_of_LDPC_codewords;
        unsigned num_of_LDPC_zerobits;

        fxp_complex mod_symbols[48 + MaxCodedPacketSize]; // PLCP symbol + data symbols
        fxp_complex ofdm_symbols[MaxNumOfSymbols * NumOfSubcarriers];

        fxp_complex fxp_time_signal[PreambleLength + MaxNumOfSymbols *
                                                     (CyclicPrefixLength + NumOfSubcarriers)];

        unsigned num_of_total_bits;

#ifdef LOGGING
        logging("Transmitter...\n", 1);
#endif

        //[ Prepare PLCP header
#ifdef LOGGING
        logging("Prepare PLCP header...\n", 1);
#endif
        memset(data_bits, 0, 24);
        get_PLCP_header(data_bits, packet_size);
#ifdef TEST_SEQ
        dump_char_array(data_bits, 24, "PLCP_header.log", "PLCP header");
#endif

        if (sim_params.useGolay)
        {
            GolayEncode(golay_handle, data_bits, coded_bits);
            GolayEncode(golay_handle, data_bits + 12, coded_bits + 24);
        }
        else
            conv_encoder_11a(data_bits, 24, coded_bits);

#ifdef TEST_SEQ
        dump_char_array(coded_bits, 48, "PLCP_header_coded.log", "PLCP header after encoding");
#endif

        interleaver_11a(coded_bits, interl_bits, 48, 48);

#ifdef TEST_SEQ
        dump_char_array(interl_bits, 48, "PLCP_header_interl.log", "PLCP header after interleaving");
#endif

        modulator_11a(mod_symbols, interl_bits, 48, 1);

#ifdef LOGGING
        logging("Prepare PLCP header finished\n", -1);
#endif
        //]

        //[ Add SERVICE field to data bits and scramble its
        memset(data_bits, 0, 16);
        memcpy(data_bits + 16, inf_bits, 8 * packet_size);
        //]

        //[ Add tail bits
        memset(data_bits + 16 + 8 * packet_size, 0, 6);
        //]
        //[ Padding
        if (!sim_params.useLDPC && !sim_params.useGolay)
        {
            //[ Add padding bits to the information bits in convolutional code case
            num_of_OFDM_symbols = (unsigned)ceil((double)(16 + 8 * packet_size + 6) /
                                                 (double)(bits_per_OFDM_symbol * sim_params.code_rate));

            num_of_pad_bits = (unsigned)(num_of_OFDM_symbols * bits_per_OFDM_symbol * sim_params.code_rate -
                                         (16 + 8 * packet_size + 6));
            for (i = 0; i < num_of_pad_bits; i++)
            {
                data_bits[16 + 8 * packet_size + 6 + i] = 0;
            }
            num_of_total_bits = 16 + 8 * packet_size + 6 + num_of_pad_bits;
            //]
        }
        else
        if (sim_params.useGolay)
        {
            //[ Add padding bits to the information bits in Golay code case
            num_of_OFDM_symbols = (unsigned)ceil((double)(16 + 8 * packet_size) /
                                                 (double)(bits_per_OFDM_symbol * 0.5));

            num_of_pad_bits = (unsigned)(num_of_OFDM_symbols * bits_per_OFDM_symbol * 0.5 -
                                         (16 + 8 * packet_size));

            for (i = 0; i < num_of_pad_bits; i++)
            {
                data_bits[16 + 8 * packet_size + i] = 0;
            }

            num_of_total_bits = 16 + 8 * packet_size + num_of_pad_bits;
            //]
        }
        else
        {
            //[ Add padding bits to the information bits in LDPC code case
            num_of_LDPC_zerobits = LDPCMessageLength - (8 * packet_size + 16) % LDPCMessageLength;

            if (num_of_LDPC_zerobits == LDPCMessageLength)
            {
                num_of_LDPC_codewords = (8 * packet_size + 16) / LDPCMessageLength;
            }
            else
            {
                num_of_LDPC_codewords = (8 * packet_size + 16) / LDPCMessageLength + 1;
            }

            //[ Add padding bits for last message
            for (i = 0; i < num_of_LDPC_zerobits; i++)
            {
                data_bits[num_of_LDPC_codewords * LDPCMessageLength -
                          num_of_LDPC_zerobits + i] = 0;
            }
            //]

            //[ Add pad bits for LDPC case due to LDPC codeword length may not divide by NumOfDataSubcarriers
            num_of_coded_bits = num_of_LDPC_codewords * LDPCCodewordLength;

            num_of_OFDM_symbols = (unsigned)ceil((double)num_of_coded_bits /
                                                 bits_per_OFDM_symbol);

            num_of_pad_bits = num_of_OFDM_symbols * bits_per_OFDM_symbol - num_of_coded_bits;

            for (i = 0; i < num_of_pad_bits; i++)
            {
                data_bits[num_of_LDPC_codewords * LDPCMessageLength + i] = 0;
            }
            //]

            num_of_total_bits = num_of_LDPC_codewords * LDPCMessageLength + num_of_pad_bits;
            //]
        }
        //]
#ifdef TEST_SEQ
        dump_char_array(data_bits,
        num_of_total_bits,
        "data_bits.log",
        "DATA bits");
#endif
        //[ Scrambling
        scrambler_11a(data_bits, num_of_total_bits, &tx_scrambler_state, 0);
        //]
        if (!sim_params.useLDPC && !sim_params.useGolay)
        {
            //[ Set tail bits to zero
            memset(data_bits + 16 + 8 * packet_size, 0, 6);
            //]
        }

        //]

#ifdef TEST_SEQ
        dump_char_array(data_bits,
        num_of_total_bits,
        "data_bits_scrambled.log",
        "DATA bits after scrambling");
#endif

        //[ Encode data
        if (!sim_params.useLDPC && !sim_params.useGolay)
        {
            //[ Encode data using convolutional code
            num_of_coded_bits =
                    conv_encoder_11a(data_bits, 16 + 8 * packet_size + 6 + num_of_pad_bits, coded_bits);
            num_of_coded_bits =
                    puncturer_11a(coded_bits,
                                  num_of_coded_bits,
                                  punctured_bits,
                                  sim_params.code_rate);

#ifdef TEST_SEQ
            dump_char_array(punctured_bits,
            num_of_coded_bits,
            "data_bits_coded.log",
            "Coded DATA bits");
#endif
            //]
        }
        else
        if (sim_params.useGolay)
        {
            // Golay encode
            num_of_coded_bits = 2 * num_of_total_bits;
            for (i = 0; i < 2 * num_of_OFDM_symbols; i++)
            {
                GolayEncode(golay_handle, data_bits + 12 * i, punctured_bits + 24 * i);
            }
        }
        else
        {
            //[ Encode data using  LDPC code
            for (i = 0; i < num_of_LDPC_codewords; i++)
            {
                //ldpc_encode(data_bits + i * LDPCMessageLength,
                //    punctured_bits + i * LDPCCodewordLength);
                ldpc_encode((const char*)data_bits + i * LDPCMessageLength,
                            (char*)punctured_bits + i * LDPCCodewordLength);
            }

            memcpy(punctured_bits + num_of_LDPC_codewords * LDPCCodewordLength,
                   data_bits + num_of_LDPC_codewords * LDPCMessageLength, num_of_pad_bits);

#ifdef TEST_SEQ
            dump_char_array(punctured_bits,
                num_of_coded_bits + num_of_pad_bits,
                "data_bits_coded_LDPC.log",
                "LDPC Coded DATA bits");
#endif
            //]
        }
        //]

        if (!sim_params.useLDPC)
        {
            //[ Interleave bits
            interleaver_11a(punctured_bits,
                            interl_bits,
                            num_of_OFDM_symbols * bits_per_OFDM_symbol,
                            bits_per_OFDM_symbol);

#ifdef TEST_SEQ
            dump_char_array(interl_bits,
            num_of_coded_bits,
            "data_bits_inter.log",
            "DATA bits after interleaving");
#endif
            //]
        }

        num_of_OFDM_symbols++; // due to PLCP header symbol

        //[ Modulate data bits to the mod_symbols array, mod_symbols precision is equal to fxp_params.QAM_precision
        if (!sim_params.useABL)
        {
            modulator_11a(mod_symbols + NumOfDataSubcarriers,
                          sim_params.useLDPC ? punctured_bits : interl_bits,
                          sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits,
                          sim_params.bits_per_QAM_symbol);
        }
        else
        {
#ifdef ABL_TEST
            dump_char_array(sim_params.useLDPC ? punctured_bits : interl_bits, 2 * bits_per_OFDM_symbol, "Data_before_ABL_modulator.log", "Data_before_ABL_modulator");
#endif

            //modulator_for_ABL(mod_symbols + NumOfDataSubcarriers,
            //    sim_params.useLDPC ? punctured_bits : interl_bits,
            //    sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits,
            //    ABL_bits_per_QAM_symbol,
            //    num_of_OFDM_symbols - 1);
            modulator_for_ABL(mod_symbols + NumOfDataSubcarriers,
                              sim_params.useLDPC ? punctured_bits : interl_bits,
                              sim_params.useLDPC ? num_of_coded_bits + num_of_pad_bits : num_of_coded_bits,
                              (unsigned int*)ABL_bits_per_QAM_symbol,
                              num_of_OFDM_symbols - 1);

#ifdef ABL_TEST
            dump_fxp_complex_array(mod_symbols, 3 * NumOfSubcarriers, "Data_after_ABL_modulator.log", "Data_after_ABL_modulator");
#endif
        }
        //]

        //[ Insert pilots symbols, ofdm_symbols precision is equal to fxp_params.QAM_precision
        insert_pilot_symbols_11a(ofdm_symbols, mod_symbols, num_of_OFDM_symbols);
        //]

        if (sim_params.advTraining)
        {
            memmove(ofdm_symbols + 3 * NumOfSubcarriers,
                    ofdm_symbols + NumOfSubcarriers,
                    (num_of_OFDM_symbols - 1) * NumOfSubcarriers * sizeof(fxp_complex));

            memset(ofdm_symbols + NumOfSubcarriers, 0, 2 * NumOfSubcarriers * sizeof(fxp_complex));

            for (j = 0; j < 2; j++)
                for (i = 0; i < NumOfUsedSubcarriers; i++)
                {
                    if (sim_consts.LongTrainingSymbols[i] > 0)
                        ofdm_symbols[(j + 1) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].re =
                                sim_consts.QAMSignalLevels[0][0][1];
                    else
                        ofdm_symbols[(j + 1) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].re =
                                sim_consts.QAMSignalLevels[0][0][0];

                    ofdm_symbols[(j + 1) * NumOfSubcarriers + sim_consts.UsedSubcIdx[i] - 1].im = 0;
                }

            num_of_OFDM_symbols += 2;
        }


#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols,
        NumOfSubcarriers,
        "freq_PLCP_header.log",
        "PLCP header in frequency domain");
#endif

#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols + NumOfSubcarriers,
        (num_of_OFDM_symbols - 1) * NumOfSubcarriers,
        "freq_DATA.log",
        "DATA in frequency domain");
#endif

        //[ Convert frequency domain symbols to time domain signal
        for (i = 0; i < num_of_OFDM_symbols; i++)
        {
            // Set ofdm_symbols precision to fxp_params.FFT_precision
            // and exp. position to fxp_params.FFT_exp_position to avoid any fxp convertions
            fft_with_mapping_11a(ofdm_symbols + i * NumOfSubcarriers, 6, 1,
                                 fxp_params.FFT_precision,
                                 fxp_params.FFT_exp_position,
                                 fxp_params.IFFT_shift_control);
            // After IFFT ofdm_symbols have (fxp_params.FFT_precision - fxp_params.IFFT_shift_control) precision,
            // after IFFT exp. position doesn't matter for it
        }
        //]

#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols,
        NumOfSubcarriers,
        "time_PLCP.log",
        "PLCP header in time domain");
#endif

#ifdef TEST_SEQ
        dump_fxp_complex_array(ofdm_symbols + NumOfSubcarriers,
        (num_of_OFDM_symbols - 1) * NumOfSubcarriers,
        "time_DATA.log",
        "DATA in time domain");
#endif

        //[ Add preamble
        add_preamble_11a(fxp_time_signal);
        //]

        //[ Add cyclic prefix
        add_cyclic_prefix_11a(fxp_time_signal + PreambleLength,
                              ofdm_symbols,
                              num_of_OFDM_symbols);
        //]

        num_of_samples = PreambleLength + num_of_OFDM_symbols *
                                          (CyclicPrefixLength + NumOfSubcarriers);

#ifdef TEST_SEQ
        dump_fxp_complex_array(fxp_time_signal,
        num_of_samples,
        "entire_packet.log",
        "DATA in time domain");
#endif

        //num_of_samples2 = num_of_samples;
        *num_of_samples2 = num_of_samples;
        printf("\n\nnum_of_samples1 = %d \n", *num_of_samples2);

        fxp_to_gsl((gsl_complex*)time_signal, fxp_time_signal, num_of_samples);

        printf("\nstd::complex<float>_time_signal\n");
        //for (int k = 0; k < num_of_samples2*2; k++)
        for (int k = 0; k < *num_of_samples2 * 2; k++)
        {
            printf(" %f; ", time_signal[k]);
        }

#ifdef LOGGING
        logging("Transmitter finished\n", -1);
#endif

        //return num_of_samples;//
        //num_of_samples = transmitter((gsl_complex*)time_signal,
        //num_of_samples = transmitter((gsl_complex*)fxp_time_signal,
        //                              inf_bits,
        //                              packet_size,
        //                              (unsigned int*)ABL_bits_per_QAM_symbol,
        //                             bits_per_OFDM_symbol );

        total_packets_time += num_of_samples / ( SampFreq * ( 1 << sim_params.sampling_power ) );
        //]

        FILE* filets = NULL;
        filets = fopen("timesignal.cfg", "w+");
        printf("\n\nMybuff - std::vector<std::complex<float>>\n");//ttimesignal
        float col_tmp;
        float col_tmp2;


        std::vector<double> a{ 1.0, 2.0, 3.0 };
        std::vector<double> b{ 4.0, 5.0, 6.0 };
        std::vector<std::complex<double>> cvec(a.size());
        std::transform(a.begin(), a.end(), b.begin(), cvec.begin(), [](double da, double db)
        {
            return std::complex<double>(da, db);
        });

        float and12 = 0;
        for (int i = 0; i < *num_of_samples2 * 2; i++) {
            fprintf(filets, "%2.2f ", time_signal[i]); //timesignal
            Mybuff[i] = time_signal[i]; // �� �������� - � ������ ��������
            //Mybuff2[i] = time_signal[i];
            //and12 = time_signal[i]._Val[1];
            //Mybuff[i] = and12;
            //and12 = 0.182;
            //Mybuff[i] = and12;
            printf("%f =", Mybuff[i]);//ttimesignal �� �������� - � ������ ��������
            printf(" %d ", Mybuff[i]); //ttimesignal
        }
        //std::transform(Mybuff.begin(), Mybuff.end(), Mybuff2.begin(), [](float da, float db)
        //{
        //        return std::complex<float>(da, db);
        //});



        //fprintf(filets, "\nreceived_signal\n");
        //for (i = 0; i < num_of_samples; i++) {
        //    fprintf(filets, "%2.2f ", received_signal[i]);//ttimesignal
            //printf("%2.2f ", received_signal[i]);//ttimesignal
        //}
        //fprintf(filets, "\nfxp_time\n");
        //for (i = 0; i < 30000; i++) {
        //    fprintf(filets, "i=%d, received_signal %2.2f\n", i, received_signal[i]);//ttimesignal
        //}
        //for (i = 0; i < 3200; i++) {
        //    fprintf(filets, "i=%d, fxp_time_signal %2.2f\n", i, fxp_time_signal[i]);//fxp_time_signal
        //    Mybuff[i] = fxp_time_signal[i];
        //}
        //fprintf(filets, "fxp_params.FFT_precision = %u\n", fxp_params.FFT_precision);//FFT_precision
        //fprintf(filets, "fxp_params.FFT_exp_position = %u\n", fxp_params.FFT_exp_position);//FFT_precision

        fprintf(filets, "\nnum_of_samples  %u\n", num_of_samples);//num of samples
        fprintf(filets, "ABL_bits_per_QAM_symbol");
        for (i = 0; i < 48; i++) {
            fprintf(filets, " %2.2f", ABL_bits_per_QAM_symbol[i]);//ABL_bits_per_QAM_symbol
        }
        fprintf(filets, "\nbits_per_OFDM_symbol  %u\n", bits_per_OFDM_symbol);//bits_per_OFDM_symbol
        fclose(filets);
    }

    //[ Free allocated buffers
    free( received_signal );
    free( cir );
    //]
    destroy_model();
    printf("\nFile written \n");
    return Mybuff;
}*/