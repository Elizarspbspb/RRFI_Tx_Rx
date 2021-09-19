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
 * User: Eegorov      Date: 10.07.21   Time: 11:00
 * Updated in $/WLAN
 *
 */
/*Receiver*/

#include <cmath>
#include <cstring>
#include "model.h"
#include "sim_consts.h"
#include "sim_params.h"
#include "commonfunc.h"
#include "receiver.h"
#include "bitloading.h"
#include "DopplerChannel/FileUtils.h"
#include <fstream>
#include <iomanip>
#include "out/WLANT/times.h"

#define BUF_SIZE 8200

using namespace std;

std::ofstream log_file("log_file_RRFI_Tx_Rx.txt", std::ios_base::out);
long msec = 0;
char time_str[sizeof("1900-01-01 23:59:59")];
char time_str2[sizeof("1900-01-01 23:59:59")];
char time_str3[sizeof("1900-01-01 23:59:59")];

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"  // При отправки в фун-ию ReadInt тип string
static void MyReceiver(std::vector<std::complex<int16_t>> fxp_time_signal, unsigned int num_of_samples, char* param, char bufout[])
{
    time_t sec;
    sec = time (NULL);
    cout<<asctime(localtime(&sec));
    cout<<"Receiver" << endl;

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
    double totalMps = 0;

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
    for (int i = 0; i < SNR_count; i++) {
        sim_params.packet_size = packet_size;
        sim_params.curSNR = SNRs[0];
    }
    sim_params.data_rate = data_rate;
    sim_params.useLDPC = (char)use_ldpc;
    sim_params.channel_type = channel_type;
    sim_params.packets_count = packet_count;
    initialize_model();
    free(SNRs);
    cout << "    Work MyReceiver..." << endl;

    /*auto *Mybuff5 = new std::complex<float> [(1 << MaxSamplingPower) *
                                                 (PreambleLength + MaxNumOfSymbols *
                                                                   (CyclicPrefixLength + NumOfSubcarriers))];
    fxp_complex fxp_time_signal2[PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers)];*/

    auto *Mybuff5 = new std::complex<int16_t> [num_of_samples*8];
    fxp_complex fxp_time_signal2[num_of_samples*8];

    cout << "\n PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers) = " << PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers) << endl;
    cout << "(1 << MaxSamplingPower) * (PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers)) = " << (1 << MaxSamplingPower) * (PreambleLength + MaxNumOfSymbols * (CyclicPrefixLength + NumOfSubcarriers))<< endl;

    std::ofstream complex_signal("Complexsignal.txt", std::ios_base::out);
    for (int i = 0; i < num_of_samples; i++)
    {
        Mybuff5[i] = fxp_time_signal[i];
        complex_signal << Mybuff5[i].real() << " " << Mybuff5[i].imag() << endl;
    }
    complex_signal.close();

    std::ofstream fxpcomplex("FxpComplex.txt", std::ios_base::out);
    for(unsigned count = 0; count < num_of_samples; count++ )
    {
        fxp_time_signal2[count].im = Mybuff5[count].imag();
        fxp_time_signal2[count].re = Mybuff5[count].real();
        fxpcomplex << fxp_time_signal2[count].re << " " << fxp_time_signal2[count].im << endl;
    }
    fxpcomplex.close();

    delete[] Mybuff5;

    unsigned i;
    unsigned char out_bits[MaxPacketSize*2];
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


            if (sim_params.useLDPC)
            {
                totalMps += bits_per_OFDM_symbol * 0.75 *
                    (double)SampFreq / (CyclicPrefixLength + NumOfSubcarriers) / 10e5;
            }
            else
            {
                totalMps += bits_per_OFDM_symbol * sim_params.code_rate *
                    (double)SampFreq / (CyclicPrefixLength + NumOfSubcarriers) / 10e5;
            }

            if (!sim_params.useABL)
            {
                bits_per_OFDM_symbol = sim_params.bits_per_QAM_symbol * NumOfDataSubcarriers;
            }


    //[ Forming out bits
    printf("\nnum_of_samples = %d\n", num_of_samples);
    receiver((fxp_complex*)fxp_time_signal2,
             num_of_samples,
             //MaxPacketSize,
             out_bits,
             (unsigned int*)ABL_bits_per_symbol,
             &bits_per_OFDM_symbol,                // MaxBitsPerQAMSymbol
             &syncError,
             &feedBackNumOfOFDMSyms);

    int byte = 8;
    int chet = 0;
    int chet1 = 0;
    int per = 0;
    int per1;
    i = 0;
    char symbol1 = 50;

    while ((i < DATAURA))
    {
        while(chet<byte)
        {
            per1 = (int)pow(2, chet1);
            per1 = per1*out_bits[chet];
            per = per + per1;
            chet++;
            chet1++;
        }
        chet1 = 0;
        bufout[i] = (char)per;
        symbol1 = (char)per;
        if(symbol1 == '\0')
            break;
        per = 0;
        i++;
        byte = byte+8;
    }
    //]
}
#pragma clang diagnostic pop