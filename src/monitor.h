#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>
#include <stdint.h>

void monitor_clear();
void monitor_do_update();
void monitor_draw_sprite(int32_t x, int32_t y, uint8_t* sprite, uint32_t sprite_size_in_bytes, bool* didCollide);

#endif
