#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef RUN_TESTS
#include <stdio.h>
void renderer_blit(int32_t* data) {(void)data;}
void renderer_log(const char* message) {printf("%s\n", message);}
bool renderer_get_key(uint8_t* outKey) {(void)outKey; return true;}
bool renderer_is_key_down(uint8_t key) {(void)key; return true;}
#else
void renderer_initialize();
void renderer_do_update();
void renderer_shutdown();
void renderer_blit(int32_t* data);
void renderer_set_update_func(void (*update_func)());
void renderer_log(const char* message);
bool renderer_get_key(uint8_t* outKey);
bool renderer_is_key_down(uint8_t key);
#endif

#endif
