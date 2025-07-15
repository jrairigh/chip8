#include "renderer.h"
#include "chip8.h"

#include "raylib.h"

#include <math.h>
#include <string.h>

#define RASTER_COLUMNS 64
#define RASTER_ROWS 32

extern Chip8 s_chip8;

int32_t scale_x = 15;
int32_t scale_y = 15;
static int32_t RASTER_DISPLAY[RASTER_COLUMNS];
static AudioStream g_tone;

static void (*vm_update)();
static void (*vm_shutdown)();
static inline void draw_column(int32_t x);
static void audio_processor(void *bufferData, unsigned int frames);

void renderer_initialize()
{
    InitWindow(RASTER_COLUMNS * scale_x, RASTER_ROWS * scale_y, "Chip8 Emulator");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_ALL);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    
    InitAudioDevice();
    g_tone = LoadAudioStream(44100, 16, 1);
    int16_t sin_wave[44100];
    const size_t sin_wave_size = sizeof(sin_wave) / sizeof(sin_wave[0]);
    for (int i = 0; i < sin_wave_size; ++i)
    {
        sin_wave[i] = (int16_t)floor((32767.0 * sin(((double)i / (double)sin_wave_size) * 2.0 * PI)));
    }

    UpdateAudioStream(g_tone, sin_wave, 10);
    SetAudioStreamVolume(g_tone, 1.0f);
    AttachAudioStreamProcessor(g_tone, audio_processor);

    if(IsAudioStreamReady(g_tone))
    {
        TraceLog(LOG_INFO, "Audio stream is ready");
        PlayAudioStream(g_tone);
        PauseAudioStream(g_tone);
    }

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

        if (IsWindowResized())
        {
            scale_x = GetScreenWidth() / RASTER_COLUMNS;
            scale_y = GetScreenHeight() / RASTER_ROWS;
        }

        if (!IsAudioStreamProcessed(g_tone))
        {
            TraceLog(LOG_INFO, "Audio stream needs refill");
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
    DetachAudioStreamProcessor(g_tone, audio_processor);
    UnloadAudioStream(g_tone);
    CloseAudioDevice();
    CloseWindow();
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

void renderer_play_tone()
{
    if(IsAudioStreamPlaying(g_tone))
    {
        return;
    }

    ResumeAudioStream(g_tone);
    TraceLog(LOG_INFO, "Playing tone");
}

void renderer_stop_tone()
{
    if(!IsAudioStreamPlaying(g_tone))
    {
        return;
    }
    
    PauseAudioStream(g_tone);
    TraceLog(LOG_INFO, "Stopping tone");
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

static inline void draw_column(int32_t x)
{
    DrawRectangle(x * scale_x,  0 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  0) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  1 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  1) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  2 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  2) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  3 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  3) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  4 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  4) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  5 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  5) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  6 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  6) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  7 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  7) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  8 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  8) ? WHITE : BLACK);
    DrawRectangle(x * scale_x,  9 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 <<  9) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 10 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 10) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 11 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 11) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 12 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 12) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 13 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 13) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 14 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 14) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 15 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 15) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 16 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 16) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 17 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 17) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 18 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 18) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 19 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 19) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 20 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 20) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 21 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 21) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 22 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 22) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 23 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 23) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 24 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 24) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 25 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 25) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 26 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 26) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 27 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 27) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 28 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 28) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 29 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 29) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 30 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 30) ? WHITE : BLACK);
    DrawRectangle(x * scale_x, 31 * scale_y, scale_x, scale_y, RASTER_DISPLAY[x] & (1 << 31) ? WHITE : BLACK);
}

void audio_processor(void *bufferData, unsigned int frames)
{
    TraceLog(LOG_INFO, "Audio processor called with %u frames", frames);
}
