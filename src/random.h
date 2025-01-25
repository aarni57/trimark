#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>
#include <assert.h>

static uint32_t xorshift32(uint32_t *seed) {
    uint32_t x = *seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *seed = x;
    return x;
}

static int32_t random32_range(uint32_t *seed, int32_t low, int32_t high) {
    assert(low <= high);
    uint32_t r = xorshift32(seed);
    return low + (int32_t)((uint64_t)r * (uint64_t)(high - low) / UINT32_MAX);
}

#endif
