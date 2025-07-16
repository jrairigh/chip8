#include "codes.h"
#include "chip8.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void chip8_load_program(uint16_t* program, size_t program_size);
void chip8_initialize(const char* rom);
void chip8_cycle();

extern Chip8 s_chip8;

#define BEGIN_TEST(name) { \
    chip8_initialize(""); \
    s_chip8.speed = 1; \
    total_tests++; \
    const char* test_name = name; \
    uint16_t program[] = {

#define RUN_TEST }; \
    chip8_load_program(program, sizeof(program) / sizeof(uint16_t));\
    while(!s_chip8.halted) \
        chip8_cycle(); \
    bool passed = true;
    
#define ASSERT_REG(reg, value) passed = passed && (s_chip8.v[reg] == value);
#define ASSERT_SP(value) passed = passed && (s_chip8.sp == value);
#define ASSERT_PC(value) passed = passed && (s_chip8.pc == value);
#define ASSERT_INDEX(value) passed = passed && (s_chip8.index == value);
#define ASSERT_STACK(level, value) passed = passed && (s_chip8.stack[level] == value);
#define ASSERT_DT(value) passed = passed && (s_chip8.delay_timer == value);
#define ASSERT_ST(value) passed = passed && (s_chip8.sound_timer == value);
#define ASSERT_MEM(index, value) passed = passed && (s_chip8.ram[index] == value);

#define END_TEST \
    if(passed) { \
        passed_tests++; \
    } \
    printf("Test %s\n%s\n", test_name, passed ? "PASSED" : "FAILED");}

