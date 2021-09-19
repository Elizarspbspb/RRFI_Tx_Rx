/* File History
* *****************  Version 1  *****************
* User: Eegorov      Date: 1.09.21   Time: 12:00
*
*/

#define BOARD 1         // 1 - Activate; 0 - Inactive
#define BUF_SIZE 8200
#define Num_of_samples 8160
#define MaxSizeUDP 65280
#define DATAURA2 2617
int DATAURA = 2617;
#include <boost/test/unit_test.hpp>
#include "out/WLANT/MyReceiver.cpp"
#include "out/WLANT/MyTransmitter.cpp"
#include <fstream>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
//#include <mpi/mpi.h>
#include <pthread.h>
#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define SUCCESS        0

int count; /* общие данные для потоков */

 std::vector<std::complex<int16_t>> buff_from(Num_of_samples*8);
 /*std::vector<std::complex<float>> buff_in((1 << MaxSamplingPower) *
                                      (PreambleLength + MaxNumOfSymbols *
                                                        (CyclicPrefixLength + NumOfSubcarriers)));*/
 std::vector<std::complex<int16_t>> buff_in(Num_of_samples);

 unsigned int num_of_samples = 0;
 char bufout[BUF_SIZE];
 char *file = "Configuration.cfg";
 char bufin[DATAURA2];

#if BOARD == 1
#include "ETSI_RRS_USRP_Device.h"
#include "ETSI_RRS_RRFI.h"
/*struct TestFixture {
    ETSI_RRS_USRP_Device usrpDevice;
    ETSI_RRS_RRFI rrfi;
    TestFixture() : rrfi(usrpDevice) {}
    ~TestFixture() = default;
};
TestFixture rrfiboard; // для работы с платой
        const double freq = 2480000000;
        const double bw = 35000000;
        const double sr = 2000000;
        const double gain = 50;
*/
class TestFixture {
private:
    const double freq = 2480000000;
    const double bw = 35000000;
    const double sr = 2000000;
    const double gain = 50;
public:
    ETSI_RRS_USRP_Device usrpDevice;
    ETSI_RRS_RRFI rrfi;
    TestFixture() : rrfi(usrpDevice) {}
    ~TestFixture() = default;

    int get_freq(){
        return freq;
    }
    int get_bw(){
        return bw;
    }
    int get_sr(){
        return sr;
    }
    int get_gain(){
        return gain;
    }
};
TestFixture rrfiboard; // для работы с платой

/*const double freq = 2400000000;
const double bw = 20000000;
const double sr = 2000000;
const double gain = 60;*/

#endif

void* Transmitter_thread(void *args)
{
    TestFixture rrfiboard;
#if BOARD == 1
    msec = times(time_str, time_str2, time_str3);
    log_file << left << setfill(' ') << setw(89) << "Transmitting . . . "<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
    cout << "\e[35m" << "Transmitting . . . " << "\e[0m" << endl;

    rrfiboard.rrfi.set_maxTxPowerLevel(90);
    rrfiboard.rrfi.set_txAntennaPort(0);
    rrfiboard.rrfi.get_txAntennaPort();
    //rrfiboard.rrfi.set_txCenterFrequency(freq);         //tuning to a desired center frequency
    rrfiboard.rrfi.set_txCenterFrequency(rrfiboard.get_freq());         //tuning to a desired center frequency

    rrfiboard.rrfi.pause2();     // My

    rrfiboard.rrfi.get_txCenterFrequency();
    //rrfiboard.rrfi.set_txBandwidth(bw);
    rrfiboard.rrfi.set_txBandwidth(rrfiboard.get_bw());
    rrfiboard.rrfi.get_txBandwidth();
    //rrfiboard.rrfi.set_txSamplingRate(sr);
    rrfiboard.rrfi.set_txSamplingRate(rrfiboard.get_sr());
    rrfiboard.rrfi.get_txSamplingRate();
    //rrfiboard.rrfi.set_txPowerLevel(gain);
    rrfiboard.rrfi.set_txPowerLevel(rrfiboard.get_gain());
    rrfiboard.rrfi.get_txPowerLevel();

    buff_in = MyTransmitter(file, &num_of_samples, bufin);

    std::ofstream buff_in_signal("Buff_in.txt", std::ios_base::out);
    for (int i = 0; i < num_of_samples; i++)
        buff_in_signal << buff_in[i].imag() << " " << buff_in[i].real() << endl;
    buff_in_signal.close();

    //rrfiboard.rrfi.tx_from_buff(buff_in, num_of_samples); // вот так
    for (int i = 0; i < num_of_samples*100; i++)
    {
        rrfiboard.rrfi.tx_from_buff(buff_in, num_of_samples);
    }

    msec = times(time_str, time_str2, time_str3);
    log_file << left << setfill(' ') << setw(89) << "End Transmitting"<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
    cout << "\e[31m" << "End Transmitting" << "\e[0m" << endl;
    return SUCCESS;
#endif
}

