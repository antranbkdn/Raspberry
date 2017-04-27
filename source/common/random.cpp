#include "random.h"
#include <ctime>
#include <cstdlib>

uint16_t random_16bit(uint16_t min, uint16_t max) {
    srand (time (NULL));
    return rand() % (max-min+1) + min;
}

