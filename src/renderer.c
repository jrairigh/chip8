#include "renderer.h"
#include "chip8.h"

#include "raylib.h"
#include "raymath.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#define MAX_ROMS 82
#define MAX_ROM_NAME_SIZE 64

#ifndef CHIP8_LOGLEVEL
#define CHIP8_LOGLEVEL 0
#endif

extern Chip8 g_chip8;

static const struct KeypadPair
{
    uint8_t Key;
    uint8_t Value;
} KeyBindings[16] = {
    {KEY_ONE, 0x1}, {KEY_TWO, 0x2}, {KEY_THREE, 0x3}, {KEY_FOUR, 0xC},
    {KEY_Q, 0x4}, {KEY_W, 0x5}, {KEY_E, 0x6}, {KEY_R, 0xD},
    {KEY_A, 0x7}, {KEY_S, 0x8}, {KEY_D, 0x9}, {KEY_F, 0xE},
    {KEY_Z, 0xA}, {KEY_X, 0x0}, {KEY_C, 0xB}, {KEY_V, 0xF}
};

static const struct ToneConstants
{
    float AudioFrequency;
    uint32_t SampleRate;
    uint32_t SampleSize;
    uint32_t Channels;
    int32_t MaxSamplesPerUpdate;
} ToneK = {
    .AudioFrequency = 440.0f,
    .SampleRate = 44100,
    .SampleSize = 16,
    .Channels = 1,
    .MaxSamplesPerUpdate = 4096
};

static struct RendererContext
{
    const uint32_t* Monitor;
    const int32_t RasterRows;
    const int32_t RasterColumns;
    const int32_t Scale;
    const float TransitionExtraDelay;
    const float TransitionTimeInSeconds;
    float transition_time;
    float sine_idx;
    char roms[MAX_ROMS][MAX_ROM_NAME_SIZE];
    bool is_info_menu_shown;
    bool step;
    int32_t info_menu_height;
    int32_t selected_rom;
    int32_t old_window_height;
    Texture2D menu_bg_tex2d;
    AudioStream tone;
} s_ctx = {
    .RasterRows = 32,
    .RasterColumns = 64,
    .TransitionExtraDelay = 0.5f,
    .TransitionTimeInSeconds = 1.5f,
    .Monitor = NULL,
    .sine_idx = 0.0f,
    .Scale = 15,
    .tone = {0},
    .is_info_menu_shown = false,
    .step = false,
    .info_menu_height = 0,
    .menu_bg_tex2d = {0},
    .roms = {{0}},
    .selected_rom = 0,
    .transition_time = 0.0f,
    .old_window_height = 0
};

static InitFunc vm_init;
static UpdateFunc vm_update;
static ShutdownFunc vm_shutdown;
static void draw_column(uint32_t x);
static void audio_processor(void *bufferData, uint32_t frames);
static void draw_mini_sprite(int32_t x, int32_t y, int32_t width, int32_t height);
static void draw_stack(int32_t x, int32_t y, int32_t width, int32_t height);
static void update_window(bool is_info_showing);

static void render_menu(void);
static void render_transition(void);
static void render_game(void);
static void (*render_state)(void) = render_menu;

void renderer_initialize(const uint32_t* monitor)
{
    s_ctx.Monitor = monitor;
    InitWindow(s_ctx.RasterColumns * s_ctx.Scale, s_ctx.RasterRows * s_ctx.Scale, "Chip8 Emulator");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_DEBUG + CHIP8_LOGLEVEL);
    
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(ToneK.MaxSamplesPerUpdate);
    s_ctx.tone = LoadAudioStream(ToneK.SampleRate, ToneK.SampleSize, ToneK.Channels);
    SetAudioStreamCallback(s_ctx.tone, audio_processor);

    SetAudioStreamVolume(s_ctx.tone, 1.0f);

    if(IsAudioStreamReady(s_ctx.tone))
    {
        TraceLog(LOG_INFO, "Audio stream is ready");
        PlayAudioStream(s_ctx.tone);
        PauseAudioStream(s_ctx.tone);
    }

    s_ctx.old_window_height = GetScreenHeight();

    s_ctx.menu_bg_tex2d = LoadTexture("../menu_bg_img.png");

    FilePathList roms = LoadDirectoryFiles(".");

    for(uint32_t i = 0; i < roms.count; ++i)
    {
        const char* rom_name = GetFileName(roms.paths[i]);

        if(strlen(rom_name) > MAX_ROM_NAME_SIZE)
        {
            continue;
        }

        strcpy(s_ctx.roms[i], rom_name);
    }

    UnloadDirectoryFiles(roms);
}

