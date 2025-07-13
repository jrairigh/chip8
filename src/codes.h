#ifndef CODES_H
#define CODES_H

#define PROGRAM_START 0x200

// Interpreter digit start at 0x000
#define D0 0x000
#define D1 (D0 + (5 * 1))
#define D2 (D0 + (5 * 2))
#define D3 (D0 + (5 * 3))
#define D4 (D0 + (5 * 4))
#define D5 (D0 + (5 * 5))
#define D6 (D0 + (5 * 6))
#define D7 (D0 + (5 * 7))
#define D8 (D0 + (5 * 8))
#define D9 (D0 + (5 * 9))
#define DA (D0 + (5 * 10))
#define DB (D0 + (5 * 11))
#define DC (D0 + (5 * 12))
#define DD (D0 + (5 * 13))
#define DE (D0 + (5 * 14))
#define DF (D0 + (5 * 15))

#define ADDR(instr) ((instr) & 0x0FFF)
#define NIBBLE(instr) ((instr) & 0x000F)
#define X(instr) ((instr & 0x0F00) >> 8)
#define Y(instr) ((instr & 0x00F0) >> 4)
#define BYTE(instr) ((instr) & 0x00FF)

#define REGX(x) ((x << 8) & 0x0F00)
#define REGY(y) ((y << 4) & 0x00F0)

#define CLS 0x00E0,
#define RET 0x00EE,
#define JP(addr) (0x1000 | ADDR(addr)),
#define JP1(addr) (0xB000 | ADDR(addr)),
#define CALL(addr) (0x2000 | ADDR(addr)),
#define DRW(x, y, n) (0xD000 | REGX(x) | REGY(y) | NIBBLE(n)),
#define RND(x, byte) (0xC000 | REGX(x) | BYTE(byte)),
#define AND(x, y) (0x8002 | REGX(x) | REGY(y)),
#define OR(x, y) (0x8001 | REGX(x) | REGY(y)),
#define XOR(x, y) (0x8003 | REGX(x) | REGY(y)),
#define SUB(x, y) (0x8005 | REGX(x) | REGY(y)),
#define SUBN(x, y) (0x8007 | REGX(x) | REGY(y)),
#define SHR(x) (0x8006 | REGX(x)),
#define SHL(x) (0x800E | REGX(x)),
#define SKP(x) (0xE09E | REGX(x)),
#define SKNP(x) (0xE0A1 | REGX(x)),

#define SE1(x, byte) (0x3000 | REGX(x) | BYTE(byte)),
#define SE2(x, y) (0x5000 | REGX(x) | REGY(y)),

#define SNE1(x, byte) (0x4000 | REGX(x) | BYTE(byte)),
#define SNE2(x, y) (0x9000 | REGX(x) | REGY(y)),

#define LD1(x, byte) (0x6000 | REGX(x) | BYTE(byte)),
#define LD2(x, y) (0x8000 | REGX(x) | REGY(y)),
#define LD3(x) (0xF00A | REGX(x)),
#define LD4(x) (0xF018 | REGX(x)),
#define LD5(x) (0xF015 | REGX(x)),
#define LD6(x) (0xF007 | REGX(x)),
#define LD7(x) (0xF029 | REGX(x)),
#define LD8(x) (0xF033 | REGX(x)),
#define LD9(x) (0xF055 | REGX(x)),
#define LDA(x) (0xF065 | REGX(x)),
#define LDB(addr) (0xA000 | ADDR(addr)),

#define ADD1(x, byte) (0x7000 | REGX(x) | BYTE(byte)),
#define ADD2(x, y) (0x8004 | REGX(x) | REGY(y)),
#define ADD3(x) (0xF01E | REGX(x)),

#endif
