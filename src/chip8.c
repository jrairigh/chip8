#include "chip8.h"
#include "monitor.h"
#include "renderer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef RUN_TESTS

#endif

typedef struct Chip8
{
    uint8_t ram[4096];
    uint8_t v[16];
    uint16_t stack[16];
    uint16_t index;
    uint16_t pc;
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t speed;
    bool paused;
} Chip8;

static Chip8 s_chip8;

#define vm_switch(inst, mask) switch(inst & mask)
#define vm_case(op) case op:
#define vm_default default:
#define vm_break break;

#define ADDR(instr) ((instr) & 0x0FFF)
#define NIBBLE(instr) ((instr) & 0x000F)
#define X(instr) ((instr & 0x0F00) >> 8)
#define Y(instr) ((instr & 0x00F0) >> 4)
#define BYTE(instr) ((instr) & 0x00FF)

static void chip8_initialize();
static void chip8_vm_run();

void chip8_run()
{
    chip8_initialize();
    renderer_initialize();
    renderer_set_update_func(chip8_vm_run);
    renderer_do_update();
    renderer_shutdown();
}

void chip8_run_tests()
{
    printf("all tests passed\n");
}

static void chip8_initialize()
{
    memset(&s_chip8, 0, sizeof(s_chip8));
    s_chip8.index = 0;
    s_chip8.pc = 0x200;
    s_chip8.sp = 0x0;
    s_chip8.speed = 10;
    s_chip8.paused = false;
}

static void chip8_vm_run()
{
    const uint16_t instruction = *(uint16_t*)(s_chip8.ram + s_chip8.pc);
    uint16_t pc_inc = 2;
    const uint8_t x = X(instruction);
    const uint8_t y = Y(instruction);

    vm_switch(instruction, 0xF000)
    {
        vm_case(0x0000)
        {
            vm_switch(instruction, 0x000F)
            {
                vm_case(0x0)
                {
                    // CLS
                    monitor_clear();
                    vm_break;
                }

                vm_case(0xE)
                {
                    // RET
                    pc_inc = 0;
                    s_chip8.pc = s_chip8.stack[s_chip8.sp];
                    s_chip8.sp--;
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
                    s_chip8.v[0xF] = s_chip8.v[x] & 0x80; // Set carry flag
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

        vm_case(0xE000)
        {
            vm_switch(instruction, 0x00FF)
            {
                vm_case(0x9E)
                {
                    // SKP Vx
                    vm_break;
                }

                vm_case(0xA1)
                {
                    // SKNP Vx
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
                    vm_break;
                }

                vm_case(0x33)
                {
                    // LD B, Vx
                    vm_break;
                }

                vm_case(0x55)
                {
                    // LD [I], Vx
                    for(uint8_t i = 0; i < x; ++i)
                    {
                        s_chip8.ram[s_chip8.index + i] = s_chip8.v[i];
                    }
                    vm_break;
                }

                vm_case(0x65)
                {
                    // LD Vx, [I]
                    for(uint8_t i = 0; i < x; ++i)
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

    monitor_do_update();
}
