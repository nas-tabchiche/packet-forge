#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <time.h>
#include <math.h>

#include "checksum.h"
#include "pseudoHeader.h"
#include "randGen.h"

void traitementTCP(int sock, char dtg[1024], char *dt, char sip[32], char dip[32], unsigned short sp, unsigned short dp) {

    printf("tcp\n\n"); //debug

    //entete TCP
    struct tcphdr *tcph = (struct tcphdr *) (dtg + sizeof(struct ip));

    struct sockaddr_in sin;
    struct pseudo_entete ph;

    //Data part
    dt = dtg + sizeof(struct iphdr) + sizeof(struct tcphdr);
    strcpy(dt, "69 LA TRIK"); //remplissage champ données

    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr(dip); //adresse destination

    struct iphdr *iph = (struct iphdr *)dtg;

    //remplissage de l'entete IP
    iph -> ihl = 5;
    iph -> version = 4; //ipv4
    iph -> tos = 0;
    iph -> tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(dt);
    iph -> id = htonl (1);
    iph -> frag_off = 0;
    iph -> ttl = 255;
    iph -> protocol = IPPROTO_TCP; //pour l'instant pour debugger on va envoyer des paquets UDP, c'est plus simple
    iph -> check = 0;
    iph -> saddr = inet_addr(sip); //inet_addr convertit la notation ipv4 en binaire
    iph -> daddr = sin.sin_addr.s_addr;

    //calcul de la checksum
    iph -> check = checksum ((unsigned short *)dtg, iph -> tot_len);

    //remplissage entete TCP
    tcph -> source = htons(sp);
    tcph -> dest = htons(dp);
    tcph -> check = 0;
    tcph -> seq = randGen(0, (int)(pow(2, 32)) -1);
    tcph -> ack_seq = 0;
    tcph -> doff = 5; // taille en-tête tcp
    tcph -> fin = 0;
    tcph -> syn = 1;
    tcph -> rst = 0;
    tcph -> psh = 0;
    tcph -> ack = 0;
    tcph -> urg = 0;
    tcph -> window = htons(40); // taille maximum de la fenêtre
    tcph -> urg_ptr = 0;

    printf("%d\n", tcph -> seq);

    //calcul de la checksum udp à l'aide du pseudo-entete
    ph.source = inet_addr(sip);
    ph.dest = sin.sin_addr.s_addr;
    ph.mbz = 0; //MBZ toujours à 0
    ph.type = IPPROTO_TCP;
    ph.longueur = htons(sizeof(struct tcphdr) + strlen(dt));

    int taille_pseudogramme = sizeof(struct pseudo_entete) + sizeof(struct tcphdr) + strlen(dt);
    char *pseudogram = malloc(taille_pseudogramme);

    memcpy(pseudogram , (char*)&ph, sizeof(struct pseudo_entete));
    memcpy(pseudogram + sizeof(struct pseudo_entete), tcph, sizeof(struct tcphdr) + strlen(dt));

    tcph -> check = checksum((unsigned short*)pseudogram , taille_pseudogramme);

    //Envoi du paquet
    if (sendto(sock, dtg, iph -> tot_len,  0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Échec de l'envoi du paquet.");
    }
    else {
        printf("Paquet envoyé. Longueur : %d \n", iph -> tot_len);
    }
}
