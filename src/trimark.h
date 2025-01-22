#ifndef TRIMARK_H
#define TRIMARK_H

#include <stdint.h>

int trimark_init();
void trimark_cleanup();

void trimark_update();
void trimark_render(uint8_t *screen);

void trimark_print_results();

#endif
