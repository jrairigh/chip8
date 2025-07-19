#include "monitor.h"
#include "renderer.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MONITOR_COLUMNS 64
#define MONITOR_ROWS 32

static uint32_t MONITOR[MONITOR_COLUMNS];

static void monitor_set_pixel(uint8_t x, uint8_t y, bool set, bool* did_collide);

void monitor_initialize(const InitFunc init_func, const UpdateFunc update_func, const ShutdownFunc shutdown_func)
{
    renderer_initialize(MONITOR);
    renderer_set_init_func(init_func);
    renderer_set_update_func(update_func);
    renderer_set_shutdown_func(shutdown_func);
    renderer_do_update();
    renderer_shutdown();
}

void monitor_log(const LogLevel level, const char* text, ...)
{
    va_list args;
    va_start(args, text);
    renderer_log(level, text, args);
    va_end(args);
}

void monitor_clear(void)
{
    memset(MONITOR, 0, sizeof(MONITOR));
}

void monitor_draw_sprite(const uint8_t x, const uint8_t y, const uint8_t* sprite, const uint8_t sprite_size_in_bytes, bool* did_collide)
{
    *did_collide = false;
    
    for(uint8_t i = 0; i < 8; ++i)
    {
        for(uint8_t j = 0; j < sprite_size_in_bytes; ++j)
        {
            const uint8_t row = sprite[j];

            const bool set = ((row << i) & 0x80) > 0;
            if(!set)
            {
                continue;
            }

            monitor_set_pixel(x + i, y + j, set, did_collide);
        }
    }
}

static void monitor_set_pixel(uint8_t x, uint8_t y, const bool set, bool* did_collide)
{
    x = x >= MONITOR_COLUMNS ? x % MONITOR_COLUMNS : x;
    y = y >= MONITOR_ROWS ? y % MONITOR_ROWS : y;

    MONITOR[x] ^= (set << y);
    *did_collide = *did_collide || (MONITOR[x] & (1 << y)) == 0;
}

bool monitor_get_key(uint8_t* out_key)
{
    return renderer_get_key(out_key);
}

bool monitor_is_key_down(const uint8_t key)
{
    return renderer_is_key_down(key);
}

void monitor_play_tone(void)
{
    renderer_play_tone();
}

void monitor_stop_tone(void)
{
    renderer_stop_tone();
}
