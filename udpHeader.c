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
