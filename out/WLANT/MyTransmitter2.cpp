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
 */
/*TRANSMITTER*/

#include <cmath>
#include <cstring>//transm
#include "model.h"//transm
#include "sim_consts.h"//transm
#include "sim_params.h"//transm
#include "commonfunc.h"//transm
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
#include <cinttypes> // int64_t  printf
#include <fstream>
#include "fxp_to_gsl.cpp"
#include "gsl_to_fxp.cpp"
#include <ctime>
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

static std::vector<std::complex<float>> simulate1(double SNR, unsigned packet_size, unsigned data_rate, int channel_type, int use_ldpc, int packet_count, unsigned int *num_of_samples2, const char buf[BUF_SIZE]);

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

// ---------- My function //
static int64_t getFileSize(const char* file_name)
{
    int64_t _file_size = 0;
    FILE* fd = fopen(file_name, "rb");
    if (fd == nullptr) {
        _file_size = -1;
    }
    else {
        while (getc(fd) != EOF)
            _file_size++;
        fclose(fd);
    }
    return _file_size;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"  // Используется для решения ошибки при отправки в фун-ию ReadInt тип string, а нужен char*
static void MyReceiver1(std::vector<std::complex<float>> fxp_time_signal, unsigned int sizebuff3, char* param, char bufout[])
{
    time_t sec;
    sec = time (NULL);
    cout<<asctime(localtime(&sec));
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

    sim_params.useABL = (char)use_ABL;
    sim_params.useJitter=use_jitter;
    sim_params.idealFrequencySync = (char)idealFrequencySync;
    sim_params.idealChannelEstimation = (char)idealChannelEstimation;
    sim_params.subcarriersPower = (char)subcarriersPower;
    sim_params.idealTiming = (char)idealTiming;
    sim_params.advTraining = (char)advTraining;
    setUpABLThresholds( ABL_thresholds );

    sim_params.useGolay = (char)useGolay;

        for (int i = 0; i < SNR_count; i++)
        {
            sim_params.packet_size = packet_size;
            sim_params.curSNR = SNRs[0];
        }

    printf("    Work MyReceiver...\n");

    auto *Mybuff5 = new std::complex<float> [(1 << MaxSamplingPower) *
                                                 (PreambleLength + MaxNumOfSymbols *
                                                                   (CyclicPrefixLength + NumOfSubcarriers))];

    fxp_complex fxp_time_signal2[PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers)];

    for (int i = 0; i < sizebuff3*2; i++) // очень долго
    {
        Mybuff5[i] = fxp_time_signal[i];
    }

    gsl_to_fxp((gsl_complex*)Mybuff5, fxp_time_signal2, sizebuff3 * 2);

    //printf("\ngsl_to_fxp\n");

        unsigned i;

        //char log_filename[30];
        //char instant_log_filename[30];
        //double time_last_dump = 0;

        unsigned char out_bits[MaxPacketSize];
        //unsigned ir_duration;
        //ir_duration = (Routine_get_channel_ir_len[channel_type])();

        unsigned bits_per_OFDM_symbol;
        unsigned int ABL_bits_per_symbol[NumOfDataSubcarriers];
        char syncError;

        unsigned feedBackNumOfOFDMSyms;

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
            int per = 0;
            int per1;
            i = 0;
            while ((ii < MaxPacketSize))
            {
                while(chet<ii)
                {
                    per1 = (int)pow(2, chet1);
                    per1 = per1*out_bits[chet];
                    printf(" (%d) ", per1);
                    per = per + per1;
                    printf(" %u ", out_bits[chet]);
                    chet++;
                    chet1++;
                }
                printf(" -- %u ", per);
                printf("\n");
                bufout[i] = (char)per;
                if(per == '\0')
                    break;
                per = 0;
                chet1 = 0;
                ii = ii+8;
                i++;
            }
            printf("\n");

            //]

            std::ofstream recevier11("recevier11.bin", std::ios_base::out);

            for (i = 0; i < MaxPacketSize; i++)
            {
                if ((i != 0) && (i % 8 == 0))
                {
                    //fprintf(recevier11, "\n");
                }
                recevier11 << out_bits[i]; //bits_per_OFDM_symbol
            }
            recevier11.close();

         /*     if (syncError)
                {
                    packet_detection_errors++;
                }
                unsigned num_of_received_samples;
                unsigned iter;
                unsigned bits_transmitted = 0;
                unsigned packets_transmitted = 0;
                unsigned error_bits = 0;
                unsigned error_packets = 0;
                unsigned char packet_error;

                double throughput;
                double data_throughput;
                double totalMps = 0;
                unsigned bits_transmitted_Sync = 0;
                unsigned error_bits_Sync = 0;
                double total_packets_time = 0;
                unsigned packet_detection_errors = 0;

                unsigned averNumOfOFDMSyms = 0;
                unsigned allNumOfOFDMSyms = 0;
                unsigned maxNumOfOFDMSyms = 0;

                packet_error = 0;

              for (i = 0; i < 8 * packet_size; i++)
              {
                    if (inf_bits[i] != out_bits[i])
                    {
                     error_bits++;

                     if (!syncError)
                     {
                        error_bits_Sync++;
                     }

                       packet_error = 1;
                    }
             }
            printf("P7\n");
            if (packet_error)
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
        //for (int i = 0; i < num_of_samples; i++) {  // sizebuff3 вместо num_of_sample s
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
#pragma clang diagnostic pop