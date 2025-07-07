#include "chip8.h"
#include "monitor.h"
#include "renderer.h"

#include <stdbool.h>
#include <stdint.h>

static uint8_t s_memory[4096] = {0};
static int s_registers[16] = {0};
static int s_stack[16] = {0};
int index = 0;
int pc = 0x200;
int sp = 0;
int delay_timer = 0;
int sound_timer = 0;
bool paused = false;
int speed = 10;

#define vm_case(op) case op:
#define vm_break break;

static void vm_run()
{
    const uint16_t instruction = ((uint16_t*)s_memory)[pc];
    const uint8_t opcode = (instruction & 0xF000) >> 12;

    switch(opcode)
    {
        vm_case(0)
        {
            vm_break;
        }

        vm_case(1)
        {
            vm_break;
        }

        vm_case(2)
        {
            vm_break;
        }
    }

    monitor_do_update();
}

void chip8_run()
{
    renderer_initialize();
    renderer_set_update_func(vm_run);
    renderer_do_update();
    renderer_shutdown();
}
