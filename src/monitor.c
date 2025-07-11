#include "monitor.h"
#include "renderer.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

const int MONITOR_COLUMNS = 64;
const int MONITOR_ROWS = 32;
static int MONITOR[64];

static void monitor_set_pixel(int x, int y, bool is_on, bool* didCollide);

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

static void monitor_set_pixel(int x, int y, bool is_on, bool* didCollide)
{
    if(!is_on)
    {
        return;
    }

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

    MONITOR[x] ^= (1 << y);
    *didCollide = (MONITOR[x] & (1 << y)) == 0;
}
