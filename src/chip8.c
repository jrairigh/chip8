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

static void chip8_load_rom(const char* rom_path);
static void chip8_vm_run(const uint16_t instruction);
static void chip8_shutdown();
void chip8_initialize(const char* rom);
void chip8_cycle();
void chip8_load_program(uint16_t* program, size_t program_size);

#ifndef RUN_TESTS
void chip8_run()
{
    monitor_initialize(chip8_initialize, chip8_cycle, chip8_shutdown);
}
#else
void chip8_run_tests();
void chip8_run()
{
    chip8_run_tests();
}
#endif

void chip8_initialize(const char* rom)
{
    memset(&s_chip8, 0, sizeof(s_chip8));
    s_chip8.index = 0;
    s_chip8.pc = PROGRAM_START;
    s_chip8.sp = 0;
    s_chip8.speed = 10;
    s_chip8.halted = false;
    s_chip8.paused = false;

    // Store font set into interpreter area of memory (0x000 to 0x1FF)
    const uint8_t digits[] = {
        /*0*/0xF0, 0x90, 0x90, 0x90, 0xF0,
        /*1*/0x20, 0x60, 0x20, 0x20, 0x70,
        /*2*/0xF0, 0x10, 0xF0, 0x80, 0xF0,
        /*3*/0xF0, 0x10, 0xF0, 0x10, 0xF0,
        /*4*/0x90, 0x90, 0xF0, 0x10, 0x10,
        /*5*/0xF0, 0x80, 0xF0, 0x10, 0xF0,
        /*6*/0xF0, 0x80, 0xF0, 0x90, 0xF0,
        /*7*/0xF0, 0x10, 0x20, 0x40, 0x40,
        /*8*/0xF0, 0x90, 0xF0, 0x90, 0xF0,
        /*9*/0xF0, 0x90, 0xF0, 0x10, 0xF0,
        /*A*/0xF0, 0x90, 0xF0, 0x90, 0x90,
        /*B*/0xE0, 0x90, 0xE0, 0x90, 0xE0,
        /*C*/0xF0, 0x80, 0x80, 0x80, 0xF0,
        /*D*/0xE0, 0x90, 0x90, 0x90, 0xE0,
        /*E*/0xF0, 0x80, 0xF0, 0x80, 0xF0,
        /*F*/0xF0, 0x80, 0xF0, 0x80, 0x80
    };

    memcpy(&s_chip8.ram[D0], digits, sizeof(digits));

    if(strlen(rom) > 0)
    {
        char chBuffer[256];
        sprintf(chBuffer, "Loading ROM %s", rom);
        monitor_log(LOG_INFO, chBuffer);
        chip8_load_rom(rom);
    }

    monitor_clear();
}

void chip8_cycle()
{
    if(s_chip8.halted)
    {
        return;
    }

    if(s_chip8.paused)
    {
        // The only instruction that pauses machine is waiting for key press
        // The reason the code is removed from the main chip_vm_run function is because
        // raylib returns true each time GetKeyPressed is called in the current frame.
        // Because the vm may run more than one cycle per frame this caused back to back
        // get key instructions to not wait for input.

        const uint16_t upper = (uint16_t)s_chip8.ram[s_chip8.pc];
        const uint16_t lower = (uint16_t)s_chip8.ram[s_chip8.pc + 1];
        const uint8_t x = X((upper << 8) | lower);

        uint8_t key;
        bool is_pressed = monitor_get_key(&key);
        if(is_pressed)
        {
            s_chip8.v[x] = key;
            s_chip8.paused = false;
        }

        return;
    }

    for(int32_t i = 0; i < s_chip8.speed; ++i)
    {
        const uint16_t upper = (uint16_t)s_chip8.ram[s_chip8.pc];
        const uint16_t lower = (uint16_t)s_chip8.ram[s_chip8.pc + 1];
        chip8_vm_run((upper << 8) | lower);

        if(s_chip8.paused)
        {
            return;
        }
    }

    if(s_chip8.delay_timer > 0)
    {
        --s_chip8.delay_timer;
    }

    if(s_chip8.sound_timer > 0)
    {
        --s_chip8.sound_timer;
        monitor_play_tone();
    }
    else
    {
        monitor_stop_tone();
    }
}

void chip8_load_program(uint16_t* program, size_t program_size)
{
    for(size_t i = 0, j = 0; i < program_size; ++i, j += 2)
    {
        const uint16_t instruction = program[i];
        const uint8_t lower = (uint8_t)(instruction & 0xFF);
        const uint8_t upper = (uint8_t)((instruction & 0xFF00) >> 8);
        s_chip8.ram[s_chip8.pc + j] = upper;
        s_chip8.ram[s_chip8.pc + j + 1] = lower;
    }
}

