#ifndef MYTIME_H
#define MYTIME_H

#include <stdint.h>

uint64_t time_get_us();
void time_sleep(uint64_t us);

#endif
