#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <termios.h>
#include <cstring>
#include <fstream>

#define BUF_SIZE 256

using namespace std;
 
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Incorrect Transmitter using!\n");
        return -1;
    }

    int port;
    sscanf(argv[1], "%d", &port); //Port number translating from char* to int

    int sock;
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
    {
        perror("socket");
        return -2;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); //Destination port for transfer
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    std::ofstream fout("/home/apt-15/Docs/Server/Transmitter/Pids_Storage.txt", std::ios_base::out);
    fout << getpid(); //Writing the PID...
    fout << "\n";
    fout.close();

    char buf[BUF_SIZE];

    ifstream fin;

    while(1) //Endless transfer
    {
        fin.open("DATA.txt", std::ios_base::in);

        while(!fin.eof())
        {
	        fin.getline(buf, BUF_SIZE, '\n');
	        sendto(sock, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));
	        sleep(1);
        }

        fin.close();
    }
}