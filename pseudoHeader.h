#ifndef PSEUDO_ENTETE_H
#define PSEUDO_ENTETE_H

struct pseudo_entete { //https://www.frameip.com/entete-udp/#34-8211-checksum
    uint32_t source;
    uint32_t dest;
    uint8_t mbz;
    uint8_t type;
    uint16_t longueur;
};

#endif
