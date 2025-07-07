#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>

void monitor_clear();
void monitor_paint();
void monitor_do_update();
void monitor_set_pixel(int x, int y, bool* didCollide);

#endif