void* Receiver_thread(void *args)
{
#if BOARD == 1
    msec = times(time_str, time_str2, time_str3);
    log_file << left << setfill(' ') << setw(89) << "Receiving . . . "<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
    cout << "\e[35m" << "Receiving . . . " << "\e[0m" << endl;

    rrfiboard.rrfi.set_maxTxPowerLevel(90);
    rrfiboard.rrfi.set_rxAntennaPort(1);
    rrfiboard.rrfi.get_rxAntennaPort();
    //rrfiboard.rrfi.set_rxCenterFrequency(freq);     //tuning to a desired center frequency
    rrfiboard.rrfi.set_txCenterFrequency(rrfiboard.get_freq());         //tuning to a desired center frequency
    rrfiboard.rrfi.pause2();    // My

    rrfiboard.rrfi.get_rxCenterFrequency();
    //rrfiboard.rrfi.set_rxBandwidth(bw);
    rrfiboard.rrfi.set_txBandwidth(rrfiboard.get_bw());
    rrfiboard.rrfi.get_rxBandwidth();
    //rrfiboard.rrfi.set_rxSamplingRate(sr);
    rrfiboard.rrfi.set_txSamplingRate(rrfiboard.get_sr());
    rrfiboard.rrfi.get_rxSamplingRate();
    //rrfiboard.rrfi.set_rxGain(gain);
    rrfiboard.rrfi.set_txPowerLevel(rrfiboard.get_gain());
    rrfiboard.rrfi.get_rxGain();

    cout << "Another parameters: " << endl;
    //rrfiboard.rrfi.get_rxAntennaPorts();  // ERROR
    //int per = 0;
    //cout << "Enter 2 for continue...";
    //cin >> per;

    for(int count = 0; count < Num_of_samples; count++) {
        buff_from[count] = 0;
    }
        //buff_from = rrfiboard.rrfi.rx_to_buff(MaxPacketSize, 20, Num_of_samples);
    //buff_from = rrfiboard.rrfi.rx_to_buff(Num_of_samples*1000, 6, Num_of_samples*1000);
    buff_from = rrfiboard.rrfi.rx_to_buff(Num_of_samples*1000, 6, Num_of_samples*1000);
        //buff_from_sh = rrfiboard.rrfi.rx_to_buff(MaxPacketSize, 30, Num_of_samples*2);  // -2, -3, 0, -2, -1, -1, -3, 1, -2
        //buff_from_sh = rrfiboard.rrfi.rx_to_buff(Num_of_samples*2*2, 10, Num_of_samples*2); // 0, -2, 5, 0, 4, 3, 2, 0, 1, 4, 0, 5, -1
        //buff_from_sh = rrfiboard.rrfi.rx_to_buff(Num_of_samples, 10, 100); //всего 100 комплексных чисел
        //buff_from_sh = rrfiboard.rrfi.rx_to_buff(Num_of_samples*2, 10, Num_of_samples); //всего 100 комплексных чисел

    //rrfiboard.rrfi.rx_to_file("RRFI_to_file.txt", Num_of_samples, 10, Num_of_samples);

    std::ofstream buff_from_signal("Buff_from.txt", std::ios_base::out);
    std::ofstream buff_from_signal_iq("Buff_from_iq.txt", std::ios_base::out);
    for (int i = 0; i < Num_of_samples*1000; i++)
    {
        buff_from_signal << buff_from[i] << endl;
        buff_from_signal_iq << buff_from[i].imag() << " | " << buff_from[i].real() << endl;
        //cout << buff_from[i] << "; ";
    }
    buff_from_signal.close();
    buff_from_signal_iq.close();

    msec = times(time_str, time_str2, time_str3);
    log_file << left << setfill(' ') << setw(89) << "End Receiving"<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
    cout << "\e[31m" << "End Receiving" << "\e[0m" << endl;

    for(int i = 0; i < BUF_SIZE; i++) {
        bufout[i] = '0';
    }

    MyReceiver(buff_from, Num_of_samples, file, bufout);

    for(int i = 0; i < 100; i++) {
        std::cout << bufout[i] << " ";
    }
    return SUCCESS;
#endif
}

