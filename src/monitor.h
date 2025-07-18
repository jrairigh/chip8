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
typedef void (*UpdateFunc)();
typedef void (*ShutdownFunc)();

void monitor_initialize(InitFunc init_func, UpdateFunc update_func, ShutdownFunc shutdown_func);
void monitor_clear();
void monitor_draw_sprite(uint8_t x, uint8_t y, uint8_t* sprite, uint8_t sprite_size_in_bytes, bool* didCollide);
bool monitor_get_key(uint8_t* outKey);
bool monitor_is_key_down(uint8_t key);
void monitor_play_tone();
void monitor_stop_tone();
void monitor_log(LogLevel level, const char* text, ...);

#endif