void renderer_do_update(void)
{
    while(!WindowShouldClose())
    {
        render_state();
    }
}

void renderer_shutdown(void)
{
    TraceLog(LOG_INFO, "Shutting down Chip8 VM");
    vm_shutdown();
    UnloadTexture(s_ctx.menu_bg_tex2d);
    UnloadAudioStream(s_ctx.tone);
    CloseAudioDevice();
    CloseWindow();
}

void renderer_set_init_func(const InitFunc init_func)
{
    vm_init = init_func;
}

void renderer_set_update_func(const UpdateFunc update_func)
{
    vm_update = update_func;
}

void renderer_set_shutdown_func(const ShutdownFunc shutdown_func)
{
    vm_shutdown = shutdown_func;
}

void renderer_log(const int32_t log_level, const char* message, const va_list args)
{
    TraceLogV(LOG_DEBUG + log_level, message, args);
}

void renderer_play_tone(void)
{
    if(IsAudioStreamPlaying(s_ctx.tone))
    {
        return;
    }

    ResumeAudioStream(s_ctx.tone);
    TraceLog(LOG_INFO, "Playing tone");
}

void renderer_stop_tone(void)
{
    if(!IsAudioStreamPlaying(s_ctx.tone))
    {
        return;
    }

    PauseAudioStream(s_ctx.tone);
    TraceLog(LOG_INFO, "Stopping tone");
}

bool renderer_get_key(uint8_t* out_key)
{
    for(size_t i = 0; i < sizeof(KeyBindings) / sizeof(KeyBindings[0]); ++i)
    {
        if(IsKeyPressed(KeyBindings[i].Key))
        {
            *out_key = KeyBindings[i].Value;
            return true;
        }
    }

    return false;
}

bool renderer_is_key_down(const uint8_t key)
{
    for(size_t i = 0; i < sizeof(KeyBindings) / sizeof(KeyBindings[0]); ++i)
    {
        if(KeyBindings[i].Value == key && IsKeyDown(KeyBindings[i].Key))
        {
            return true;
        }
    }

    return false;
}

static void draw_column(const uint32_t x)
{
    assert((int32_t)x < s_ctx.RasterColumns);
    for(int32_t i = 0; i < s_ctx.RasterRows; ++i)
    {
        const Color color = s_ctx.Monitor[x] & (1 <<  i) ? WHITE : BLACK;
        DrawRectangle((int32_t)x * s_ctx.Scale,  i * s_ctx.Scale + s_ctx.info_menu_height, s_ctx.Scale, s_ctx.Scale, color);
    }
}

void audio_processor(void *buffer, const uint32_t frames)
{
    const float incr = ToneK.AudioFrequency / (float)ToneK.SampleRate;
    int16_t *d = buffer;

    for (uint32_t i = 0; i < frames; ++i)
    {
        d[i] = (int16_t)(32000.0f * sinf(2.0f * PI * s_ctx.sine_idx));
        s_ctx.sine_idx += incr;

        if (s_ctx.sine_idx > 1.0f)
        {
            s_ctx.sine_idx -= 1.0f;
        }
    }
}

