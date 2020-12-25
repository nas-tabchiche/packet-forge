#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <time.h>

#include "traitementTCP.h"
#include "traitementUDP.h"
#include "randGen.h"
#include "checksum.h"
#include "pseudoHeader.h"
#include "stringIP.h"

int main (int argc, char **argv) {

    if(argc < 4){
    perror("sudo ./main [protocole] [IP source] [IP destination] [port destination]");
    exit(1);
    }

    //récupération des arguments
    char protocole[4], ipsource[32], ipdest[32];
    unsigned short source_port, dest_port;

    strcpy(protocole, argv[1]);
    strcpy(ipsource, argv[2]);
    strcpy(ipdest, argv[3]);
    dest_port = atoi(argv[4]);

    srand(time(NULL));

    //création du socket raw

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    if(s < 0) {
        perror("erreur socket(), essayez sudo.");
        exit(1);
    }

    printf("socket OK.\n\n"); //pour débuggage, à supprimer

    //représentation du paquet sous forme de datagramme
    char datagram[1024], source_ip[32], *data;//, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, sizeof(datagram));

    //entete IP
    //struct iphdr *iph = (struct iphdr *)datagram;

    //while(1) {

    //notre ip spoofée
    uint32_t ips = rand(); //on génère un bête entier 32 bits
    strcpy(source_ip, stringIP(ips));

    source_port = randGen(1024, 65535); //générer port destination aléatoire entre 1024 et 65535

    printf("protocole \t: %s\nipsource \t: %s\nport source \t: %d\nipdest \t\t: %s\nport dest \t: %d\n\n", protocole, source_ip, source_port, ipdest, dest_port); //debuggage


    if(strcmp(protocole, "udp") == 0) {
        traitementUDP(s, datagram, data, source_ip, ipdest, source_port, dest_port);
    }

    else if(strcmp(protocole, "tcp") == 0) {
        traitementTCP(s, datagram, data, source_ip, ipdest, source_port, dest_port);
    }

    //}

    return 0;
}
