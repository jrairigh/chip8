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

Chip8 s_chip8;
void chip8_initialize();
void chip8_vm_run();

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
void chip8_run_tests();
void chip8_run()
{
    chip8_run_tests();
}
#endif

void chip8_initialize()
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

void chip8_vm_run()
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
