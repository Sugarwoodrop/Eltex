#include "buyer.h"

void init_buyer(buyer* buyer){
    srand(time(NULL));
    buyer->demand = 9900 + rand() % 200;
}