static void render_menu(void)
{
    const Vector2 mousePos = GetMousePosition();
    const Rectangle playButtonBounds = (Rectangle){419, 411, 128, 53};
    const Rectangle cycleRightButtonBounds = (Rectangle){834, 340, 41, 36};
    const Rectangle cycleLeftButtonBounds = (Rectangle){84, 340, 41, 36};

    BeginDrawing();
    DrawTexture(s_ctx.menu_bg_tex2d, 0, 0, WHITE);
    DrawText(s_ctx.roms[s_ctx.selected_rom], (int32_t)(cycleLeftButtonBounds.x + cycleLeftButtonBounds.width + 10), (int32_t)cycleLeftButtonBounds.y, 30, WHITE);

    const bool isOverPlayButton = CheckCollisionPointRec(mousePos, playButtonBounds);
    const bool isOverCycleRightButton = CheckCollisionPointRec(mousePos, cycleRightButtonBounds);
    const bool isOverCycleLeftButton = CheckCollisionPointRec(mousePos, cycleLeftButtonBounds);

    DrawText("PLAY", (int32_t)playButtonBounds.x, (int32_t)playButtonBounds.y, 48, isOverPlayButton ? GREEN : WHITE);
    DrawRectangle((int32_t)cycleLeftButtonBounds.x, (int32_t)cycleLeftButtonBounds.y, (int32_t)cycleLeftButtonBounds.width, (int32_t)cycleLeftButtonBounds.height, isOverCycleLeftButton ? (Color){0, 255, 0, 66} : (Color){0, 255, 0, 33});
    DrawRectangle((int32_t)cycleRightButtonBounds.x, (int32_t)cycleRightButtonBounds.y, (int32_t)cycleRightButtonBounds.width, (int32_t)cycleRightButtonBounds.height, isOverCycleRightButton ? (Color){0, 255, 0, 66} : (Color){0, 255, 0, 33});
    EndDrawing();

    if(isOverPlayButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        render_state = render_transition;
        s_ctx.transition_time = (float)GetTime() + s_ctx.TransitionTimeInSeconds;
        vm_init(s_ctx.roms[s_ctx.selected_rom]);
    }
    else if(isOverCycleRightButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        s_ctx.selected_rom = (s_ctx.selected_rom + 1) % MAX_ROMS;
    }
    else if(isOverCycleLeftButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        s_ctx.selected_rom = (s_ctx.selected_rom + MAX_ROMS - 1) % MAX_ROMS;
    }

    if(IsAudioStreamPlaying(s_ctx.tone))
    {
        PauseAudioStream(s_ctx.tone);
    }
}

static void render_transition(void)
{
    if(GetTime() >= s_ctx.transition_time - s_ctx.TransitionExtraDelay)
    {
        render_state = render_game;
        return;
    }

    const float t = Clamp(1.0f - ((s_ctx.transition_time - s_ctx.TransitionExtraDelay - (float)GetTime()) / (s_ctx.TransitionTimeInSeconds - s_ctx.TransitionExtraDelay)), 0.0f, 1.0f);
    const int32_t width = (int32_t)floorf(t * (float)s_ctx.RasterColumns * (float)s_ctx.Scale);
    const Rectangle playButtonBounds = (Rectangle){419, 411, 128, 53};
    const Rectangle cycleLeftButtonBounds = (Rectangle){84, 340, 41, 36};

    BeginDrawing();
    DrawTexture(s_ctx.menu_bg_tex2d, 0, 0, WHITE);
    DrawText(s_ctx.roms[s_ctx.selected_rom], (int32_t)(cycleLeftButtonBounds.x + cycleLeftButtonBounds.width + 10), (int32_t)cycleLeftButtonBounds.y, 30, WHITE);
    DrawText("PLAY", (int32_t)playButtonBounds.x, (int32_t)playButtonBounds.y, 48, WHITE);
    DrawRectangle(0, 0, width, s_ctx.RasterRows * s_ctx.Scale, BLACK);
    EndDrawing();
}

