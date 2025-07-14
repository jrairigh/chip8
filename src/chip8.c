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

static bool is_big_endian();
static void chip8_load_rom(const char* rom_path);
void chip8_initialize();
void chip8_cycle();
void chip8_vm_run();
void chip8_shutdown();

#ifndef RUN_TESTS
void chip8_run(const char* rom)
{
    chip8_initialize();
    chip8_load_rom(rom);
    renderer_initialize();
    renderer_set_update_func(chip8_cycle);
    renderer_set_shutdown_func(chip8_shutdown);
    renderer_do_update();
    renderer_shutdown();
}
#else
void chip8_run_tests();
void chip8_run(const char* rom)
{
    (void)rom;
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
}

void chip8_cycle()
{
    for(int32_t i = 0; i < s_chip8.speed; ++i)
    {
        chip8_vm_run();
    }

    if(s_chip8.delay_timer > 0)
    {
        --s_chip8.delay_timer;
    }

    if(s_chip8.sound_timer > 0)
    {
        --s_chip8.sound_timer;
    }
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
                    renderer_log("CLS");
                    monitor_clear();
                    vm_break;
                }
                
                vm_case(0xEE)
                {
                    // RET
                    renderer_log("RET");
                    s_chip8.pc = s_chip8.stack[s_chip8.sp];
                    s_chip8.sp--;
                    vm_break;
                }

                vm_default
                {
                    // halt
                    //renderer_log("HALTED");
                    pc_inc = 0;
                    vm_break;
                }
            }

            vm_break;
        }

        vm_case(0x1000)
        {
            // JP addr
            //renderer_log("JP");
            pc_inc = 0;
            s_chip8.pc = ADDR(instruction);
            vm_break;
        }

        vm_case(0x2000)
        {
            // CALL addr
            renderer_log("CALL");
            pc_inc = 0;
            s_chip8.sp++;
            s_chip8.stack[s_chip8.sp] = s_chip8.pc;
            s_chip8.pc = ADDR(instruction);
            vm_break;
        }

        vm_case(0x3000)
        {
            // SE Vx, byte
            renderer_log("SE1");
            s_chip8.pc += (uint16_t)(s_chip8.v[x] == BYTE(instruction)) << 1;
            vm_break;
        }
        
        vm_case(0x4000)
        {
            // SNE Vx, byte
            renderer_log("SNE1");
            s_chip8.pc += (uint16_t)(s_chip8.v[x] != BYTE(instruction)) << 1;
            vm_break;
        }
        
        vm_case(0x5000)
        {
            // SE Vx, Vy
            renderer_log("SE2");
            s_chip8.pc += (uint16_t)(s_chip8.v[x] == s_chip8.v[y]) << 1;
            vm_break;
        }
        
        vm_case(0x6000)
        {
            // LD Vx, byte
            renderer_log("LD1");
            s_chip8.v[x] = BYTE(instruction);
            vm_break;
        }
        
        vm_case(0x7000)
        {
            // ADD Vx, byte
            renderer_log("ADD1");
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
                    renderer_log("LD2");
                    s_chip8.v[x] = s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x1)
                {
                    // OR Vx, Vy
                    renderer_log("OR");
                    s_chip8.v[x] |= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x2)
                {
                    // AND Vx, Vy
                    renderer_log("OR");
                    s_chip8.v[x] &= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x3)
                {
                    // XOR Vx, Vy
                    renderer_log("XOR");
                    s_chip8.v[x] ^= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x4)
                {
                    // ADD Vx, Vy
                    renderer_log("ADD2");
                    const uint8_t max_value = 0xFF - s_chip8.v[x];
                    s_chip8.v[0xF] = max_value < s_chip8.v[y]; // Set carry flag
                    s_chip8.v[x] += s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x5)
                {
                    // SUB Vx, Vy
                    renderer_log("SUB");
                    s_chip8.v[0xF] = s_chip8.v[x] > s_chip8.v[y]; // Set borrow flag
                    s_chip8.v[x] -= s_chip8.v[y];
                    vm_break;
                }
                
                vm_case(0x6)
                {
                    // SHR Vx {, Vy}
                    renderer_log("SHR");
                    s_chip8.v[0xF] = s_chip8.v[x] & 0x1; // Set carry flag
                    s_chip8.v[x] >>= 1;
                    vm_break;
                }
                
                vm_case(0x7)
                {
                    // SUBN Vx, Vy
                    renderer_log("SUBN");
                    s_chip8.v[0xF] = s_chip8.v[y] > s_chip8.v[x]; // Set borrow flag
                    s_chip8.v[x] = s_chip8.v[y] - s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0xE)
                {
                    // SHL Vx {, Vy}
                    renderer_log("SHL");
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
            renderer_log("SNE2");
            s_chip8.pc += (uint16_t)(s_chip8.v[x] != s_chip8.v[y]) << 1;
            vm_break;
        }
        
        vm_case(0xA000)
        {
            // LD I, addr
            renderer_log("LDB");
            s_chip8.index = ADDR(instruction);
            vm_break;
        }
        
        vm_case(0xB000)
        {
            // JP V0, addr
            renderer_log("JP1");
            pc_inc = 0;
            s_chip8.pc = ADDR(instruction) + s_chip8.v[0];
            vm_break;
        }
        
        vm_case(0xC000)
        {
            // RND Vx, byte
            renderer_log("RND");
            s_chip8.v[x] = (rand() % 256) & BYTE(instruction);
            vm_break;
        }
        
        vm_case(0xD000)
        {
            // DRW Vx, Vy, nibble
            renderer_log("DRW");
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
                    renderer_log("SKP");
                    s_chip8.pc += monitor_is_key_down(s_chip8.v[x]) << 1;
                    vm_break;
                }
                
                vm_case(0xA1)
                {
                    // SKNP Vx
                    renderer_log("SKNP");
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
                    renderer_log("LD6");
                    s_chip8.v[x] = s_chip8.delay_timer;
                    vm_break;
                }
                
                vm_case(0x0A)
                {
                    // LD Vx, K
                    renderer_log("LD3");
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
                    renderer_log("LD5");
                    s_chip8.delay_timer = s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0x18)
                {
                    // LD ST, Vx
                    renderer_log("LD4");
                    s_chip8.sound_timer = s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0x1E)
                {
                    // ADD I, Vx
                    renderer_log("ADD3");
                    s_chip8.index += (uint16_t)s_chip8.v[x];
                    vm_break;
                }
                
                vm_case(0x29)
                {
                    // LD F, Vx
                    renderer_log("LD7");
                    s_chip8.index = D0 + NIBBLE(s_chip8.v[x]) * 5;
                    vm_break;
                }
                
                vm_case(0x33)
                {
                    // LD B, Vx
                    renderer_log("LD8");
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
                    renderer_log("LD9");
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
                    renderer_log("LDA");
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
}

