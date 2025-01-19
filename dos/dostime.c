#include "mytime.h"

#include <time.h>
#include <unistd.h>

uint64_t time_get_us() {
    const uclock_t t = uclock();
    return (uint64_t)t * 1000000ULL / UCLOCKS_PER_SEC;
}

void time_sleep(uint64_t us) {
    usleep(us);
}
