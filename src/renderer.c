#include "renderer.h"

#include "raylib.h"

#include <string.h>

const int SCALE = 10;
const int RASTER_COLUMNS = 64;
const int RASTER_ROWS = 32;
static int RASTER_DISPLAY[64];

static void (*monitor_update)();

static inline void draw_column(int x)
{
    DrawRectangle(x * SCALE, 0, SCALE, SCALE, RASTER_DISPLAY[x] & 1 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 2 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 2 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 4 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 3 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 8 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 4 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 16 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 5 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 32 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 6 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 64 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 7 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 128 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 8 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 256 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 9 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 512 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 10 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 1024 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 11 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & 2048 ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 12 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 12) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 13 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 13) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 14 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 14) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 15 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 15) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 16 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 16) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 17 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 17) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 18 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 18) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 19 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 19) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 20 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 20) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 21 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 21) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 22 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 22) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 23 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 23) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 24 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 24) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 25 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 25) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 26 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 26) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 27 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 27) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 28 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 28) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 29 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 29) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 30 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 30) ? WHITE : BLACK);
    DrawRectangle(x * SCALE, 31 * SCALE, SCALE, SCALE, RASTER_DISPLAY[x] & (1 << 31) ? WHITE : BLACK);
}

void renderer_initialize()
{
    InitWindow(64 * SCALE, 32 * SCALE, "Chip8 Emulator");
    SetTargetFPS(60);

    memset(RASTER_DISPLAY, 0, sizeof(RASTER_DISPLAY));
}

void renderer_do_update()
{
    bool is_fps_shown = false;

    while(!WindowShouldClose())
    {
        monitor_update();

        if (IsKeyPressed(KEY_F1))
        {
            is_fps_shown = !is_fps_shown;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for(int x = 0; x < RASTER_COLUMNS; ++x)
        {
            draw_column(x);
        }

        if(is_fps_shown)
        {
            DrawFPS(10, 10);
        }

        EndDrawing();
    }
}

void renderer_shutdown()
{

}

void renderer_blit(int* data)
{
    memcpy(RASTER_DISPLAY, data, sizeof(RASTER_DISPLAY));
}

void renderer_set_update_func(void (*update_func)())
{
    monitor_update = update_func;
}
