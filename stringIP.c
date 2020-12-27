#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

char *stringIP(uint32_t ip){
    unsigned char octet[4];
    char *s_ip = malloc(32);
    octet[0] = ip & 0xFF; //on récupère les 8 1ers bits en appliquant un & logique
    octet[1] = (ip >> 8) & 0xFF; //on décale vers la droite de 8 bits
    octet[2] = (ip >> 16) & 0xFF; //16 bits
    octet[3] = (ip >> 24) & 0xFF; // 24 bits

    sprintf(s_ip, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);

    return s_ip;
}
