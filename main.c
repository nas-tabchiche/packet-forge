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
    uint16_t longueur;
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

int main (int argc, char **argv) {

    /*if(argc < 3){
    perror("Pas assez d'arguments");
    exit(1);
    }*/

    //récupération des arguments
    int i = 0;
    for (i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    char protocole [3], ipsource[32], ipdest[32];
    unsigned short source_port, dest_port;

    //strcpy(protocole, argv[1]);
    strcpy(ipsource, argv[2]);
    strcpy(ipdest, argv[3]);
    printf("protocole : %s\nipsource : %s\nipdest : %s\n", protocole, ipsource, ipdest);



    //création du socket raw
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); //quand sur IPPROTO_UDP les paquets renvoient une erreur BAD UDP LENGTH < 8, qui disparait quand on passe sur IPPROTO_RAW

    if(s < 0) {
        perror("erreur socket(), essayez sudo.");
        exit(1);
    }
    printf("socket OK.\n"); //pour débuggage, à supprimer

    //représentation du paquet sous forme de datagramme
    char datagram[1024], source_ip[32], *data, *pseudogram;

    //initialiser le tampon à 0
    memset(datagram, 0, sizeof(datagram));

    //entete IP
    struct iphdr *iph = (struct iphdr *)datagram;


    if(strcmp(argv[1], "udp") == 0) {
        printf("udp"); //debug

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
        sin.sin_addr.s_addr = inet_addr("192.168.0.1"); //adresse destination

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
        udph -> source = htons(6666);
        udph -> dest = htons(6666);
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

    else if(strcmp(argv[1], "tcp") == 0) {
        printf("tcp"); //debug

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
        sin.sin_addr.s_addr = inet_addr("192.168.1.2"); //adresse destination

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
        tcph -> source = htons(1234);
        tcph -> dest = htons(80);
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
