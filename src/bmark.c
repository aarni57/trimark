#include "bmark.h"
#include "triangle.h"
#include "screen.h"
#include "mytime.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//

static uint32_t xorshift32_state = 0xcafebabe;

static uint32_t xorshift32() {
    uint32_t x = xorshift32_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return xorshift32_state = x;
}

static int32_t random32_range(int32_t low, int32_t high) {
    assert(low <= high);
    return low + (int32_t)((uint64_t)xorshift32() * (uint64_t)(high - low) / UINT32_MAX);
}

static int32_t edge(
    int32_t x0, int32_t y0,
    int32_t x1, int32_t y1,
    int32_t x2, int32_t y2) {
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}

#define swap32(a, b) { int32_t c = b; b = a; a = c; }

//

#define FIRST_COLOR 1
#define LAST_COLOR (255 - 8)

#define NUM_TRIANGLES 5000

static uint8_t *stored_screen = NULL;
static int16_t *draw_buffer = NULL;

//

int bmark_init() {
    stored_screen = calloc(sizeof(*stored_screen) * SCREEN_NUM_PIXELS, 1);
    if (!stored_screen) {
        bmark_cleanup();
        return 0;
    }

    draw_buffer = malloc(sizeof(*draw_buffer) * 8 * NUM_TRIANGLES);
    if (!draw_buffer) {
        bmark_cleanup();
        return 0;
    }

    uint8_t color = FIRST_COLOR;

    int16_t *draw_buffer_tgt = draw_buffer;
    for (uint32_t i = 0; i < NUM_TRIANGLES; ++i) {
        int32_t x0, y0, x1, y1, x2, y2;
        int32_t area = 0;
        do {
            x0 = random32_range(0, SCREEN_WIDTH * SUBPIXEL_ONE);
            y0 = random32_range(0, SCREEN_HEIGHT * SUBPIXEL_ONE);
            x1 = random32_range(0, SCREEN_WIDTH * SUBPIXEL_ONE);
            y1 = random32_range(0, SCREEN_HEIGHT * SUBPIXEL_ONE);
            x2 = random32_range(0, SCREEN_WIDTH * SUBPIXEL_ONE);
            y2 = random32_range(0, SCREEN_HEIGHT * SUBPIXEL_ONE);

            if (edge(x0, y0, x1, y1, x2, y2) < 0) {
                swap32(x0, x1);
                swap32(y0, y1);
            }

            area = edge(x0, y0, x1, y1, x2, y2);
            assert(area >= 0);
        } while (area == 0);

        *draw_buffer_tgt++ = x0;
        *draw_buffer_tgt++ = y0;
        *draw_buffer_tgt++ = x1;
        *draw_buffer_tgt++ = y1;
        *draw_buffer_tgt++ = x2;
        *draw_buffer_tgt++ = y2;
        *draw_buffer_tgt++ = color;
        *draw_buffer_tgt++ = 0; // Unused

        color++;
        if (color == LAST_COLOR + 1)
            color = FIRST_COLOR;
    }

    return 1;
}

void bmark_cleanup() {
    free(draw_buffer); draw_buffer = NULL;
}

#define NUM_RUNS 4

static int rendering_done = 0;
static uint64_t rendering_begin_time[NUM_RUNS] = { 0 };
static uint64_t rendering_end_time[NUM_RUNS] =  { 0 };

void bmark_update() {
    if (!rendering_done) {
        rendering_done = 1;
        for (uint32_t i = 0; i < NUM_RUNS; ++i) {
            rendering_begin_time[i] = time_get_us();
            draw_triangles(draw_buffer, NUM_TRIANGLES, stored_screen);
            rendering_end_time[i] = time_get_us();
        }
    }
}

void bmark_render(uint8_t *screen) {
    memcpy(screen, stored_screen, sizeof(*screen) * SCREEN_NUM_PIXELS);
}

void bmark_print_results() {
    for (uint32_t i = 0; i < NUM_RUNS; ++i) {
        uint64_t us = rendering_end_time[i] - rendering_begin_time[i];
        uint64_t ms = us / 1000;
        uint64_t us_remainder = us - ms * 1000;
        printf("Run #%u: %"PRIu64".%"PRIu64"ms\n", i + 1, ms, us_remainder);
    }
}
