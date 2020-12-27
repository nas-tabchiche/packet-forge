#include <time.h>

#include "randGen.h"

int randGen(int mini, int maxi) {
    int randNum = rand() % (maxi + 1 - mini) + mini;
    return randNum;
}
