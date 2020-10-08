#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
//#include <sys/types.h>
#include <stdlib.h> //exit(0);
#include <errno.h> //errno - numero erreur
//#include <netinet/ip.h>	//Provides declarations for ip header
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // sleep()
#include <endian.h> //definition little endian et big endian

int main(int argc, char* argv[]) {
    //creation raw socket
    ins s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
    return 0;
}