void chip8_shutdown()
{
    printf("Shutdown chip8 emulation\n");
}

static void chip8_load_rom(const char* rom_path)
{
#define TEST_PROGRAM
#ifdef TEST_PROGRAM
#include "program.h"
    memcpy(&s_chip8.ram[s_chip8.pc], program, sizeof(program));
#else
    FILE* rom = fopen(rom_path, "rb");
    uint8_t* write_ptr = &s_chip8.ram[s_chip8.pc];
    size_t rom_size = 0;

    printf("Opening ROM\n");
    if(rom)
    {
        printf("Loading ROM\n");
        uint8_t buffer[256];
        size_t bytes_read = fread(buffer, sizeof(uint8_t), sizeof(buffer), rom);
        
        while(bytes_read > 0)
        {
            if(write_ptr - &s_chip8.ram[s_chip8.pc] >= (ptrdiff_t)(0xFFF - 0x200))
            {
                fprintf(stderr, "ROM too large to fit in memory\n");
                break;
            }

            memcpy(write_ptr, buffer, bytes_read);
            write_ptr += bytes_read;
            rom_size += bytes_read;

            if(feof(rom))
            {
                printf("EOF reached\n");
                break;
            }
            
            bytes_read = fread(buffer, sizeof(uint8_t), sizeof(buffer), rom);
        }
        
        fclose(rom);
        
        //printf("Writing ROM copy\n");
        //FILE* out = fopen("ROM_COPY", "wb");
        //fwrite(&s_chip8.ram[s_chip8.pc], sizeof(uint8_t), rom_size, out);
        //fclose(out);
    }
    
    if(is_big_endian())
    {
        printf("Big endian architecture\n");
    }
    else
    {
        printf("Little endian architecture\n");
        assert(rom_size % 2 == 0);
        for (size_t i = 0; i + 1 < rom_size; i += 2) 
        {
            const uint8_t tmp = s_chip8.ram[s_chip8.pc + i];
            s_chip8.ram[s_chip8.pc + i] = s_chip8.ram[s_chip8.pc + i + 1];
            s_chip8.ram[s_chip8.pc + i + 1] = tmp;
        }
    }
#endif
}

static bool is_big_endian()
{
    const uint16_t value = 0x0001;
    const uint8_t* bytes = (const uint8_t*)&value;
    return bytes[0] == 0 && bytes[1] == 1;
}
