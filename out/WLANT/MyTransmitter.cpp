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
 * User: Eegorov      Date: 10.11.20   Time: 15:00
 * Updated in $/WLAN
 *
 * *****************  Version 9  *****************
 * User: Eegorov      Date: 10.07.21   Time: 12:00
 * Updated in $/WLAN
 *
 */
/*TRANSMITTER*/

#include <cmath>
#include <cstring>
#include "model.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include "channel.hpp"
#include "receiver.h"
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
#include "ldpc_encode.h"
#include "modulatorABL.h"
#include "golay_wrap.h"
#include <fstream>
#include <iomanip>
#include "out/WLANT/times.h"
using namespace std;

/*std::ofstream log_file("log_file_Transmitter.txt", std::ios_base::out);
long msec = 0;
char time_str[sizeof("1900-01-01 23:59:59")];
char time_str2[sizeof("1900-01-01 23:59:59")];
char time_str3[sizeof("1900-01-01 23:59:59")];*/

std::ofstream TEst_Test_Test("Test_signal.txt", std::ios_base::out);
std::ofstream filets("timesignal.cfg", std::ios_base::out);

//[ Test message from 802.11a standard
/*static unsigned char test_message[100] =
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
//]*/

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

static std::vector<std::complex<int16_t>> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, unsigned int *num_of_samples2, const char buf[]);

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"

static std::vector<std::complex<int16_t>> MyTransmitter(char* param, unsigned int *num_of_samples2, char buf[])
{
    std::vector<std::complex<int16_t>> buff(MaxPacketSize);
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

    log_file << "---------------------------------------------------------------------------------------------------------------" << endl;
    msec = times(time_str, time_str2, time_str3);
    log_file << left << setfill(' ') << setw(89) << " Start reading the configuration file " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

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
    cout << " Channel_type = " << channel_type << endl;


    sim_params.useABL = (char)use_ABL;
    sim_params.useJitter=use_jitter;
    sim_params.idealFrequencySync = (char)idealFrequencySync;
    sim_params.idealChannelEstimation = (char)idealChannelEstimation;
    sim_params.subcarriersPower = (char)subcarriersPower;
    sim_params.idealTiming = (char)idealTiming;
    sim_params.advTraining = (char)advTraining;
    setUpABLThresholds( ABL_thresholds );

    sim_params.useGolay = (char)useGolay;

	#ifdef AQ_LOG
	aq_log=fopen("aq_log.txt","w");
	fprintf(aq_log,"ACF\tThreshold\tRMS\n");
	fclose(aq_log);
    #endif

	#ifdef TEST_SEQ
	packet_size = 100;
	#endif

	for (int i = 0; i < SNR_count; i++) {
	    sim_params.packet_size = packet_size;
	    sim_params.curSNR = SNRs[0];
	    buff = simulate(SNRs[0], packet_size, data_rate, channel_type, use_ldpc, packet_count, num_of_samples2, buf);
	}
	free(SNRs);
    return buff;
}
#pragma clang diagnostic pop // Используется для решения ошибки при отправки в фун-ию ReadInt тип string, а нужен char*

