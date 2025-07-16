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

void monitor_initialize(void (*init_func)(const char*), void (*update_func)(), void (*shutdown_func)());
void monitor_clear();
void monitor_draw_sprite(int32_t x, int32_t y, uint8_t* sprite, uint32_t sprite_size_in_bytes, bool* didCollide);
bool monitor_get_key(uint8_t* outKey);
bool monitor_is_key_down(uint8_t key);
void monitor_play_tone();
void monitor_stop_tone();
void monitor_log(LogLevel level, const char* text);

#endif
