#include "renderer.h"
#include "chip8.h"

#include "raylib.h"

#include <string.h>

extern Chip8 s_chip8;

const int32_t SCALE = 10;
const int32_t RASTER_COLUMNS = 64;
const int32_t RASTER_ROWS = 32;
static int32_t RASTER_DISPLAY[64];

static void (*vm_update)();
static void (*vm_shutdown)();

static inline void draw_column(int32_t x)
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
    SetTraceLogLevel(LOG_ALL);
    SetWindowState(FLAG_WINDOW_UNDECORATED);

    memset(RASTER_DISPLAY, 0, sizeof(RASTER_DISPLAY));
}

void renderer_do_update()
{
    bool is_fps_shown = false;

    while(!WindowShouldClose())
    {
        vm_update();

        if (IsKeyPressed(KEY_F1))
        {
            is_fps_shown = !is_fps_shown;
        }

        if (IsKeyPressed(KEY_EQUAL) && s_chip8.speed < 10000000)
        {
            s_chip8.speed *= 10;
        }

        if (IsKeyPressed(KEY_MINUS) && s_chip8.speed > 1)
        {
            s_chip8.speed /= 10;
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

            const char* chip8Info = TextFormat("v0: %.02x  v1: %.02x  v2: %.02x  v3: %.02x  v4: %.02x  v5: %.02x  v6: %.02x  v7: %.02x\n"
                "v8: %.02x  v9: %.02x  va: %.02x  vb: %.02x  vc: %.02x  vd: %.02x  ve: %.02x  vf: %.02x\n"
                "index: %.04x  pc: %.04x  sp: %.02x  delay_timer: %.02x  sound_timer: %.02x\n"
                "speed: %d\n",
                s_chip8.v[0], s_chip8.v[1], s_chip8.v[2], s_chip8.v[3], s_chip8.v[4], s_chip8.v[5], s_chip8.v[6], s_chip8.v[7], 
                s_chip8.v[8], s_chip8.v[9], s_chip8.v[10], s_chip8.v[11], s_chip8.v[12], s_chip8.v[13], s_chip8.v[14], s_chip8.v[15],
                s_chip8.index, s_chip8.pc, s_chip8.sp, s_chip8.delay_timer, s_chip8.sound_timer, s_chip8.speed);
            DrawText(chip8Info, 10, 30, 18, DARKGREEN);
        }

        EndDrawing();
    }
}

void renderer_shutdown()
{
    TraceLog(LOG_INFO, "Shutting down Chip8 VM");
    vm_shutdown();
}

void renderer_blit(int32_t* data)
{
    memcpy(RASTER_DISPLAY, data, sizeof(RASTER_DISPLAY));
}

void renderer_set_update_func(void (*update_func)())
{
    vm_update = update_func;
}

void renderer_set_shutdown_func(void (*shutdown_func)())
{
    vm_shutdown = shutdown_func;
}

void renderer_log(const char* message)
{
    TraceLog(LOG_INFO, message);
}

bool renderer_get_key(uint8_t* outKey)
{
    if(IsKeyPressed(KEY_ONE))
    {
        *outKey = 0x1;
        return true;
    }

    if(IsKeyPressed(KEY_TWO))
    {
        *outKey = 0x2;
        return true;
    }

    if(IsKeyPressed(KEY_THREE))
    {
        *outKey = 0x3;
        return true;
    }

    if(IsKeyPressed(KEY_FOUR))
    {
        *outKey = 0xC;
        return true;
    }

    if(IsKeyPressed(KEY_Q))
    {
        *outKey = 0x4;
        return true;
    }

    if(IsKeyPressed(KEY_W))
    {
        *outKey = 0x5;
        return true;
    }

    if(IsKeyPressed(KEY_E))
    {
        *outKey = 0x6;
        return true;
    }

    if(IsKeyPressed(KEY_R))
    {
        *outKey = 0xD;
        return true;
    }

    if(IsKeyPressed(KEY_A))
    {
        *outKey = 0x7;
        return true;
    }

    if(IsKeyPressed(KEY_S))
    {
        *outKey = 0x8;
        return true;
    }

    if(IsKeyPressed(KEY_D))
    {
        *outKey = 0x9;
        return true;
    }

    if(IsKeyPressed(KEY_F))
    {
        *outKey = 0xE;
        return true;
    }

    if(IsKeyPressed(KEY_Z))
    {
        *outKey = 0xA;
        return true;
    }

    if(IsKeyPressed(KEY_X))
    {
        *outKey = 0x0;
        return true;
    }

    if(IsKeyPressed(KEY_C))
    {
        *outKey = 0xB;
        return true;
    }

    if(IsKeyPressed(KEY_V))
    {
        *outKey = 0xF;
        return true;
    }

    return false;
}

bool renderer_is_key_down(uint8_t key)
{
    if(IsKeyDown(KEY_ONE) && key == 0x1)
    {
        return true;
    }

    if(IsKeyDown(KEY_TWO) && key == 0x2)
    {
        return true;
    }

    if(IsKeyDown(KEY_THREE) && key == 0x3)
    {
        return true;
    }

    if(IsKeyDown(KEY_FOUR) && key == 0xC)
    {
        return true;
    }

    if(IsKeyDown(KEY_Q) && key == 0x4)
    {
        return true;
    }

    if(IsKeyDown(KEY_W) && key == 0x5)
    {
        return true;
    }

    if(IsKeyDown(KEY_E) && key == 0x6)
    {
        return true;
    }

    if(IsKeyDown(KEY_R) && key == 0xD)
    {
        return true;
    }

    if(IsKeyDown(KEY_A) && key == 0x7)
    {
        return true;
    }

    if(IsKeyDown(KEY_S) && key == 0x8)
    {
        return true;
    }

    if(IsKeyDown(KEY_D) && key == 0x9)
    {
        return true;
    }

    if(IsKeyDown(KEY_F) && key == 0xE)
    {
        return true;
    }

    if(IsKeyDown(KEY_Z) && key == 0xA)
    {
        return true;
    }

    if(IsKeyDown(KEY_X) && key == 0x0)
    {
        return true;
    }

    if(IsKeyDown(KEY_C) && key == 0xB)
    {
        return true;
    }

    if(IsKeyDown(KEY_V) && key == 0xF)
    {
        return true;
    }

    return false;
}
