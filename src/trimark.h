#ifndef TRIMARK_H
#define TRIMARK_H

#include <stdint.h>

int trimark_init(int argc, const char *const *argv);
void trimark_cleanup();

void trimark_run();
void trimark_update();
void trimark_blit(uint8_t *screen);

void trimark_print_results();

#endif