static std::vector<std::complex<int16_t>> simulate(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, unsigned int *num_of_samples2, const char buf[])
{
    unsigned int i;
    char log_filename[30];
    char instant_log_filename[30];
    unsigned char inf_bits[MaxPacketSize];

    //std::complex<float> time_signal[...]; // было так, но выделили память теперь динамически
    /*auto *time_signal = new std::complex<float> [(1 << MaxSamplingPower) *
                                                 (PreambleLength + MaxNumOfSymbols *
                                                                   (CyclicPrefixLength + NumOfSubcarriers))];*/
    auto *time_signal = new std::complex<int16_t> [MaxPacketSize];

    //gsl_complex* cir = NULL;

    unsigned ir_duration;
    unsigned iter;
    unsigned bits_per_OFDM_symbol;
    double totalMps = 0;
    double total_packets_time = 0;
    unsigned int ABL_bits_per_QAM_symbol[NumOfDataSubcarriers];

    std::vector<std::complex<int16_t>> Mybuff(MaxPacketSize);

    //[ Initialize simulation parameters
    sim_params.data_rate = data_rate;
    sim_params.useLDPC = (char)use_ldpc;
    sim_params.channel_type = channel_type;
    sim_params.packets_count = packet_count;
    sprintf( log_filename, "log%f.log", SNR );
    sprintf( instant_log_filename, "__I_log%f.log", SNR );
    initialize_model();
    //]

    //[ Allocate channel signal and IR storage
    ir_duration = (Routine_get_channel_ir_len[channel_type])();
    //cir = (gsl_complex*)malloc(ir_duration * sizeof(gsl_complex));
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
        unsigned char mas1[DATAURA];
        char symbol;
        int mask = 1;      // 00000001 Use mask
        //int mask2 = 128;   // 10000000
        int chet = 0;
        int and1;
        int byte = 8;
        i = 0;

        while (i < DATAURA)
        {
            symbol = buf[i];
            mas1[i] = symbol;
            while(chet<byte)
            {
                and1 = (mas1[i] & mask);
                mas1[i] >>= mask;
                inf_bits[chet] = and1;
                chet++;
            }
            if(buf[i] == '\0')
                break;
            byte = byte+8;
            i++;
        }
        byte = 8;
        i = 0;
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

        cout << " \nABL_bits_per_QAM_symbol = " << *ABL_bits_per_QAM_symbol << endl;
        cout << " \nBits_per_OFDM_symbol = " << bits_per_OFDM_symbol << endl;

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
        unsigned num_of_pad_bits;
        unsigned num_of_samples;
        unsigned num_of_LDPC_codewords;
        unsigned num_of_LDPC_zerobits;

        fxp_complex mod_symbols[48 + MaxCodedPacketSize]; // PLCP symbol + data symbols
        fxp_complex ofdm_symbols[MaxNumOfSymbols * NumOfSubcarriers];
        fxp_complex fxp_time_signal[PreambleLength + MaxNumOfSymbols *
                                                     (CyclicPrefixLength + NumOfSubcarriers)];

        for(unsigned int pop = 0; pop < 48 + MaxCodedPacketSize; pop++)
        {
            mod_symbols[pop].re = 0;
            mod_symbols[pop].im = 0;
        }
        for(unsigned int pop = 0; pop < MaxNumOfSymbols * NumOfSubcarriers; pop++)
        {
            ofdm_symbols[pop].re = 0;
            ofdm_symbols[pop].im = 0;
        }
        for(unsigned int pop = 0; pop < PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers); pop++)
        {
            fxp_time_signal[pop].re = 0;
            fxp_time_signal[pop].im = 0;
        }
        cout << "MaxNumOfSymbols * NumOfSubcarriers = " << MaxNumOfSymbols * NumOfSubcarriers << endl;
        cout << "PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers) = " << PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers) << endl;

        //memset(mod_symbols, 0, 48 + MaxCodedPacketSize);

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
            msec = times(time_str, time_str2, time_str3);
            log_file << left << setfill(' ') << setw(89) << " Start GolayEncode " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

            GolayEncode(golay_handle, data_bits, coded_bits);
            GolayEncode(golay_handle, data_bits + 12, coded_bits + 24);
        }
        else
        {
            msec = times(time_str, time_str2, time_str3);
            log_file << left << setfill(' ') << setw(89) << " Start convolutional encoder " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
            conv_encoder_11a(data_bits, 24, coded_bits);
        }

#ifdef TEST_SEQ
        dump_char_array(coded_bits, 48, "PLCP_header_coded.log", "PLCP header after encoding");
#endif
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Start interleaver " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

        interleaver_11a(coded_bits, interl_bits, 48, 48);

#ifdef TEST_SEQ
        dump_char_array(interl_bits, 48, "PLCP_header_interl.log", "PLCP header after interleaving");
