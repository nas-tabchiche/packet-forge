#include <time.h>

#include "randGen.h"

int randGen(int mini, int maxi) {
    int source_port = rand() % (maxi + 1 - mini) + mini;

    return source_port;
}
