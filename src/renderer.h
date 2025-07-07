#ifndef RENDERER_H
#define RENDERER_H

void renderer_initialize();
void renderer_do_update();
void renderer_shutdown();
void renderer_blit(int* data);
void renderer_set_update_func(void (*update_func)());

#endif
