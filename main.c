/*
    #include "ipHeader.h"
    #include "tcpHeader.h"
    #include "udpHeader.h"
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>

struct pseudo_entete { //https://www.frameip.com/entete-udp/#34-8211-checksum
    uint32_t source;
    uint32_t dest;
    uint8_t mbz;
    uint8_t type;
    uint16_t longueur_udp;
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

int main (/*argc, argv[]*/) {

    /*if(argc < 4){
	perror("Pas assez d'arguments");
	exit(1);
	}*/

    //création du socket raw
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); //quand sur IPPROTO_UDP les paquets renvoient une erreur BAD UDP LENGTH < 8, qui disparait quand on passe sur IPPROTO_RAW

    if(s < 0) {
        perror("erreur socket(), essayez sudo.");
        exit(1);
    }
    printf("socket OK.\n");

    //représentation du paquet sous forme de datagramme
    char datagram[4096], source_ip[32], *data, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, sizeof(datagram));

    //entete IP
    struct iphdr *iph = (struct iphdr *)datagram;

    //entete UDP
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct ip));

    struct sockaddr_in sin;
    struct pseudo_entete ph;

    //Data part
    data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    strcpy(data, "TEST TEST 123456789 TEST TEST"); //remplissage champ données

    //notre IP spoofée
    strcpy(source_ip, "192.168.1.10");

    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr("192.168.1.2"); //adresse destination

    //remplissage de l'entete IP
    iph -> ihl = 5;
    iph -> version = 4; //ipv4
    iph -> tos = 0;
    iph -> tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
    iph -> id = htonl (54321);
    iph -> frag_off = 0;
    iph -> ttl = 255;
    iph -> protocol = IPPROTO_UDP; //pour l'instant pour debugger on va envoyer des paquets UDP, c'est plus simple
    iph -> check = 0;
    iph -> saddr = inet_addr(source_ip); //inet_addr convertit la notation ipv4 en binaire
    iph -> daddr = sin.sin_addr.s_addr;

    //calcul de la checksum
    iph -> check = checksum ((unsigned short *)datagram, iph -> tot_len);

    //remplissage entete UDP
    udph -> source = htons(6666);
    udph -> dest = htons(8622);
    udph -> len = htons(8 + strlen(data));
    udph -> check = 0;

    //calcul de la checksum udp à l'aide du pseudo-entete
    ph.source = inet_addr(source_ip);
    ph.dest = sin.sin_addr.s_addr;
    ph.mbz = 0; //MBZ toujours à 0
    ph.type = IPPROTO_UDP;
    ph.longueur_udp = htons(sizeof(struct udphdr) + strlen(data));

    int taille_pseudogramme = sizeof(struct pseudo_entete) + sizeof(struct udphdr) + strlen(data);
    pseudogram = malloc(taille_pseudogramme);

    memcpy(pseudogram , (char*)&ph, sizeof(struct pseudo_entete));
    memcpy(pseudogram + sizeof(struct pseudo_entete), udph, sizeof(struct udphdr) + strlen(data));

    udph -> check = checksum((unsigned short*)pseudogram , taille_pseudogramme);

    //Envoi du paquet
    if (sendto(s, datagram, iph -> tot_len,  0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Échec de l'envoi du paquet.");
    }
    else {
        printf("Paquet envoyé. Longueur : %d \n", iph -> tot_len);
    }

    return 0;
}
