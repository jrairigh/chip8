#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*InitFunc)(const char*);
typedef void (*UpdateFunc)(void);
typedef void (*ShutdownFunc)(void);

#ifdef RUN_TESTS
void renderer_initialize(const uint32_t* monitor) {}
void renderer_do_update(void) {}
void renderer_shutdown(void) {}
void renderer_set_init_func(InitFunc init_func) {(void)init_func;}
void renderer_set_update_func(UpdateFunc update_func) {(void)update_func;}
void renderer_set_shutdown_func(ShutdownFunc shutdown_func) {(void)shutdown_func;}
void renderer_log(int logLevel, const char* message, va_list args) {(void)logLevel; (void)message; (void)args;}
bool renderer_get_key(uint8_t* outKey) {(void)outKey; return true;}
bool renderer_is_key_down(uint8_t key) {(void)key; return true;}
void renderer_play_tone(void) {}
void renderer_stop_tone(void) {}
#else
void renderer_initialize(const uint32_t* monitor);
void renderer_do_update(void);
void renderer_shutdown(void);
void renderer_set_init_func(InitFunc init_func);
void renderer_set_update_func(UpdateFunc update_func);
void renderer_set_shutdown_func(ShutdownFunc shutdown_func);
void renderer_log(int log_level, const char* message, va_list args);
bool renderer_get_key(uint8_t* out_key);
bool renderer_is_key_down(uint8_t key);
void renderer_play_tone(void);
void renderer_stop_tone(void);
#endif

#endif
