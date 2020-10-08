#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
//#include <sys/types.h>
#include <stdlib.h> //exit(0);
#include <errno.h> //errno - numero erreur
//#include <netinet/ip.h>	//Provides declarations for ip header
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // sleep()

struct tcpHeader {
    uint16_t psource; // port source
    uint16_t pdest; // port de destination
    uint32_t seq; //séquence
    uint32_t ack; // numéro ACK
    uint16_t fenetre; // fenetre
    uint16_t def; // offset-4bits réservé-6bits urg-1bit ack-1bit psh-1bit rst-1bit syn-1bit fin-1bit
    uint16_t checksum; // somme de contrôle
    uint16_t ptr; // pointeur

};
uint16_t checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     * Taken from https://tools.ietf.org/html/rfc1071
     */

    register uint32_t sum = 0;
    uint16_t * ptr = addr;

    while (count > 1)  {
        /*  This is the inner loop */
        sum += * ptr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if (count > 0)
        sum += * (uint8_t *) ptr;

    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

int main(){
return 0;
}