static void render_game(void)
{
    if(s_ctx.is_info_menu_shown && IsKeyPressed(KEY_F10))
    {
        s_ctx.step = true;
    }
    else if(!s_ctx.is_info_menu_shown)
    {
        s_ctx.step = false;
    }

    if(s_ctx.step && IsKeyPressed(KEY_F10))
    {
        vm_update();
    }
    else if(!s_ctx.step)
    {
        vm_update();
    }

    if (IsKeyPressed(KEY_F2))
    {
        render_state = render_menu;
        s_ctx.is_info_menu_shown = false;
        update_window(false);
        return;
    }

    if (IsKeyPressed(KEY_F1))
    {
        s_ctx.is_info_menu_shown = !s_ctx.is_info_menu_shown;
        update_window(s_ctx.is_info_menu_shown);
    }

    if (IsKeyPressed(KEY_EQUAL) && g_chip8.speed < 10000000)
    {
        g_chip8.speed *= 10;
    }

    if (IsKeyPressed(KEY_MINUS) && g_chip8.speed > 1)
    {
        g_chip8.speed /= 10;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for(int x = 0; x < s_ctx.RasterColumns; ++x)
    {
        draw_column(x);
    }

    if(s_ctx.is_info_menu_shown)
    {
        const char* chip8Info = TextFormat("v0: %.02x  v1: %.02x  v2: %.02x  v3: %.02x  v4: %.02x  v5: %.02x  v6: %.02x  v7: %.02x\n\n"
            "v8: %.02x  v9: %.02x  va: %.02x  vb: %.02x  vc: %.02x  vd: %.02x  ve: %.02x  vf: %.02x\n\n"
            "index: %.04x  pc: %.04x  sp: %.02x  delay_timer: %.02x  sound_timer: %.02x\n\n"
            "speed: %d\n",
            g_chip8.v[0], g_chip8.v[1], g_chip8.v[2], g_chip8.v[3], g_chip8.v[4], g_chip8.v[5], g_chip8.v[6], g_chip8.v[7], 
            g_chip8.v[8], g_chip8.v[9], g_chip8.v[10], g_chip8.v[11], g_chip8.v[12], g_chip8.v[13], g_chip8.v[14], g_chip8.v[15],
            g_chip8.index, g_chip8.pc, g_chip8.sp, g_chip8.delay_timer, g_chip8.sound_timer, g_chip8.speed);
        DrawRectangle(0, 0, 650, 150, DARKGRAY);
        DrawText(chip8Info, 10, 36, 20, GREEN);
        draw_stack(0, 150, 650, 60);
        draw_mini_sprite(650, 0, 240, 210);
        DrawFPS(10, 10);
    }

    if(GetTime() < s_ctx.transition_time)
    {
        DrawRectangle(0, 0, s_ctx.RasterColumns * s_ctx.Scale, s_ctx.RasterRows * s_ctx.Scale, BLACK);
    }

    EndDrawing();
}

static void draw_mini_sprite(const int32_t x, const int32_t y, const int32_t width, const int32_t height)
{
    assert(width % 8 == 0 && height % 15 == 0);
    const int32_t px_width = width / 8;
    const int32_t px_height = height / 15;

    DrawRectangle(x, y, width, height, BLACK);
    
    for(int32_t i = 0; i < 15; ++i)
    {
        const uint8_t row = g_chip8.ram[g_chip8.index + i];
        DrawRectangle(0 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x80) ? WHITE : BLACK);
        DrawRectangle(1 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x40) ? WHITE : BLACK);
        DrawRectangle(2 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x20) ? WHITE : BLACK);
        DrawRectangle(3 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x10) ? WHITE : BLACK);
        DrawRectangle(4 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x08) ? WHITE : BLACK);
        DrawRectangle(5 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x04) ? WHITE : BLACK);
        DrawRectangle(6 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x02) ? WHITE : BLACK);
        DrawRectangle(7 * px_width + x, (i + y) * px_height, px_width, px_height, (row & 0x01) ? WHITE : BLACK);
    }

    // draw a grid
    const Color line_color = (Color){128, 128, 128, 128};
    
    for(int32_t i = 1; i <= 14; ++i)
    {
        DrawLine(x, y + i * px_height, x + width, y + i * px_height, line_color);
    }

    for(int32_t i = 1; i <= 7; ++i)
    {
        DrawLine(x + i * px_width, y, x + i * px_width, height, line_color);
    }

    DrawRectangleLines(x, y, width, height, DARKGRAY);
}

static void draw_stack(const int32_t x, const int32_t y, const int32_t width, const int32_t height)
{
    const int32_t px_width = width / 16;
    DrawRectangleLines(x, y, width, height, BLACK);

    const Color line_color = (Color){128, 128, 128, 128};

    for(int32_t i = 1; i <= 15; ++i)
    {
        DrawLine(x + i * px_width, y, x + i * px_width, y + height, line_color);
    }

    for(uint8_t i = 0; i < g_chip8.sp; ++i)
    {
        DrawRectangle(x + i * px_width, y, px_width, height, RED);
        DrawText(TextFormat("%.04x", g_chip8.stack[i]), x + i * px_width + 2, y + 6, 16, WHITE);
    }

    DrawRectangleLines(x, y, width, height, DARKGRAY);
}

static void update_window(const bool is_info_showing)
{
    const int32_t window_width = GetScreenWidth();
    int32_t window_height;

    if(is_info_showing)
    {
        s_ctx.info_menu_height = 210;
        window_height = s_ctx.old_window_height + s_ctx.info_menu_height;
    }
    else
    {
        s_ctx.info_menu_height = 0;
        window_height = s_ctx.old_window_height;
    }

    SetWindowSize(window_width, window_height);
}
