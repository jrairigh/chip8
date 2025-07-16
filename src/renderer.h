#ifndef RENDERER_H
#define RENDERER_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef RUN_TESTS
#include <stdio.h>
void renderer_initialize() {}
void renderer_do_update() {}
void renderer_shutdown() {}
void renderer_blit(int32_t* data) {(void)data;}
void renderer_set_init_func(void (*init_func)(const char*)) {(void)init_func;}
void renderer_set_update_func(void (*update_func)()) {(void)update_func;}
void renderer_set_shutdown_func(void (*shutdown_func)()) {(void)shutdown_func;}
void renderer_log(int logLevel, const char* message, va_list args) {(void)logLevel; vprintf(message, args);}
bool renderer_get_key(uint8_t* outKey) {(void)outKey; return true;}
bool renderer_is_key_down(uint8_t key) {(void)key; return true;}
void renderer_play_tone() {}
void renderer_stop_tone() {}
#else
void renderer_initialize();
void renderer_do_update();
void renderer_shutdown();
void renderer_blit(int32_t* data);
void renderer_set_init_func(void (*init_func)(const char*));
void renderer_set_update_func(void (*update_func)());
void renderer_set_shutdown_func(void (*shutdown_func)());
void renderer_log(int logLevel, const char* message, va_list args);
bool renderer_get_key(uint8_t* outKey);
bool renderer_is_key_down(uint8_t key);
void renderer_play_tone();
void renderer_stop_tone();
#endif

#endif
