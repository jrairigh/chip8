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
static int32_t MONITOR[MONITOR_COLUMNS];

static void monitor_set_pixel(int32_t x, int32_t y, int32_t set, bool* didCollide);

void monitor_initialize(void (*init_func)(const char*), void (*update_func)(), void (*shutdown_func)())
{
    renderer_initialize();
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
    renderer_log((int)level, text, args);
    va_end(args);
}

void monitor_clear()
{
    memset(MONITOR, 0, sizeof(MONITOR));
}

void monitor_draw_sprite(int32_t x, int32_t y, uint8_t* sprite, uint32_t sprite_size_in_bytes, bool* didCollide)
{
    for(int i = 0; i < sprite_size_in_bytes; ++i)
    {
        const uint8_t row = sprite[i];
        monitor_set_pixel(x + 0, y + i, (row & 0x80) >> 7, didCollide);
        monitor_set_pixel(x + 1, y + i, (row & 0x40) >> 6, didCollide);
        monitor_set_pixel(x + 2, y + i, (row & 0x20) >> 5, didCollide);
        monitor_set_pixel(x + 3, y + i, (row & 0x10) >> 4, didCollide);
        monitor_set_pixel(x + 4, y + i, (row & 0x08) >> 3, didCollide);
        monitor_set_pixel(x + 5, y + i, (row & 0x04) >> 2, didCollide);
        monitor_set_pixel(x + 6, y + i, (row & 0x02) >> 1, didCollide);
        monitor_set_pixel(x + 7, y + i, (row & 0x01) >> 0, didCollide);
    }
    
    renderer_blit(MONITOR);
}

static void monitor_set_pixel(int32_t x, int32_t y, int32_t set, bool* didCollide)
{
    if(x < 0)
    {
        x += MONITOR_COLUMNS;
    }
    else if(x >= MONITOR_COLUMNS)
    {
        x -= MONITOR_COLUMNS;
    }

    if(y < 0)
    {
        y += MONITOR_ROWS;
    }
    else if(y >= MONITOR_ROWS)
    {
        y -= MONITOR_ROWS;
    }

    MONITOR[x] ^= (set << y);
    *didCollide = (MONITOR[x] & (1 << y)) == 0;
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
