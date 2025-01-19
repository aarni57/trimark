#include "mytime.h"

#include <time.h>
#include <unistd.h>

uint64_t time_get_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000;
}

void time_sleep(uint64_t us) {
    usleep((useconds_t)us);
}
