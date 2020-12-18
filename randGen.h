#include <time.h>

#ifndef RANDGEN_H
#define RANDGEN_H

int randGen(int mini, int maxi) {
    srand(time(NULL));
    int source_port = rand() % (maxi + 1 - mini) + mini;

    return source_port;
}

#endif