int main(int argc, char* argv[])
{
    msec = times(time_str, time_str2, time_str3);
    log_file << left << setfill(' ') << setw(89) << " Start RRFI_Tx_Rx " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
    log_file << "Board IN_ACTIVATE" << endl;
    cout << " RRFI_Tx_Rx\n";

    pthread_t thread1;
    pthread_t thread2;

    if(argc != 1)
    {cout << "Incorrect RRFI_Tx_Rx using!\n";
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " End RRFI_Tx_Rx " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
        cout << "End RRFI_Tx_Rx\n";
        return -1;}

    char *param = "Configuration.cfg";
    int packet_size=0, data_rate=0;
    ReadInt("General", "packet_size", param, "decimal", &packet_size);
    ReadInt("General", "data_rate", param, "decimal", &data_rate);
    cout << "packet_size = " << packet_size << endl;
    cout << "data_rate = " << data_rate << endl;

    if ( data_rate == 6 )
    {
        DATAURA = packet_size;
        if(packet_size > 288)
            DATAURA = 288;
    }
    else if ( data_rate == 9 )
    {
        DATAURA = packet_size;
        if(packet_size > 433)
            DATAURA = 433;
    }
    else if ( data_rate == 12 )
    {
        DATAURA = packet_size;
        if(packet_size > 579)
            DATAURA = 579;
    }
    else if ( data_rate == 18 )
    {
        DATAURA = packet_size;
        if(packet_size > 870)
            DATAURA = 870;
    }
    else if ( data_rate == 24 )
    {
        DATAURA = packet_size;
        if(packet_size > 1161)
            DATAURA = 1161;
    }
    else if ( data_rate == 36 )
    {
        DATAURA = packet_size;
        if(packet_size > 1743)
            DATAURA = 1743;
    }
    else if ( data_rate == 48 )
    {
        DATAURA = packet_size;
        if(packet_size > 2325)
            DATAURA = 2325;
    }
    else if ( data_rate == 54 )
    {
        DATAURA = packet_size;
        if(packet_size > 2616)
            DATAURA = 2616;
    }
    else
    {
        std::cout << "Invalid data_rate in Configuration.cfg" << std::endl;
        exit(1);
    }

    //int sock_app;
    int sock_tx;
    //int sock_test_object2;
    int sock_rx;
    struct sockaddr_in addrapp;
    struct sockaddr_in addr_transmitter, cliAddr_transmitter;
    socklen_t cliAddrLentransmitter;

    sock_tx = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Отправка сигнала в URA
    sock_rx = socket(AF_INET, SOCK_DGRAM, 0); // Получение сигнала из URA

    if(sock_tx < 0)
        {perror("sock_tx");return -2;}
    if(sock_rx < 0)
        {perror("sock_rx");return -3;}

    addrapp.sin_family = AF_INET;
    addrapp.sin_addr.s_addr = htonl(INADDR_ANY);
    //unsigned int PORT_PRIL = 5566;
    unsigned int PORT_TRANSMITTER = 5566;
    addrapp.sin_port = htons(PORT_TRANSMITTER);

    addr_transmitter.sin_family = AF_INET;
    addr_transmitter.sin_addr.s_addr = htonl(INADDR_ANY);
    //unsigned int PORT_TRANSMITTER;
    unsigned int PORT_RECEIVER;
    for(PORT_RECEIVER = 4455; PORT_RECEIVER <= 4555; PORT_RECEIVER++) //Free ports checking
    {
        addr_transmitter.sin_port = htons(PORT_RECEIVER);
        if(bind(sock_rx, (struct sockaddr*)&addr_transmitter, sizeof(addr_transmitter)) == 0)
            break;
    }

    std::ofstream fout("Pids_Storage.txt", std::ios_base::out);
    fout << getpid(); //Writing the PID...
    fout << "Port for sock_tx: ";
    std::cout << "Number of port for 2 parameter URA: " << PORT_TRANSMITTER << std::endl;
    fout << PORT_TRANSMITTER << "\n"; // Му and its port number to file!
    fout << "Port for sock_rx: ";
    std::cout << "Number of port for 1 parameter URA: " << PORT_RECEIVER << std::endl;
    fout << PORT_RECEIVER; // Му and its port number to file!
    fout.close();

    char file_name[6];
    sprintf(file_name, "%d", getpid());

    char path_to_file[100];
    strcpy(path_to_file, "../");
    strcat(path_to_file, file_name);
    strcat(path_to_file, ".txt");
    long int readStatus;
    cliAddrLentransmitter = sizeof(cliAddr_transmitter);

    //char bufout[BUF_SIZE];
    //memset(bufout, 0, BUF_SIZE);
    std::complex<int16_t> Mybuff_from[Num_of_samples];
    //std::vector<std::complex<int16_t>> buff_from(Num_of_samples);
    //char *file = "Configuration.cfg";
    cout << " Open Configuration file...\n";
    //char bufin[DATAURA];
    //unsigned int num_of_samples = 0;
    int status;
    int status_addr;
    while(true)
    {
        cout << "\e[41m" << "Waiting data from URA . . .\n" << "\e[0m" << endl;

        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Start time of waiting for receiving data from URA " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
        cout << "Start time of waiting for receiving data from URA" << endl;

        #if BOARD == 1
        readStatus = recvfrom(sock_rx , bufin , DATAURA, 0 , (struct sockaddr*)&cliAddr_transmitter, &cliAddrLentransmitter); // Му While there is some data in the socket...
        if(readStatus > 0 )
        {
            msec = times(time_str, time_str2, time_str3);
            log_file << left << setfill(' ') << setw(89) << " End time of waiting for receiving data from URA " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
            cout << "End time of waiting for receiving data from URA" << endl;

            /*std::vector<std::complex<int16_t>> buff((1 << MaxSamplingPower) *
                                                  (PreambleLength + MaxNumOfSymbols *
                                                                    (CyclicPrefixLength + NumOfSubcarriers)));*/
            cout << "Open Configuration file..." << endl;

            status = pthread_create(&thread1, NULL, Receiver_thread, NULL);
            if (status != 0) {
                printf("main error: can't create Receiver_thread, status = %d\n", status);
                exit(ERROR_CREATE_THREAD);
            }
            sleep(3);
            status = pthread_create(&thread2, NULL, Transmitter_thread, NULL);
            if (status != 0) {
                printf("main error: can't create Transmitter_thread, status = %d\n", status);
                exit(ERROR_CREATE_THREAD);
            }

            status = pthread_join(thread1, (void**)&status_addr);
            if (status != SUCCESS) {
                printf("main error: can't join Receiver_thread, status = %d\n", status);
                exit(ERROR_JOIN_THREAD);
            }
            printf("joined with address %d\n", status_addr);

            status = pthread_join(thread2, (void**)&status_addr);
            if (status != SUCCESS) {
                printf("main error: can't join Transmitter_thread, status = %d\n", status);
                exit(ERROR_JOIN_THREAD);
            }
            printf("joined with address %d\n", status_addr);

            //rrfiboard.rrfi.tx_from_buff(buff, num_of_samples); // вот так
            //rrfi.tx_from_buff(buff, num_of_samples); // старая версия Леонид
        }

        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Start send data to URA "<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
        cout << "Start send data to URA" << endl;

        if (sendto(sock_tx, bufout, DATAURA, 0, (struct sockaddr*)&addrapp, sizeof(addrapp)) < 0)
        {
            perror("sending error1...\n");
            exit(-1);
        }
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Finish send data to URA "<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
        cout << "Finish send data to URA" << endl;

        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " End RRFI_Tx_Rx " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
        cout << "End RRFI_Tx_Rx" << endl;
        #else
        readStatus = recvfrom(sock_rx , bufin , DATAURA, 0 , (struct sockaddr*)&cliAddr_transmitter, &cliAddrLentransmitter); // Му While there is some data in the socket...
        if(readStatus > 0 )
        {
            std::vector<std::complex<int16_t>> buff((1 << MaxSamplingPower) *
                                                  (PreambleLength + MaxNumOfSymbols *
                                                                    (CyclicPrefixLength + NumOfSubcarriers)));
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wwritable-strings"
            char *fp = "Configuration.cfg";
            #pragma clang diagnostic pop

            cout << "Open Configuration file..." << endl;

            buff = MyTransmitter(fp, &num_of_samples, bufin);

            cout << "\n Num_of_samples = " << num_of_samples << endl;
            cout << "Buffer fulled" << endl;

            for(unsigned int count = 0; count < num_of_samples; count++)
            {
                buff_from[count] = buff[count];
            }
        }

        MyReceiver(buff_from, Num_of_samples, file, bufout);      //Inactive board + Test2

        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Start send data to URA "<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

        if (sendto(sock_tx, bufout, DATAURA, 0, (struct sockaddr*)&addrapp, sizeof(addrapp)) < 0)
        {
            perror("sending error1...\n");
            exit(-1);
        }
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " Finish send data to URA "<<" | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;

        cout << " \nData Sand.\n";
        msec = times(time_str, time_str2, time_str3);
        log_file << left << setfill(' ') << setw(89) << " End RRFI_Tx_Rx " << " | " << time_str<<"-"<< time_str2<<":"<< time_str3 << ":" << right << setfill('0') << setw(6) <<  msec << endl;
        #endif
    }
}