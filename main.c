#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include "checksum.h"
#include "pseudoHeader.h"


int main (int argc, char **argv) {

    if(argc < 5){
    perror("Pas assez d'arguments");
    exit(1);
    }

    //récupération des arguments
    char protocole[4], ipsource[32], ipdest[32];
    unsigned short source_port, dest_port;

    strcpy(protocole, argv[1]);
    strcpy(ipsource, argv[2]);
    source_port = atoi(argv[3]);
    strcpy(ipdest, argv[4]);
    dest_port = atoi(argv[5]);
    printf("protocole \t: %s\nipsource \t: %s\nport source \t: %d\nipdest \t\t: %s\nport dest \t: %d\n\n", protocole, ipsource, source_port, ipdest, dest_port); //debuggage



    //création du socket raw
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); //quand sur IPPROTO_UDP les paquets renvoient une erreur BAD UDP LENGTH < 8, qui disparait quand on passe sur IPPROTO_RAW

    if(s < 0) {
        perror("erreur socket(), essayez sudo.");
        exit(1);
    }
    printf("socket OK.\n\n"); //pour débuggage, à supprimer

    //représentation du paquet sous forme de datagramme
    char datagram[1024], source_ip[32], *data, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, sizeof(datagram));

    //entete IP
    struct iphdr *iph = (struct iphdr *)datagram;


    if(strcmp(protocole, "udp") == 0) {
        printf("udp\n\n"); //debug

        //entete UDP
        struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct ip));

        struct sockaddr_in sin;
        struct pseudo_entete ph;

        //Data part
        data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
        strcpy(data, "TEST TEST 123456789 TEST TEST"); //remplissage champ données

        //notre IP spoofée
        strcpy(source_ip, ipsource);

        sin.sin_family = AF_INET;
        sin.sin_port = htons(80);
        sin.sin_addr.s_addr = inet_addr(ipdest); //adresse destination

        //remplissage de l'entete IP
        iph -> ihl = 5;
        iph -> version = 4; //ipv4
        iph -> tos = 0;
        iph -> tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
        iph -> id = htonl (1);
        iph -> frag_off = 0;
        iph -> ttl = 255;
        iph -> protocol = IPPROTO_UDP; //pour l'instant pour debugger on va envoyer des paquets UDP, c'est plus simple
        iph -> check = 0;
        iph -> saddr = inet_addr(source_ip); //inet_addr convertit la notation ipv4 en binaire
        iph -> daddr = sin.sin_addr.s_addr;

        //calcul de la checksum
        iph -> check = checksum ((unsigned short *)datagram, iph -> tot_len);

        //remplissage entete UDP
        udph -> source = htons(source_port);
        udph -> dest = htons(dest_port);
        udph -> len = htons(8 + strlen(data));
        udph -> check = 0;

        //calcul de la checksum udp à l'aide du pseudo-entete
        ph.source = inet_addr(source_ip);
        ph.dest = sin.sin_addr.s_addr;
        ph.mbz = 0; //MBZ toujours à 0
        ph.type = IPPROTO_UDP;
        ph.longueur = htons(sizeof(struct udphdr) + strlen(data));

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
    }

    else if(strcmp(protocole, "tcp") == 0) {
        printf("tcp\n\n"); //debug

        //entete TCP
        struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));

        struct sockaddr_in sin;
        struct pseudo_entete ph;

        //Data part
        data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
        strcpy(data, "TEST TEST 123456789 TEST TEST"); //remplissage champ données

        //notre IP spoofée
        strcpy(source_ip, ipsource);

        sin.sin_family = AF_INET;
        sin.sin_port = htons(80);
        sin.sin_addr.s_addr = inet_addr(ipdest); //adresse destination

        //remplissage de l'entete IP
        iph -> ihl = 5;
        iph -> version = 4; //ipv4
        iph -> tos = 0;
        iph -> tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(data);
        iph -> id = htonl (1);
        iph -> frag_off = 0;
        iph -> ttl = 255;
        iph -> protocol = IPPROTO_TCP; //pour l'instant pour debugger on va envoyer des paquets UDP, c'est plus simple
        iph -> check = 0;
        iph -> saddr = inet_addr(source_ip); //inet_addr convertit la notation ipv4 en binaire
        iph -> daddr = sin.sin_addr.s_addr;

        //calcul de la checksum
        iph -> check = checksum ((unsigned short *)datagram, iph -> tot_len);

        //remplissage entete TCP
        tcph -> source = htons(source_port);
        tcph -> dest = htons(dest_port);
        tcph -> check = 0;
        tcph -> seq = 0;
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


        //calcul de la checksum udp à l'aide du pseudo-entete
        ph.source = inet_addr(source_ip);
        ph.dest = sin.sin_addr.s_addr;
        ph.mbz = 0; //MBZ toujours à 0
        ph.type = IPPROTO_TCP;
        ph.longueur = htons(sizeof(struct tcphdr) + strlen(data));

        int taille_pseudogramme = sizeof(struct pseudo_entete) + sizeof(struct tcphdr) + strlen(data);
        pseudogram = malloc(taille_pseudogramme);

        memcpy(pseudogram , (char*)&ph, sizeof(struct pseudo_entete));
        memcpy(pseudogram + sizeof(struct pseudo_entete), tcph, sizeof(struct tcphdr) + strlen(data));

        tcph -> check = checksum((unsigned short*)pseudogram , taille_pseudogramme);

        //Envoi du paquet
        if (sendto(s, datagram, iph -> tot_len,  0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            perror("Échec de l'envoi du paquet.");
        }
        else {
            printf("Paquet envoyé. Longueur : %d \n", iph -> tot_len);
        }
    }
    return 0;
}