static void chip8_vm_run(const uint16_t instruction)
{
    assert((s_chip8.pc & 0x1) == 0); // Ensure PC is even
    const uint16_t currentPC = s_chip8.pc;

    s_chip8.pc += 2;

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
                    monitor_log(LOG_DEBUG, "%.04x: CLS", currentPC);
                    monitor_clear();
                    vm_break;
                }
                
                vm_case(0xEE)
                {
                    // RET
                    monitor_log(LOG_DEBUG, "%.04x: RET", currentPC);
                    s_chip8.sp--;
                    s_chip8.pc = s_chip8.stack[s_chip8.sp];
                    s_chip8.stack[s_chip8.sp] = 0;
                    vm_break;
                }

                vm_default
                {
                    // halt
                    monitor_log(LOG_INFO, "%.04x: HALTED 0x%.04x", currentPC, instruction);
                    s_chip8.halted = true;
                    vm_break;
                }
            }

            vm_break;
        }

        vm_case(0x1000)
        {
            // JP addr
            monitor_log(LOG_DEBUG, "%.04x: JP(0x%.04x)", currentPC, ADDR(instruction));
            s_chip8.pc = ADDR(instruction);
            vm_break;
        }

        vm_case(0x2000)
        {
            // CALL addr
            monitor_log(LOG_DEBUG, "%.04x: CALL(0x%.04x)", currentPC, ADDR(instruction));
            s_chip8.stack[s_chip8.sp] = s_chip8.pc;
            s_chip8.sp++;
            s_chip8.pc = ADDR(instruction);
            vm_break;
        }

        vm_case(0x3000)
        {
            // SE Vx, byte
            monitor_log(LOG_DEBUG, "%.04x: SE1(%d, 0x%.02x) // Skip if x == byte", currentPC, x, BYTE(instruction));
            s_chip8.pc += (uint16_t)(s_chip8.v[x] == BYTE(instruction)) << 1;
            vm_break;
        }
        
        vm_case(0x4000)
        {
            // SNE Vx, byte
            monitor_log(LOG_DEBUG, "%.04x: SNE1(%d, 0x%.02x) // Skip if x != byte", currentPC, x, BYTE(instruction));
            s_chip8.pc += (uint16_t)(s_chip8.v[x] != BYTE(instruction)) << 1;
            vm_break;
        }
        
        vm_case(0x5000)
        {
            // SE Vx, Vy
            monitor_log(LOG_DEBUG, "%.04x: SE2(%d, %d) // Skip if x == y", currentPC, x, y);
            s_chip8.pc += (uint16_t)(s_chip8.v[x] == s_chip8.v[y]) << 1;
            vm_break;
        }
        
        vm_case(0x6000)
        {
            // LD Vx, byte
            monitor_log(LOG_DEBUG, "%.04x: LD1(%d, 0x%.02x) // x = byte", currentPC, x, BYTE(instruction));
            s_chip8.v[x] = BYTE(instruction);
            vm_break;
        }
        
        vm_case(0x7000)
        {
            // ADD Vx, byte
            monitor_log(LOG_DEBUG, "%.04x: ADD1(%d, 0x%.02x) // x += byte", currentPC, x, BYTE(instruction));
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
                    monitor_log(LOG_DEBUG, "%.04x: LD2(%d, %d) // x = y", currentPC, x, y);
                    s_chip8.v[x] = s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x1)
                {
                    // OR Vx, Vy
                    monitor_log(LOG_DEBUG, "%.04x: OR(%d, %d) // x |= y", currentPC, x, y);
                    s_chip8.v[x] |= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x2)
                {
                    // AND Vx, Vy
                    monitor_log(LOG_DEBUG, "%.04x: AND(%d, %d) // x &= y", currentPC, x, y);
                    s_chip8.v[x] &= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x3)
                {
                    // XOR Vx, Vy
                    monitor_log(LOG_DEBUG, "%.04x: XOR(%d, %d) // x ^= y", currentPC, x, y);
                    s_chip8.v[x] ^= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x4)
                {
                    // ADD Vx, Vy
                    monitor_log(LOG_DEBUG, "%.04x: ADD2(%d, %d) // x += y", currentPC, x, y);
                    const uint8_t max_value = 0xFF - s_chip8.v[x];
                    s_chip8.v[0xF] = max_value < s_chip8.v[y]; // Set carry flag
                    s_chip8.v[x] += s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x5)
                {
                    // SUB Vx, Vy
                    monitor_log(LOG_DEBUG, "%.04x: SUB(%d, %d) // x -= y", currentPC, x, y);
                    s_chip8.v[0xF] = s_chip8.v[x] > s_chip8.v[y]; // Set borrow flag
                    s_chip8.v[x] -= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x6)
                {
                    // SHR Vx {, Vy}
                    monitor_log(LOG_DEBUG, "%.04x: SHR(%d) // x >>= 1", currentPC, x);
                    s_chip8.v[0xF] = s_chip8.v[x] & 0x1; // Set carry flag
                    s_chip8.v[x] >>= 1;
                    vm_break;
                }
                
                vm_case(0x7)
                {
                    // SUBN Vx, Vy
                    monitor_log(LOG_DEBUG, "%.04x: SUBN(%d, %d) // x = y - x", currentPC, x, y);
                    s_chip8.v[0xF] = s_chip8.v[y] > s_chip8.v[x]; // Set borrow flag
                    s_chip8.v[x] = s_chip8.v[y] - s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0xE)
                {
                    // SHL Vx {, Vy}
                    monitor_log(LOG_DEBUG, "%.04x: SHL(%d) // x <<= 1", currentPC, x);
                    s_chip8.v[0xF] = (s_chip8.v[x] & 0x80) > 0; // Set carry flag
                    s_chip8.v[x] <<= 1;
                    vm_break;
                }

                vm_default
                {
                    // halt
                    monitor_log(LOG_INFO, "%.04x: HALTED 0x%.04x", currentPC, instruction);
                    s_chip8.halted = true;
                    vm_break;
                }
            }
            vm_break;
        }
        
        vm_case(0x9000)
        {
            // SNE Vx, Vy
            monitor_log(LOG_DEBUG, "%.04x: SNE2(%d, %d) // Skip if x != y", currentPC, x, y);
            s_chip8.pc += (uint16_t)(s_chip8.v[x] != s_chip8.v[y]) << 1;
            vm_break;
        }
        
        vm_case(0xA000)
        {
            // LD I, addr
            monitor_log(LOG_DEBUG, "%.04x: LDB(%d) // index = addr", currentPC, ADDR(instruction));
            s_chip8.index = ADDR(instruction);
            vm_break;
        }
        
        vm_case(0xB000)
        {
            // JP V0, addr
            monitor_log(LOG_DEBUG, "%.04x: JP1(%d) // pc = addr + V0", currentPC, ADDR(instruction));
            s_chip8.pc = ADDR(instruction) + s_chip8.v[0];
            vm_break;
        }
        
        vm_case(0xC000)
        {
            // RND Vx, byte
            monitor_log(LOG_DEBUG, "%.04x: RND(%d, 0x%.02x) // x = rand()", currentPC, x, BYTE(instruction));
            s_chip8.v[x] = (rand() % 256) & BYTE(instruction);
            vm_break;
        }
        
        vm_case(0xD000)
        {
            // DRW Vx, Vy, nibble
            monitor_log(LOG_DEBUG, "%.04x: DRW(%d, %d, %d)", currentPC, x, y, NIBBLE(instruction));
            monitor_draw_sprite(s_chip8.v[x], s_chip8.v[y], s_chip8.ram + s_chip8.index, NIBBLE(instruction), (bool*)&s_chip8.v[0xF]);

            if(s_chip8.v[0xF])
            {
                monitor_log(LOG_DEBUG, "Collision detected");
            }

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
                    monitor_log(LOG_DEBUG, "%.04x: SKP(%d) // Skip if key down", currentPC, x);
                    s_chip8.pc += (uint16_t)(monitor_is_key_down(s_chip8.v[x])) << 1;
                    vm_break;
                }
                
                vm_case(0xA1)
                {
                    // SKNP Vx
                    monitor_log(LOG_DEBUG, "%.04x: SKNP(%d) // Skip if key not down", currentPC, x);
                    s_chip8.pc += (uint16_t)(monitor_is_key_down(s_chip8.v[x]) == 0) << 1;
                    vm_break;
                }

                vm_default
                {
                    // halt
                    monitor_log(LOG_INFO, "%.04x: HALTED 0x%.04x", currentPC, instruction);
                    s_chip8.halted = true;
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
                    monitor_log(LOG_DEBUG, "%.04x: LD6(%d) // x = delay timer", currentPC, x);
                    s_chip8.v[x] = s_chip8.delay_timer;
                    vm_break;
                }
                
                vm_case(0x0A)
                {
                    // LD Vx, K
                    monitor_log(LOG_DEBUG, "%.04x: LD3(%d) // x = get_key()", currentPC, x);
                    s_chip8.paused = true;
                    vm_break;
                }
                
                vm_case(0x15)
                {
                    // LD DT, Vx
                    monitor_log(LOG_DEBUG, "%.04x: LD5(%d) // delay timer = x", currentPC, x);
                    s_chip8.delay_timer = s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0x18)
                {
                    // LD ST, Vx
                    monitor_log(LOG_DEBUG, "%.04x: LD4(%d) // sound timer = x", currentPC, x);
                    s_chip8.sound_timer = s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0x1E)
                {
                    // ADD I, Vx
                    monitor_log(LOG_DEBUG, "%.04x: ADD3(%d) // index += x", currentPC, x);
                    s_chip8.index += (uint16_t)s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0x29)
                {
                    // LD F, Vx
                    monitor_log(LOG_DEBUG, "%.04x: LD7(%d) // index = font at x", currentPC, x);
                    s_chip8.index = D0 + NIBBLE(s_chip8.v[x]) * 5;
                    vm_break;
                }
                
                vm_case(0x33)
                {
                    // LD B, Vx
                    monitor_log(LOG_DEBUG, "%.04x: LD8(%d) // Store BCD of x at index", currentPC, x);
                    const uint8_t value = s_chip8.v[x];
                    const uint8_t ones = value % 10;
                    const uint8_t tens = (value % 100) / 10;
                    const uint8_t hundreds = value / 100;
                    
                    if(s_chip8.index >= PROGRAM_START)
                    {
                        s_chip8.ram[s_chip8.index] = hundreds;
                        s_chip8.ram[s_chip8.index + 1] = tens;
                        s_chip8.ram[s_chip8.index + 2] = ones;
                    }
                    else
                    {
                        monitor_log(LOG_WARNING, "LD8 failed: index out of bounds");
                        s_chip8.paused = true;
                    }
                    
                    vm_break;
                }
                
                vm_case(0x55)
                {
                    // LD [I], Vx
                    monitor_log(LOG_DEBUG, "%.04x: LD9(%d) // Store register 0 thru x into index", currentPC, x);
                    for(uint8_t i = 0; i <= x; ++i)
                    {
                        if((s_chip8.index + i) >= PROGRAM_START)
                        {
                            s_chip8.ram[s_chip8.index + i] = s_chip8.v[i];
                        }
                        else
                        {
                            monitor_log(LOG_WARNING, "LD9 failed: index out of bounds");
                            s_chip8.paused = true;
                        }
                    }
                    
                    vm_break;
                }
                
                vm_case(0x65)
                {
                    // LD Vx, [I]
                    monitor_log(LOG_DEBUG, "%.04x: LDA(%d) // load registers 0 thru x with values starting at index", currentPC, x);
                    for(uint8_t i = 0; i <= x; ++i)
                    {
                        s_chip8.v[i] = s_chip8.ram[s_chip8.index + i];
                    }

                    vm_break;
                }

                vm_default
                {
                    // halt
                    monitor_log(LOG_INFO, "%.04x: HALTED 0x%.04x", currentPC, instruction);
                    s_chip8.halted = true;
                    vm_break;
                }
            }

            vm_break;
        }

        vm_default
        {
            // halt
            monitor_log(LOG_INFO, "%.04x: HALTED 0x%.04x", currentPC, instruction);
            s_chip8.halted = true;
            vm_break;
        }
    }
}

