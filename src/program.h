#include "codes.h"

/*
Test program draws one pixels at a time row by row every 3 cycles
*/
unsigned short program[] = {
/*200*/ LDB(0x204)         // load index with sprite data
/*202*/ JP(0x214)

// Begin sprite data. The last byte on 212 isn't used.
/*204*/ 0b10000000'00000000,
/*206*/ 0b00000000'00000000,
/*208*/ 0b00000000'00000000,
/*20a*/ 0b00000000'00000000,
/*20c*/ 0b00000000'00000000,
/*20e*/ 0b00000000'00000000,
/*210*/ 0b00000000'00000000,
/*212*/ 0b00000000'00000000,
// End sprite data

/*214*/ CLS             
/*216*/ LD6(0)          // x = DT
/*218*/ SNE1(0, 0)      // x != 0 i.e. DT up
/*21a*/ CALL(0x21e)     // Call IncY()
/*21c*/ JP(0x216)       

// Begin func IncY
/*21e*/ LD1(0, 3)      // Begin function call
/*220*/ LD5(0)          // DT = 3 cycles
/*222*/ DRW(1, 2, 15)   
/*224*/ ADD1(2, 1)      // increment Y position
/*226*/ SNE1(2, 32)     // Y position != 32
/*228*/ CALL(0x22c)     // Call InxX()
/*22a*/ RET             
// End func

// Begin func IncX
/*22c*/ ADD1(1, 1)      // inscrement x
/*22e*/ LD1(3, 32)      
/*230*/ SUB(2, 3)       // reset y to 0
/*232*/ RET             
// End func
};