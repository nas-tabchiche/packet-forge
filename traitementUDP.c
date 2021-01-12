#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <time.h>

#include "checksum.h"
#include "pseudoHeader.h"

void traitementUDP(int sock, char dtg[1024], char *dt, char sip[32], char dip[32], unsigned short sp, unsigned short dp) {

    //entete UDP
    struct udphdr *udph = (struct udphdr *) (dtg + sizeof(struct ip));

    struct sockaddr_in sin;
    struct pseudo_entete ph;

    struct iphdr *iph = (struct iphdr *)dtg;

    //Data part
    char *dta = dtg + sizeof(struct iphdr) + sizeof(struct udphdr);
    strcpy(dta, dt); //remplissage champ données

    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr(dip); //adresse destination

    //remplissage de l'entete IP
    iph -> ihl = 5;
    iph -> version = 4; //ipv4
    iph -> tos = 0;
    iph -> tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(dta);
    iph -> id = htonl (1);
    iph -> frag_off = 0;
    iph -> ttl = 255;
    iph -> protocol = IPPROTO_UDP; //pour l'instant pour debugger on va envoyer des paquets UDP, c'est plus simple
    iph -> check = 0;
    iph -> saddr = inet_addr(sip); //inet_addr convertit la notation ipv4 en binaire
    iph -> daddr = sin.sin_addr.s_addr;

    //calcul de la checksum
    iph -> check = checksum ((unsigned short *)dtg, iph -> tot_len);

    //remplissage entete UDP
    udph -> source = htons(sp);
    udph -> dest = htons(dp);
    udph -> len = htons(8 + strlen(dta));
    udph -> check = 0;

    //calcul de la checksum udp à l'aide du pseudo-entete
    ph.source = inet_addr(sip);
    ph.dest = sin.sin_addr.s_addr;
    ph.mbz = 0; //MBZ toujours à 0
    ph.type = IPPROTO_UDP;
    ph.longueur = htons(sizeof(struct udphdr) + strlen(dta));

    int taille_pseudogramme = sizeof(struct pseudo_entete) + sizeof(struct udphdr) + strlen(dta);
    char *pseudogram = malloc(taille_pseudogramme);

    memcpy(pseudogram , (char*)&ph, sizeof(struct pseudo_entete));
    memcpy(pseudogram + sizeof(struct pseudo_entete), udph, sizeof(struct udphdr) + strlen(dta));

    udph -> check = checksum((unsigned short*)pseudogram , taille_pseudogramme);

    //Envoi du paquet
    if (sendto(sock, dtg, iph -> tot_len,  0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Échec de l'envoi du paquet.");
    }
    else {
        printf("Paquet envoyé. Longueur : %d octets\n", iph -> tot_len);
        //fflush(stdout);
    }

}