static void chip8_shutdown()
{
    printf("Shutdown chip8 emulation\n");
}

static void chip8_load_rom(const char* rom_path)
{
// If you want to write your own test program then uncomment the following #define and update program.h with your Chip8 program.
//#define TEST_PROGRAM
#ifdef TEST_PROGRAM
#include "program.h"
    (void)rom_path;
    for(size_t i = 0; i < sizeof(program) / sizeof(program[0]); ++i)
    {
        const uint16_t instruction = program[i];
        const uint8_t lower = (uint8_t)(instruction & 0xFF);
        const uint8_t upper = (uint8_t)((instruction & 0xFF00) >> 8);
        s_chip8.ram[s_chip8.pc + i * 2] = upper;
        s_chip8.ram[s_chip8.pc + i * 2 + 1] = lower;
    }
#else
    FILE* rom = fopen(rom_path, "rb");
    uint8_t* write_ptr = &s_chip8.ram[s_chip8.pc];
    size_t rom_size = 0;

    if(rom)
    {
        monitor_log(LOG_INFO, "ROM opened");
        uint8_t buffer[256];
        size_t bytes_read = fread(buffer, sizeof(uint8_t), sizeof(buffer), rom);
        
        while(bytes_read > 0)
        {
            if(write_ptr - &s_chip8.ram[s_chip8.pc] >= (ptrdiff_t)(0xFFF - PROGRAM_START))
            {
                monitor_log(LOG_ERROR, "ROM too large to fit in memory");
                break;
            }

            memcpy(write_ptr, buffer, bytes_read);
            write_ptr += bytes_read;
            rom_size += bytes_read;

            if(feof(rom))
            {
                monitor_log(LOG_DEBUG, "EOF reached");
                break;
            }
            
            bytes_read = fread(buffer, sizeof(uint8_t), sizeof(buffer), rom);
        }
        
        fclose(rom);
    }
#endif
}
