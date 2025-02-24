#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define NUM_COLORS 256

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define SCREEN_CENTER_X (SCREEN_WIDTH >> 1)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT >> 1)

#define SCREEN_NUM_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)

static inline uint32_t mul_by_screen_stride(uint32_t x) {
    return (x << 8) + (x << 6);
}

#endif
