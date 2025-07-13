#include "codes.h"
#include "chip8.h"
#include "monitor.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define vm_switch(inst, mask) switch(inst & mask)
#define vm_case(op) case op:
#define vm_default default:
#define vm_break break;

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

#ifdef RUN_TESTS
#define BEGIN_TEST(name) { \
    chip8_initialize(); \
    total_tests++; \
    const char* test_name = name; \
    uint16_t program[] = {

#define RUN_TEST }; \
    memcpy(&s_chip8.ram[PROGRAM_START], program, sizeof(program)); \
    while((*(uint16_t*)&s_chip8.ram[s_chip8.pc]) != 0x0000) \
        chip8_vm_run(); \
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
    printf("Test %-35s %s\n", test_name, passed ? "PASSED" : "FAILED");}
#endif

Chip8 s_chip8;
static void chip8_initialize();
static void chip8_vm_run();

#ifndef RUN_TESTS
void chip8_run()
{
    chip8_initialize();
    uint16_t program[] = {
#include "program.txt"
    };

    memcpy(&s_chip8.ram[s_chip8.pc], program, sizeof(program));
    renderer_initialize();
    renderer_set_update_func(chip8_vm_run);
    renderer_do_update();
    renderer_shutdown();
}
#else
void chip8_run_tests()
{
    int passed_tests = 0, total_tests = 0;

    BEGIN_TEST("Jump to address")
        JP(0x300)
        RUN_TEST
        ASSERT_PC(0x300)
    END_TEST

    BEGIN_TEST("Jump w/offset")
        LD1(0x0, 2)
        JP1(0x300)
        RUN_TEST
        ASSERT_PC(0x302)
    END_TEST

    BEGIN_TEST("Return")
        CALL(0x208)
        0, /*0x202 */
        0, /*0x204 */
        0, /*0x206*/
        RET
        RUN_TEST
        ASSERT_PC(0x202)
        ASSERT_STACK(1, PROGRAM_START)
        ASSERT_SP(0)
    END_TEST

    BEGIN_TEST("Call")
        CALL(0xFFE)
        RUN_TEST
        ASSERT_SP(1)
        ASSERT_STACK(1, PROGRAM_START)
        ASSERT_PC(0xFFE)
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
        ASSERT_DT(58)
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
#endif

static void chip8_initialize()
{
    memset(&s_chip8, 0, sizeof(s_chip8));
    s_chip8.index = 0;
    s_chip8.pc = PROGRAM_START;
    s_chip8.sp = 0;
    s_chip8.speed = 10;
    s_chip8.paused = false;

    // Store font set into interpreter area of memory (0x000 to 0x1FF)
    memcpy(&s_chip8.ram[D0], (uint8_t[5]){0xF0, 0x90, 0x90, 0x90, 0xF0}, 5);
    memcpy(&s_chip8.ram[D1], (uint8_t[5]){0x20, 0x60, 0x20, 0x20, 0x70}, 5);
    memcpy(&s_chip8.ram[D2], (uint8_t[5]){0xF0, 0x10, 0xF0, 0x80, 0xF0}, 5);
    memcpy(&s_chip8.ram[D3], (uint8_t[5]){0xF0, 0x10, 0xF0, 0x10, 0xF0}, 5);
    memcpy(&s_chip8.ram[D4], (uint8_t[5]){0x90, 0x90, 0xF0, 0x10, 0x10}, 5);
    memcpy(&s_chip8.ram[D5], (uint8_t[5]){0xF0, 0x80, 0xF0, 0x10, 0xF0}, 5);
    memcpy(&s_chip8.ram[D6], (uint8_t[5]){0xF0, 0x80, 0xF0, 0x90, 0xF0}, 5);
    memcpy(&s_chip8.ram[D7], (uint8_t[5]){0xF0, 0x10, 0x20, 0x40, 0x40}, 5);
    memcpy(&s_chip8.ram[D8], (uint8_t[5]){0xF0, 0x90, 0xF0, 0x90, 0xF0}, 5);
    memcpy(&s_chip8.ram[D9], (uint8_t[5]){0xF0, 0x90, 0xF0, 0x10, 0xF0}, 5);
    memcpy(&s_chip8.ram[DA], (uint8_t[5]){0xF0, 0x90, 0xF0, 0x90, 0x90}, 5);
    memcpy(&s_chip8.ram[DB], (uint8_t[5]){0xE0, 0x90, 0xE0, 0x90, 0xE0}, 5);
    memcpy(&s_chip8.ram[DC], (uint8_t[5]){0xF0, 0x80, 0x80, 0x80, 0xF0}, 5);
    memcpy(&s_chip8.ram[DD], (uint8_t[5]){0xE0, 0x90, 0x90, 0x90, 0xE0}, 5);
    memcpy(&s_chip8.ram[DE], (uint8_t[5]){0xF0, 0x80, 0xF0, 0x80, 0xF0}, 5);
    memcpy(&s_chip8.ram[DF], (uint8_t[5]){0xF0, 0x80, 0xF0, 0x80, 0x80}, 5);
}

static void chip8_vm_run()
{
    assert((s_chip8.pc & 0x1) == 0); // Ensure PC is even
    const uint16_t instruction = *(uint16_t*)(s_chip8.ram + s_chip8.pc);
    uint16_t pc_inc = 2;
    const uint8_t x = X(instruction);
    const uint8_t y = Y(instruction);

    vm_switch(instruction, 0xF000)
    {
        vm_case(0x0000)
        {
            vm_switch(instruction, 0x00FF)
            {
                vm_case(0xE0)
                {
                    // CLS
                    monitor_clear();
                    vm_break;
                }

                vm_case(0xEE)
                {
                    // RET
                    s_chip8.pc = s_chip8.stack[s_chip8.sp];
                    s_chip8.sp--;
                    vm_break;
                }

                vm_default
                {
                    // halt
                    renderer_log("HALTED");
                    pc_inc = 0;
                    vm_break;
                }
            }

            vm_break;
        }

        vm_case(0x1000)
        {
            // JP addr
            pc_inc = 0;
            s_chip8.pc = ADDR(instruction);
            vm_break;
        }

        vm_case(0x2000)
        {
            // CALL addr
            pc_inc = 0;
            s_chip8.sp++;
            s_chip8.stack[s_chip8.sp] = s_chip8.pc;
            s_chip8.pc = ADDR(instruction);
            vm_break;
        }

        vm_case(0x3000)
        {
            // SE Vx, byte
            s_chip8.pc += (uint16_t)(s_chip8.v[x] == BYTE(instruction)) << 1;
            vm_break;
        }

        vm_case(0x4000)
        {
            // SNE Vx, byte
            s_chip8.pc += (uint16_t)(s_chip8.v[x] != BYTE(instruction)) << 1;
            vm_break;
        }

        vm_case(0x5000)
        {
            // SE Vx, Vy
            s_chip8.pc += (uint16_t)(s_chip8.v[x] == s_chip8.v[y]) << 1;
            vm_break;
        }

        vm_case(0x6000)
        {
            // LD Vx, byte
            s_chip8.v[x] = BYTE(instruction);
            vm_break;
        }
        
        vm_case(0x7000)
        {
            // ADD Vx, byte
            s_chip8.v[x] += BYTE(instruction);
            vm_break;
        }

        vm_case(0x8000)
        {
            vm_switch(instruction, 0xF)
            {
                vm_case(0x0)
                {
                    // LD Vx, Vy
                    s_chip8.v[x] = s_chip8.v[y];
                    vm_break;
                }

                vm_case(0x1)
                {
                    // OR Vx, Vy
                    s_chip8.v[x] |= s_chip8.v[y];
                    vm_break;
                }

                vm_case(0x2)
                {
                    // AND Vx, Vy
                    s_chip8.v[x] &= s_chip8.v[y];
                    vm_break;
                }

                vm_case(0x3)
                {
                    // XOR Vx, Vy
                    s_chip8.v[x] ^= s_chip8.v[y];
                    vm_break;
                }

                vm_case(0x4)
                {
                    // ADD Vx, Vy
                    const uint8_t max_value = 0xFF - s_chip8.v[x];
                    s_chip8.v[0xF] = max_value < s_chip8.v[y]; // Set carry flag
                    s_chip8.v[x] += s_chip8.v[y];
                    vm_break;
                }

                vm_case(0x5)
                {
                    // SUB Vx, Vy
                    s_chip8.v[0xF] = s_chip8.v[x] > s_chip8.v[y]; // Set borrow flag
                    s_chip8.v[x] -= s_chip8.v[y];
                    vm_break;
                }

                vm_case(0x6)
                {
                    // SHR Vx {, Vy}
                    s_chip8.v[0xF] = s_chip8.v[x] & 0x1; // Set carry flag
                    s_chip8.v[x] >>= 1;
                    vm_break;
                }

                vm_case(0x7)
                {
                    // SUBN Vx, Vy
                    s_chip8.v[0xF] = s_chip8.v[y] > s_chip8.v[x]; // Set borrow flag
                    s_chip8.v[x] = s_chip8.v[y] - s_chip8.v[x];
                    vm_break;
                }

                vm_case(0xE)
                {
                    // SHL Vx {, Vy}
                    s_chip8.v[0xF] = (s_chip8.v[x] & 0x80) > 0; // Set carry flag
                    s_chip8.v[x] <<= 1;
                    vm_break;
                }
            }
            vm_break;
        }

        vm_case(0x9000)
        {
            // SNE Vx, Vy
            s_chip8.pc += (uint16_t)(s_chip8.v[x] != s_chip8.v[y]) << 1;
            vm_break;
        }

        vm_case(0xA000)
        {
            // LD I, addr
            s_chip8.index = ADDR(instruction);
            vm_break;
        }

        vm_case(0xB000)
        {
            // JP V0, addr
            pc_inc = 0;
            s_chip8.pc = ADDR(instruction) + s_chip8.v[0];
            vm_break;
        }

        vm_case(0xC000)
        {
            // RND Vx, byte
            s_chip8.v[x] = (rand() % 256) & BYTE(instruction);
            vm_break;
        }

        vm_case(0xD000)
        {
            // DRW Vx, Vy, nibble
            monitor_draw_sprite(s_chip8.v[x], s_chip8.v[y], s_chip8.ram + s_chip8.index, NIBBLE(instruction), (bool*)&s_chip8.v[0xF]);
            vm_break;
        }

        // Keyboard instructions
        vm_case(0xE000)
        {
            vm_switch(instruction, 0x00FF)
            {
                vm_case(0x9E)
                {
                    // SKP Vx
                    s_chip8.pc += monitor_is_key_down(s_chip8.v[x]) << 1;
                    vm_break;
                }

                vm_case(0xA1)
                {
                    // SKNP Vx
                    s_chip8.pc += (monitor_is_key_down(s_chip8.v[x]) == 0) << 1;
                    vm_break;
                }
            }
            
            vm_break;
        }

        vm_case(0xF000)
        {
            vm_switch(instruction, 0x00FF)
            {
                vm_case(0x07)
                {
                    // LD Vx, DT
                    s_chip8.v[x] = s_chip8.delay_timer;
                    vm_break;
                }

                vm_case(0x0A)
                {
                    // LD Vx, K
                    uint8_t key;
                    bool is_pressed = monitor_get_key(&key);
                    pc_inc = 0;
                    if(is_pressed)
                    {
                        s_chip8.v[x] = key;
                        pc_inc = 2;
                    }

                    vm_break;
                }

                vm_case(0x15)
                {
                    // LD DT, Vx
                    s_chip8.delay_timer = s_chip8.v[x];
                    vm_break;
                }

                vm_case(0x18)
                {
                    // LD ST, Vx
                    s_chip8.sound_timer = s_chip8.v[x];
                    vm_break;
                }

                vm_case(0x1E)
                {
                    // ADD I, Vx
                    s_chip8.index += (uint16_t)s_chip8.v[x];
                    vm_break;
                }

                vm_case(0x29)
                {
                    // LD F, Vx
                    s_chip8.index = D0 + NIBBLE(s_chip8.v[x]) * 5;
                    vm_break;
                }

                vm_case(0x33)
                {
                    // LD B, Vx
                    uint8_t value = s_chip8.v[x];
                    const uint8_t ones = value % 10;
                    value /= 10;
                    const uint8_t tens = value % 10;
                    value /= 10;
                    const uint8_t hundreds = value;

                    if(s_chip8.index >= PROGRAM_START)
                    {
                        s_chip8.ram[s_chip8.index] = hundreds;
                        s_chip8.ram[s_chip8.index + 1] = tens;
                        s_chip8.ram[s_chip8.index + 2] = ones;
                    }

                    vm_break;
                }

                vm_case(0x55)
                {
                    // LD [I], Vx
                    for(uint8_t i = 0; i <= x; ++i)
                    {
                        if((s_chip8.index + i) >= PROGRAM_START)
                        {
                            s_chip8.ram[s_chip8.index + i] = s_chip8.v[i];
                        }
                    }

                    vm_break;
                }

                vm_case(0x65)
                {
                    // LD Vx, [I]
                    for(uint8_t i = 0; i <= x; ++i)
                    {
                        s_chip8.v[i] = s_chip8.ram[s_chip8.index + i];
                    }

                    vm_break;
                }
            }

            vm_break;
        }
    }

    s_chip8.pc += pc_inc;

    if(s_chip8.delay_timer > 0)
    {
        --s_chip8.delay_timer;
    }

    if(s_chip8.sound_timer > 0)
    {
        --s_chip8.sound_timer;
    }
}
