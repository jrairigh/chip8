#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

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
    uint32_t speed;
    bool halted;
    bool paused;
} Chip8;

void chip8_run();

#endif