#endif
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Start modulation " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

        modulator_11a(mod_symbols, interl_bits, 48, 1);

        std::ofstream Modulator("Modulator1.txt", std::ios_base::out);
        for (int i = 0; i < 48; i++)
            Modulator << mod_symbols[i].re << " " << mod_symbols[i].im << endl;
        Modulator.close();

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
        else if (sim_params.useGolay)
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
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Start scrambler " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

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
        else if (sim_params.useGolay)
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
            printf("\n !sim_params.useABL modulator_11a\n");
            std::cout << "sim_params.bits_per_QAM_symbol = " << sim_params.bits_per_QAM_symbol << endl;
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
        std::ofstream Modulator2("Modulator2.txt", std::ios_base::out);
        for (int i = 0; i < 48 + MaxCodedPacketSize; i++)
            Modulator2 << mod_symbols[i].re << " " << mod_symbols[i].im << endl;
        Modulator2.close();
        //]

        //[ Insert pilots symbols, ofdm_symbols precision is equal to fxp_params.QAM_precision
        insert_pilot_symbols_11a(ofdm_symbols, mod_symbols, num_of_OFDM_symbols);
        //]

        std::ofstream pilot_symbols("Pilot_symbols.txt", std::ios_base::out);
        for (int i = 0; i < MaxNumOfSymbols * NumOfSubcarriers; i++)
            pilot_symbols << ofdm_symbols[i].re << " " << ofdm_symbols[i].im << endl;
        pilot_symbols.close();

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
        std::ofstream pilot_symbols2("Pilot_symbols2.txt", std::ios_base::out);
        for (int i = 0; i < MaxNumOfSymbols * NumOfSubcarriers; i++)
            pilot_symbols2 << ofdm_symbols[i].re << " " << ofdm_symbols[i].im << endl;
        pilot_symbols2.close();
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
        std::ofstream fxpcomplex("FxpComplex.txt", std::ios_base::out);
        for(int you = 0; you<num_of_samples; you++)
        {
            fxpcomplex << fxp_time_signal[you].re << " " << fxp_time_signal[you].im << endl;
        }
        fxpcomplex.close();

#ifdef TEST_SEQ
        dump_fxp_complex_array(fxp_time_signal,
        num_of_samples,
        "entire_packet.log",
        "DATA in time domain");
#endif
        *num_of_samples2 = num_of_samples;
        std::ofstream complex_signal("Complexsignal.txt", std::ios_base::out);
        for(unsigned count = 0; count < num_of_samples; count++ )
        {
            time_signal[count].imag(fxp_time_signal[count].im);
            time_signal[count].real(fxp_time_signal[count].re);
            complex_signal << time_signal[count].real() << " " << time_signal[count].imag() << endl;
        }
        complex_signal.close();

#ifdef LOGGING
        logging("Transmitter finished\n", -1);
#endif

        total_packets_time += num_of_samples / ( SampFreq * ( 1 << sim_params.sampling_power ) );
        //] End Transmitter

        std::vector<double> a{ 1.0, 2.0, 3.0 };
        std::vector<double> b{ 4.0, 5.0, 6.0 };
        std::vector<std::complex<double>> cvec(a.size());
        std::transform(a.begin(), a.end(), b.begin(), cvec.begin(), [](double da, double db)
        {
            return std::complex<double>(da, db);
        });

        for (i = 0; i < num_of_samples; i++)
        {
            //fprintf(filets, "%2.2f ", time_signal[i]); //ald
            filets << time_signal[i] << endl;
            Mybuff[i] = time_signal[i];
            TEst_Test_Test << Mybuff[i] << endl;
        }
        std::cout << " *num_of_samples2 = " << *num_of_samples2 << endl;
        for(unsigned count = 0; count < num_of_samples; count++)
        {
            time_signal[count].imag(0);
            time_signal[count].real(0);
        }
        //TEst_Test_Test.close();


        filets << "\n num_of_samples \n" << num_of_samples; //num of samples
        filets << "ABL_bits_per_QAM_symbol ";
        for (i = 0; i < 48; i++) {
            filets << ABL_bits_per_QAM_symbol[i]; //ABL_bits_per_QAM_symbol
        }
        filets << "\n bits_per_OFDM_symbol\n" << bits_per_OFDM_symbol; //bits_per_OFDM_symbol
    }
    //[ Free allocated buffers
    //free( cir );
    delete[] time_signal;
    //]
    destroy_model();

    cout << "\nThe data processing process is completed\n";
    log_file << "---------------------------------------------------------------------------------------------------------------" << endl;
    return Mybuff;
}
