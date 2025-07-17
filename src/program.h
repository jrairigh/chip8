#include "codes.h"
unsigned short program[] = {
/*200*/ CLS             
/*202*/ LDB(D7)         
/*204*/ LD6(0)          // x = DT
/*206*/ SNE1(0, 0)      // x != 0 i.e. DT up
/*208*/ CALL(0x210)     // skip over reset DT and xy increment
/*20a*/ CLS             
/*20c*/ DRW(1, 2, 15)   
/*20e*/ JP(0x204)       
// Begin func
/*210*/ LD1(0, 20)      // Begin function call
/*212*/ LD5(0)          // DT = 20 cycles
/*214*/ ADD1(1, 1)      // increment X position
/*216*/ ADD1(2, 1)      // increment Y position
/*218*/ RET             
// End func
};