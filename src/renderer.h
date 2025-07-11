#ifndef RENDERER_H
#define RENDERER_H

#ifdef RUN_TESTS
#include <stdio.h>
void renderer_blit(int* data) {(void)data;}
void renderer_log(const char* message) {printf("%s\n", message);}
#else
void renderer_initialize();
void renderer_do_update();
void renderer_shutdown();
void renderer_blit(int* data);
void renderer_set_update_func(void (*update_func)());
void renderer_log(const char* message);
#endif

#endif
