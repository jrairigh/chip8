#include "renderer.h"
#include "chip8.h"

#include "raylib.h"
#include "raymath.h"

#include <math.h>
#include <string.h>

#define RASTER_COLUMNS 64
#define RASTER_ROWS 32
#define MAX_SAMPLES 512
#define MAX_SAMPLES_PER_UPDATE 4096
#define MAX_ROMS 82
#define MAX_ROM_NAME_SIZE 64
#define TRANSITION_EXTRA_DELAY 0.5f
#define TRANSITION_TIME_IN_SECONDS (1 + TRANSITION_EXTRA_DELAY)

#ifndef CHIP8_LOGLEVEL
#define CHIP8_LOGLEVEL 0
#endif

#ifndef CHIP8_STEP
#define CHIP8_STEP 0
#endif

struct KeypadPair
{
    uint8_t key;
    uint8_t value;
};

static const struct KeypadPair s_keypadBindings[16] = {
    {KEY_ONE, 0x1}, {KEY_TWO, 0x2}, {KEY_THREE, 0x3}, {KEY_FOUR, 0xC},
    {KEY_Q, 0x4}, {KEY_W, 0x5}, {KEY_E, 0x6}, {KEY_R, 0xD},
    {KEY_A, 0x7}, {KEY_S, 0x8}, {KEY_D, 0x9}, {KEY_F, 0xE},
    {KEY_Z, 0xA}, {KEY_X, 0x0}, {KEY_C, 0xB}, {KEY_V, 0xF}
};

extern Chip8 s_chip8;

const float AudioFrequency = 440.0f;
const uint32_t SampleRate = 44100;
const uint32_t SampleSize = 16;
const uint32_t Channels = 1;
float sineIdx = 0.0f;
int32_t scale_x = 15;
int32_t scale_y = 15;
static int32_t RASTER_DISPLAY[RASTER_COLUMNS];
static AudioStream g_tone;
static bool is_fps_shown = false;
static Texture2D s_menu_bg_tex2d;
static char s_roms[MAX_ROMS][MAX_ROM_NAME_SIZE];
static int s_selected_rom = 0;
static float s_transition_time = 0.0f;

static void (*vm_init)(const char*);
static void (*vm_update)();
static void (*vm_shutdown)();
static inline void draw_column(int32_t x);
static void audio_processor(void *bufferData, uint32_t frames);

static void render_menu();
static void render_transition();
static void render_game();
static void (*render_state)() = render_menu;

void renderer_initialize()
{
    InitWindow(RASTER_COLUMNS * scale_x, RASTER_ROWS * scale_y, "Chip8 Emulator");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_DEBUG + CHIP8_LOGLEVEL);
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
    g_tone = LoadAudioStream(SampleRate, SampleSize, Channels);
    SetAudioStreamCallback(g_tone, audio_processor);

    SetAudioStreamVolume(g_tone, 1.0f);

    if(IsAudioStreamReady(g_tone))
    {
        TraceLog(LOG_INFO, "Audio stream is ready");
        PlayAudioStream(g_tone);
        PauseAudioStream(g_tone);
    }

    s_menu_bg_tex2d = LoadTexture("../menu_bg_img.png");

    FilePathList roms = LoadDirectoryFiles(".");

    for(uint32_t i = 0; i < roms.count; ++i)
    {
        const char* rom_name = GetFileName(roms.paths[i]);

        if(strlen(rom_name) > MAX_ROM_NAME_SIZE)
        {
            continue;
        }

        strcpy(s_roms[i], rom_name);
    }

    UnloadDirectoryFiles(roms);
}

void renderer_do_update()
{
    while(!WindowShouldClose())
    {
        render_state();
    }
}

void renderer_shutdown()
{
    TraceLog(LOG_INFO, "Shutting down Chip8 VM");
    vm_shutdown();
    UnloadTexture(s_menu_bg_tex2d);
    UnloadAudioStream(g_tone);
    CloseAudioDevice();
    CloseWindow();
}

void renderer_blit(int32_t* data)
{
    memcpy(RASTER_DISPLAY, data, sizeof(RASTER_DISPLAY));
}

void renderer_set_init_func(void (*init_func)(const char*))
{
    vm_init = init_func;
}

void renderer_set_update_func(void (*update_func)())
{
    vm_update = update_func;
}

void renderer_set_shutdown_func(void (*shutdown_func)())
{
    vm_shutdown = shutdown_func;
}

void renderer_log(int logLevel, const char* message, va_list args)
{
    TraceLogV(LOG_DEBUG + logLevel, message, args);
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
    for(size_t i = 0; i < sizeof(s_keypadBindings) / sizeof(s_keypadBindings[0]); ++i)
    {
        if(IsKeyPressed(s_keypadBindings[i].key))
        {
            *outKey = s_keypadBindings[i].value;
            return true;
        }
    }

    return false;
}

