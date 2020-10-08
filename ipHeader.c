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

struct ipHeader {

#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t vers : 4; //format IPv4
    uint8_t ihl : 4; //taille max du paquet : codé sur 4 bits donc 15*32/8 = 60 octets
#elseif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t ihl : 4;
    uint8_t vers : 4;
#else
#error "Erreur endian"
#endif // __BYTE_ORDER

    uint8_t tos; //type of service, valeur par défaut
    uint16_t len; //longueur totale
    uint16_t id; //identification
    uint16_t flags : 3; //etat de fragmentation
    uint16_t frag_offset : 13; //position fragment
    uint8_t ttl; //time to live, durée de vie du paquet
    uint8_t protocol; //protocole - TCP = 00110, UDP = 10001
    uint16_t checksum; //checksum, somme de controle
    uint32_t ipsource; //adresse source
    uint32_t ipdest; //adresse destination
} __attribute__((packed));

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

int main(int argc, char* argv[]) {
    return 0;
}

//ipHeader iph(char *protocole, char *source, char *destination);

