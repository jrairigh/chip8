#include "codes.h"
unsigned short program[] = {
/*200*/ CLS
/*202*/ LD1(0, 64)
/*204*/ LD1(1, 32)
/*206*/ LDB(D7)
/*208*/ DRW(0, 1, 5)
/*20A*/ JP(0x20A)
};