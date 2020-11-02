#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
//#include <sys/types.h>
#include <stdlib.h> //exit(0);
#include <errno.h> //errno - numero erreur
//#include <netinet/ip.h>	//Provides declarations for ip header
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // sleep()

struct udpHeader {
    uint16_t portS : 4; //port relatif à l’application en cours sur la machine source
    uint16_t portD : 4; // port relatif à l’application en cours sur la machine de destination
    uint16_t len; //longueur totale
    uint16_t checksum; //checksum, somme de controle
    };
