#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum LogLevel
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR, 
} LogLevel;

typedef void (*InitFunc)(const char*);
typedef void (*UpdateFunc)(void);
typedef void (*ShutdownFunc)(void);

void monitor_initialize(InitFunc init_func, UpdateFunc update_func, ShutdownFunc shutdown_func);
void monitor_clear(void);
void monitor_draw_sprite(uint8_t x, uint8_t y, const uint8_t* sprite, uint8_t sprite_size_in_bytes, bool* did_collide);
bool monitor_get_key(uint8_t* out_key);
bool monitor_is_key_down(uint8_t key);
void monitor_play_tone(void);
void monitor_stop_tone(void);
void monitor_log(LogLevel level, const char* text, ...);

#endif