bool renderer_is_key_down(uint8_t key)
{
    for(size_t i = 0; i < sizeof(s_keypadBindings) / sizeof(s_keypadBindings[0]); ++i)
    {
        if(s_keypadBindings[i].value == key && IsKeyDown(s_keypadBindings[i].key))
        {
            return true;
        }
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

void audio_processor(void *buffer, uint32_t frames)
{
    const float incr = AudioFrequency/ (float)SampleRate;
    int16_t *d = (int16_t *)buffer;

    for (uint32_t i = 0; i < frames; i++)
    {
        d[i] = (int16_t)(32000.0f*sinf(2*PI*sineIdx));
        sineIdx += incr;
        if (sineIdx > 1.0f)
        {
            sineIdx -= 1.0f;
        }
    }
}

static void render_menu()
{
    Vector2 mousePos = GetMousePosition();
    Rectangle playButtonBounds = (Rectangle){419, 411, 128, 53};
    Rectangle cycleRightButtonBounds = (Rectangle){834, 340, 41, 36};
    Rectangle cycleLeftButtonBounds = (Rectangle){84, 340, 41, 36};

    BeginDrawing();
    DrawTexture(s_menu_bg_tex2d, 0, 0, WHITE);
    DrawText(s_roms[s_selected_rom], cycleLeftButtonBounds.x + cycleLeftButtonBounds.width + 10, cycleLeftButtonBounds.y, 30, WHITE);
    
    const bool isOverPlayButton = CheckCollisionPointRec(mousePos, playButtonBounds);
    const bool isOverCycleRightButton = CheckCollisionPointRec(mousePos, cycleRightButtonBounds);
    const bool isOverCycleLeftButton = CheckCollisionPointRec(mousePos, cycleLeftButtonBounds);

    DrawText("PLAY", playButtonBounds.x, playButtonBounds.y, 48, isOverPlayButton ? GREEN : WHITE);
    DrawRectangle(cycleLeftButtonBounds.x, cycleLeftButtonBounds.y, cycleLeftButtonBounds.width, cycleLeftButtonBounds.height, isOverCycleLeftButton ? (Color){0, 255, 0, 66} : (Color){0, 255, 0, 33});
    DrawRectangle(cycleRightButtonBounds.x, cycleRightButtonBounds.y, cycleRightButtonBounds.width, cycleRightButtonBounds.height, isOverCycleRightButton ? (Color){0, 255, 0, 66} : (Color){0, 255, 0, 33});
    EndDrawing();

    if(isOverPlayButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        render_state = render_transition;
        s_transition_time = GetTime() + TRANSITION_TIME_IN_SECONDS;
        vm_init(s_roms[s_selected_rom]);
    }
    else if(isOverCycleRightButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        s_selected_rom = (s_selected_rom + 1) % MAX_ROMS;
    }
    else if(isOverCycleLeftButton && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        s_selected_rom = (s_selected_rom + MAX_ROMS - 1) % MAX_ROMS;
    }

    if(IsAudioStreamPlaying(g_tone))
    {
        PauseAudioStream(g_tone);
    }
}

static void render_transition()
{
    if(GetTime() >= s_transition_time - TRANSITION_EXTRA_DELAY)
    {
        render_state = render_game;
        return;
    }

    const float t = Clamp(1.0f - ((s_transition_time - TRANSITION_EXTRA_DELAY - (float)GetTime()) / (TRANSITION_TIME_IN_SECONDS - TRANSITION_EXTRA_DELAY)), 0.0f, 1.0f);
    int32_t width = (int32_t)floor(t * RASTER_COLUMNS * scale_x);
    Rectangle playButtonBounds = (Rectangle){419, 411, 128, 53};
    Rectangle cycleLeftButtonBounds = (Rectangle){84, 340, 41, 36};

    BeginDrawing();
    DrawTexture(s_menu_bg_tex2d, 0, 0, WHITE);
    DrawText(s_roms[s_selected_rom], cycleLeftButtonBounds.x + cycleLeftButtonBounds.width + 10, cycleLeftButtonBounds.y, 30, WHITE);
    DrawText("PLAY", playButtonBounds.x, playButtonBounds.y, 48, WHITE);
    DrawRectangle(0, 0, width, RASTER_ROWS * scale_y, BLACK);
    EndDrawing();
}

static void render_game()
{
    if(CHIP8_STEP && IsKeyPressed(KEY_F10))
    {
        vm_update();
    }
    else if(!CHIP8_STEP)
    {
        vm_update();
    }

    if (IsKeyPressed(KEY_F2))
    {
        render_state = render_menu;
        return;
    }

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

    BeginDrawing();
    ClearBackground(BLACK);

    for(int x = 0; x < RASTER_COLUMNS; ++x)
    {
        draw_column(x);
    }

    if(is_fps_shown)
    {
        
        const char* chip8Info = TextFormat("v0: %.02x  v1: %.02x  v2: %.02x  v3: %.02x  v4: %.02x  v5: %.02x  v6: %.02x  v7: %.02x\n"
            "v8: %.02x  v9: %.02x  va: %.02x  vb: %.02x  vc: %.02x  vd: %.02x  ve: %.02x  vf: %.02x\n"
            "index: %.04x  pc: %.04x  sp: %.02x  delay_timer: %.02x  sound_timer: %.02x\n"
            "speed: %d\n",
            s_chip8.v[0], s_chip8.v[1], s_chip8.v[2], s_chip8.v[3], s_chip8.v[4], s_chip8.v[5], s_chip8.v[6], s_chip8.v[7], 
            s_chip8.v[8], s_chip8.v[9], s_chip8.v[10], s_chip8.v[11], s_chip8.v[12], s_chip8.v[13], s_chip8.v[14], s_chip8.v[15],
            s_chip8.index, s_chip8.pc, s_chip8.sp, s_chip8.delay_timer, s_chip8.sound_timer, s_chip8.speed);
        DrawRectangle(0, 0, 600, 100, DARKGRAY);
        DrawText(chip8Info, 10, 30, 18, DARKGREEN);
        DrawFPS(10, 10);
    }

    if(GetTime() < s_transition_time)
    {
        DrawRectangle(0, 0, RASTER_COLUMNS * scale_x, RASTER_ROWS * scale_y, BLACK);
    }

    EndDrawing();
}