void chip8_run_tests()
{
    int passed_tests = 0, total_tests = 0;

    BEGIN_TEST("Jump to address")
        JP(0x300)
        RUN_TEST
        ASSERT_PC(0x302)
    END_TEST

    BEGIN_TEST("Jump w/offset")
        LD1(0x0, 2)
        JP1(0x300)
        RUN_TEST
        ASSERT_PC(0x304)
    END_TEST

    BEGIN_TEST("Return")
        CALL(PROGRAM_START + 8)
        0, /*Cause HALT*/
        0,
        0,
        RET
        RUN_TEST
        ASSERT_PC(PROGRAM_START + 2 + 2)
        ASSERT_STACK(0, 0)
        ASSERT_SP(0)
    END_TEST

    BEGIN_TEST("Call")
        CALL(0x800)
        RUN_TEST
        ASSERT_SP(1)
        ASSERT_STACK(0, PROGRAM_START + 2)
        ASSERT_PC(0x800 + 2)
    END_TEST

    BEGIN_TEST("Skip Next (x == byte) Case 1")
        LD1(0xC, 0xCD)
        SE1(0xC, 0xCD)
        LD1(0xD, 0xAB)
        ADD1(0xD, 0x01)
        RUN_TEST
        ASSERT_REG(0xD, 0x01)
    END_TEST

    BEGIN_TEST("Skip Next (x == byte) Case 2")
        SE1(0xC, 0xCD)
        LD1(0xD, 0xAB)
        ADD1(0xD, 0x01)
        RUN_TEST
        ASSERT_REG(0xD, 0xAC)
    END_TEST

    BEGIN_TEST("Skip Next (x != byte) Case 1")
        LD1(0xC, 0xCD)
        SNE1(0xC, 0xCD)
        LD1(0xD, 0xAB)
        ADD1(0xD, 0x01)
        RUN_TEST
        ASSERT_REG(0xD, 0xAC)
    END_TEST

    BEGIN_TEST("Skip Next (x != byte) Case 2")
        SNE1(0xC, 0xCD)
        LD1(0xD, 0xAB)
        ADD1(0xD, 0x01)
        RUN_TEST
        ASSERT_REG(0xD, 0x01)
    END_TEST

    BEGIN_TEST("Skip Next (x == y) Case 1")
        SE2(0xC, 0xE)
        ADD1(0xD, 0x01)
        RUN_TEST
        ASSERT_REG(0xD, 0x00)
    END_TEST

    BEGIN_TEST("Skip Next (x == y) Case 2")
        LD1(0xC, 0xEF)
        SE2(0xC, 0xE)
        ADD1(0xD, 0x01)
        RUN_TEST
        ASSERT_REG(0xD, 0x01)
    END_TEST

    BEGIN_TEST("Load byte")
        LD1(1, 0xcd)
        RUN_TEST
        ASSERT_REG(1, 0xcd)
    END_TEST

    BEGIN_TEST("Load register")
        LD1(0xD, 0x69)
        LD2(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0x69)
    END_TEST

    BEGIN_TEST("Load index")
        LDB(0x300)
        RUN_TEST
        ASSERT_INDEX(0x300)
    END_TEST

    BEGIN_TEST("Load font 0")
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D0)
    END_TEST

    BEGIN_TEST("Load font 1")
        LD1(0x0, 1)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D1)
    END_TEST

    BEGIN_TEST("Load font 2")
        LD1(0x0, 2)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D2)
    END_TEST

    BEGIN_TEST("Load font 3")
        LD1(0x0, 3)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D3)
    END_TEST

    BEGIN_TEST("Load font 4")
        LD1(0x0, 4)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D4)
    END_TEST

    BEGIN_TEST("Load font 5")
        LD1(0x0, 5)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D5)
    END_TEST

    BEGIN_TEST("Load font 6")
        LD1(0x0, 6)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D6)
    END_TEST

    BEGIN_TEST("Load font 7")
        LD1(0x0, 7)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D7)
    END_TEST

    BEGIN_TEST("Load font 8")
        LD1(0x0, 8)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D8)
    END_TEST

    BEGIN_TEST("Load font 9")
        LD1(0x0, 9)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(D9)
    END_TEST

    BEGIN_TEST("Load font A")
        LD1(0x0, 0xA)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(DA)
    END_TEST

    BEGIN_TEST("Load font B")
        LD1(0x0, 0xB)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(DB)
    END_TEST

    BEGIN_TEST("Load font C")
        LD1(0x0, 0xC)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(DC)
    END_TEST

    BEGIN_TEST("Load font D")
        LD1(0x0, 0xD)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(DD)
    END_TEST

    BEGIN_TEST("Load font E")
        LD1(0x0, 0xE)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(DE)
    END_TEST

    BEGIN_TEST("Load font F")
        LD1(0x0, 0xF)
        LD7(0x0)
        RUN_TEST
        ASSERT_INDEX(DF)
    END_TEST

    BEGIN_TEST("Load BCD")
        LD1(0x0, 123)
        LDB(0x800)
        LD8(0x0)
        RUN_TEST
        ASSERT_MEM(0x800, 1)
        ASSERT_MEM(0x801, 2)
        ASSERT_MEM(0x802, 3)
    END_TEST

    BEGIN_TEST("Load BCD fails")
        LD1(0x0, 1)
        LD8(0x0)
        RUN_TEST
        ASSERT_MEM(0x000, 0xF0) // shouldn't overwrite first byte of 0 digit
    END_TEST

    BEGIN_TEST("Load delay timer (Set/Get)")
        LD1(0x0, 60)
        LD5(0x0)
        LD6(0x1)
        RUN_TEST
        ASSERT_DT(57)
        ASSERT_REG(0x1, 59)
    END_TEST

    BEGIN_TEST("Load vector (write)")
        LD1(0x0, 0xF0)
        LD1(0x1, 0x10)
        LD1(0x2, 0x20)
        LD1(0x3, 0x40)
        LD1(0x4, 0x40)
        LDB(0x800)
        LD9(0x4)
        RUN_TEST
        ASSERT_MEM(0x800, 0xF0)
        ASSERT_MEM(0x801, 0x10)
        ASSERT_MEM(0x802, 0x20)
        ASSERT_MEM(0x803, 0x40)
        ASSERT_MEM(0x804, 0x40)
    END_TEST

    BEGIN_TEST("Load vector (write fails)")
        LD9(0x4) // trys to overwrite digit 0 with zeroes
        RUN_TEST
        ASSERT_MEM(0x000, 0xF0)
        ASSERT_MEM(0x001, 0x90)
        ASSERT_MEM(0x002, 0x90)
        ASSERT_MEM(0x003, 0x90)
        ASSERT_MEM(0x004, 0xF0)
    END_TEST

    BEGIN_TEST("Load vector (read)")
        LDB(D7)
        LDA(0xE)
        RUN_TEST
        // digit 7
        ASSERT_REG(0x0, 0xF0)
        ASSERT_REG(0x1, 0x10)
        ASSERT_REG(0x2, 0x20)
        ASSERT_REG(0x3, 0x40)
        ASSERT_REG(0x4, 0x40)
        // digit 8
        ASSERT_REG(0x5, 0xF0)
        ASSERT_REG(0x6, 0x90)
        ASSERT_REG(0x7, 0xF0)
        ASSERT_REG(0x8, 0x90)
        ASSERT_REG(0x9, 0xF0)
        // digit 9
        ASSERT_REG(0xA, 0xF0)
        ASSERT_REG(0xB, 0x90)
        ASSERT_REG(0xC, 0xF0)
        ASSERT_REG(0xD, 0x10)
        ASSERT_REG(0xE, 0xF0)
    END_TEST

    BEGIN_TEST("Add byte")
        ADD1(0xA, 0x01)
        ADD1(0xA, 0x02)
        ADD1(0xA, 0x03)
        RUN_TEST
        ASSERT_REG(0xA, 0x06)
    END_TEST

    BEGIN_TEST("Add register w/carry")
        LD1(0x4, 0xFF)
        LD1(0x5, 0x01)
        ADD2(0x4, 0x5)
        RUN_TEST
        ASSERT_REG(0x4, 0x00)
        ASSERT_REG(0xF, 0x01)
    END_TEST
        
    BEGIN_TEST("Add register wo/carry")
        LD1(0x4, 0xFE)
        LD1(0x5, 0x01)
        ADD2(0x4, 0x5)
        RUN_TEST
        ASSERT_REG(0x4, 0xFF)
        ASSERT_REG(0xF, 0x00)
    END_TEST

    BEGIN_TEST("Add to index")
        LD1(0x4, 0x80)
        LDB(0x800)
        ADD3(0x4)
        RUN_TEST
        ASSERT_INDEX(0x880)
    END_TEST

    BEGIN_TEST("Subtract w/borrow")
        LD1(0xC, 0x01)
        SUB(0xD, 0xC)
        RUN_TEST
        ASSERT_REG(0xD, 0xFF)
        ASSERT_REG(0xF, 0x00)
    END_TEST

    BEGIN_TEST("Subtract wo/borrow")
        LD1(0xD, 0x01)
        LD1(0xC, 0x10)
        SUB(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0x0F)
        ASSERT_REG(0xF, 0x01)
    END_TEST

    BEGIN_TEST("SUBN w/borrow")
        LD1(0xC, 0x1)
        SUBN(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0xFF)
        ASSERT_REG(0xF, 0x00)
    END_TEST

    BEGIN_TEST("SUBN wo/borrow")
        LD1(0xC, 0x01)
        LD1(0xD, 0x10)
        SUBN(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0x0F)
        ASSERT_REG(0xF, 0x01)
    END_TEST

    BEGIN_TEST("Or")
        LD1(0xD, 0x07)
        LD1(0xC, 0x0F)
        OR(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0x0F)
    END_TEST

    BEGIN_TEST("And")
        LD1(0xD, 0x07)
        LD1(0xC, 0x0F)
        AND(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0x07)
    END_TEST

    BEGIN_TEST("Xor")
        LD1(0xD, 0x07)
        LD1(0xC, 0x0F)
        XOR(0xC, 0xD)
        RUN_TEST
        ASSERT_REG(0xC, 0x08)
    END_TEST

    BEGIN_TEST("Shift right (odd)")
        LD1(0xC, 0x0F)
        SHR(0xC)
        RUN_TEST
        ASSERT_REG(0xC, 0x07)
        ASSERT_REG(0xF, 0x01)
    END_TEST

    BEGIN_TEST("Shift right (even)")
        LD1(0xC, 0x10)
        SHR(0xC)
        RUN_TEST
        ASSERT_REG(0xC, 0x08)
        ASSERT_REG(0xF, 0x00)
    END_TEST

    BEGIN_TEST("Shift left w/overflow")
        LD1(0xC, 0x80)
        SHL(0xC)
        RUN_TEST
        ASSERT_REG(0xC, 0x00)
        ASSERT_REG(0xF, 0x01)
    END_TEST

    BEGIN_TEST("Shift left wo/overflow")
        LD1(0xC, 0x08)
        SHL(0xC)
        RUN_TEST
        ASSERT_REG(0xC, 0x10)
        ASSERT_REG(0xF, 0x00)
    END_TEST

    printf("Tests passed %d/%d\n", passed_tests, total_tests);
}
