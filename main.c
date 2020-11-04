/*
#include "ipHeader.h"
#include "tcpHeader.h"
#include "udpHeader.h"
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

//pseudo entete, pour l'instant on laisse ça ici
struct pseudo_entete {
    uint32_t ad_source;
    uint32_t ad_dest;
    uint16_t placeholder;
    uint8_t proto;
    uint16_t longueur_udp;
};

//checksum
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

    //besoin nécessairement de deux arguments(numport et adresse)
	if(argc < 2){
	perror("Pas assez d'arguments");
	exit(1);
	}

    //creation raw socket
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP); //socket(DOMAIN, TYPE, PROTO), les 2 premiers champs ne changeront pas, seulement le 3eme qui sera soit IPPROTO_TCP soit IPPROTO_UDP
    if (s < 0) {
        perror("Erreur socket");
        exit(1);
    }
    printf("socket OK");

    struct sockaddr_in sin;

    //représentation du paquet sous forme de datagramme
    char datagram[4096], source_ip[32], *data, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, 4096 /*ou sizeof(datagram)*/);

    //entete IP
    struct iphdr *iph = (struct iphd *) datagram;

    //entete UDP        stackoverflow, voir si ça marche
    struct udphdr *udph = (struct udphd *) (datagram + sizeof(struct ip));

    //remplissage données
    strcpy(data, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    //ip spoofée
    strcpy(source_ip, "192.168.1.1");

    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr ("127.0.0.1"); //adresse destination, on l'envoie sur localhost pour tester

    //remplissage entete ip
    iph -> ihl = 5;
    iph -> version = 4;
    iph -> tos = 0;
    iph -> tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    iph -> id = htonl(12345); //dans notre cas l'id peut être n'importe quoi
    iph -> frag_off = 0; //apparemment il faut le mettre à 0 avant de calculer la checksum
    iph -> ttl = 255;
    iph -> protocol = 17 /*= IPPROTO_UDP*/; //pour l'instant pour débugger on va envoyer des paquets UDP, c'est plus simple
    iph -> check = 0;
    iph -> saddr = inet_addr(source_ip); //inet_addr convertit la notation ipv4 en binaire
    iph -> daddr = sin.sin_addr.s_addr;

    iph -> check = checksum(datagram, iph -> tot_len);

    return 0;
}
