#ifndef BMARK_H
#define BMARK_H

#include <stdint.h>

int bmark_init();
void bmark_cleanup();

void bmark_update();
void bmark_render(uint8_t *screen);

void bmark_print_results();

#endif
