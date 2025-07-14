#include "codes.h"
unsigned short program[] = {
/*200*/ CLS
/*202*/ LD1(0, 10)
/*204*/ LD1(1, 1)
/*206*/ LDB(D7)
/*208*/ DRW(0, 1, 3)
/*20A*/ JP(0x20A)
};