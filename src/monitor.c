#include "monitor.h"
#include "renderer.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

const int MONITOR_COLUMNS = 64;
const int MONITOR_ROWS = 32;
static int MONITOR[64];

void monitor_clear()
{
    memset(MONITOR, 0, sizeof(MONITOR));
}

void monitor_paint()
{
    bool didCollide;
    monitor_set_pixel(5 + 64, 10 + 32, &didCollide);
    for(int x = 0; x < MONITOR_COLUMNS; ++x)
    {
        for(int y = 0; y < MONITOR_ROWS; ++y)
        {
            monitor_set_pixel(x, y, &didCollide);
        }
    }
}

void monitor_set_pixel(int x, int y, bool* didCollide)
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

    MONITOR[x] ^= (1 << y);
    *didCollide = (MONITOR[x] & (1 << y)) == 0;
}

void monitor_do_update()
{
    monitor_clear();
    monitor_paint();
    renderer_blit(MONITOR);
}
