#ifndef DEMO_H
#define DEMO_H

#include <stdint.h>

int demo_init();
void demo_cleanup();

void demo_update();
void demo_render(uint8_t *screen);

#endif
