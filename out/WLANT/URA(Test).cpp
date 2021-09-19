#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 256

int main()
{
    int sock;
    struct sockaddr_in addr;
    char buf[BUF_SIZE];
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
    {
        perror("socket");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    unsigned int port;

    for(port = 3333; port <= 3444; port++) //Free ports checking
    {
	    addr.sin_port = htons(port);

	    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	        break;
    }

    std::ofstream fout("/home/apt-15/Docs/Server/URA/Pids_Storage.txt", std::ios_base::out);
    fout << getpid(); //Writing the PID...
    fout << "\n";
    fout << port; //and its port number to file!
    fout.close();

    char file_name[6];
    sprintf(file_name, "%d", getpid());

    char command[100];
    strcpy(command, "touch /home/apt-15/Docs/Server/URA_output/");
    strcat(command, file_name);
    strcat(command, ".txt");

    system(command); //"PID.txt" creating, log-file
    
    char path_to_file[100];
    strcpy(path_to_file, "/home/apt-15/Docs/Server/URA_output/");
    strcat(path_to_file, file_name);
    strcat(path_to_file, ".txt");

    while(1)
    {
        if(recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL) > 0) //While there is some data in the socket...
        {
            fout.open(path_to_file, std::ios_base::app);
            fout << buf; //Writing to file
            fout << "\n";
            fout.close();
        }
    }
}