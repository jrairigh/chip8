#include "monitor.h"
#include "renderer.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MONITOR_COLUMNS 64
#define MONITOR_ROWS 32

static uint32_t MONITOR[MONITOR_COLUMNS];

static inline void monitor_set_pixel(uint8_t x, uint8_t y, bool set, bool* didCollide);

void monitor_initialize(InitFunc init_func, UpdateFunc update_func, ShutdownFunc shutdown_func)
{
    renderer_initialize(MONITOR);
    renderer_set_init_func(init_func);
    renderer_set_update_func(update_func);
    renderer_set_shutdown_func(shutdown_func);
    renderer_do_update();
    renderer_shutdown();
}

void monitor_log(LogLevel level, const char* text, ...)
{
    va_list args;
    va_start(args, text);
    renderer_log((int32_t)level, text, args);
    va_end(args);
}

void monitor_clear()
{
    memset(MONITOR, 0, sizeof(MONITOR));
}

void monitor_draw_sprite(uint8_t x, uint8_t y, uint8_t* sprite, uint8_t sprite_size_in_bytes, bool* didCollide)
{
    *didCollide = false;
    
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < sprite_size_in_bytes; ++j)
        {
            const uint8_t row = sprite[j];

            const bool set = ((row << i) & 0x80) > 0;
            if(!set)
            {
                continue;
            }

            monitor_set_pixel(x + i, y + j, set, didCollide);
        }
    }
}

static inline void monitor_set_pixel(uint8_t x, uint8_t y, bool set, bool* didCollide)
{
    x = x >= MONITOR_COLUMNS ? x % MONITOR_COLUMNS : x;
    y = y >= MONITOR_ROWS ? y % MONITOR_ROWS : y;

    MONITOR[x] ^= (set << y);
    *didCollide = *didCollide || (MONITOR[x] & (1 << y)) == 0;
}

bool monitor_get_key(uint8_t* outKey)
{
    return renderer_get_key(outKey);
}

bool monitor_is_key_down(uint8_t key)
{
    return renderer_is_key_down(key);
}

void monitor_play_tone()
{
    renderer_play_tone();
}

void monitor_stop_tone()
{
    renderer_stop_tone();
}
