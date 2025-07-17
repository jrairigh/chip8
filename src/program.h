#include "codes.h"
unsigned short program[] = {
/*200*/ CLS             
/*202*/ LDB(D7)         
/*204*/ LD6(0)          // x = DT
/*206*/ SNE1(0, 0)      // x != 0 i.e. DT up
/*208*/ CALL(0x20c)     // skip over reset DT and xy increment
/*20a*/ JP(0x204)       
// Begin func
/*20c*/ LD1(0, 60)      // Begin function call
/*20e*/ LD5(0)          // DT = 60 cycles
/*210*/ ADD1(1, 1)      // increment X position
/*212*/ ADD1(2, 1)      // increment Y position
/*214*/ DRW(1, 2, 15)   
/*216*/ RET             
// End func
};