#include "checksum.h"
#include "pseudoHeader.h"

#ifndef TRAITEMENTUDP_H
#define TRAITEMENTUDP_H

void traitementUDP(int sock, char dtg[1024], char *dt, char sip[32], char dip[32], unsigned short sp, unsigned short dp);

#endif
