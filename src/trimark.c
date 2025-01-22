#include "trimark.h"
#include "drawtris.h"
#include "screen.h"
#include "mytime.h"
#include "tridefs.h"

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
    return low + (int32_t)((uint64_t)xorshift32() *
        (uint64_t)(high - low) / UINT32_MAX);
}

static int32_t edge(int32_t x0, int32_t y0,
    int32_t x1, int32_t y1,
    int32_t x2, int32_t y2) {
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}

#define swap32(a, b) { int32_t c = b; b = a; a = c; }

//

#define FIRST_COLOR 1
#define LAST_COLOR (255 - 8)

#define SINGLE_TRIANGLE 0

#if SINGLE_TRIANGLE
#   define NUM_TRIANGLES 1
#else
#   define NUM_TRIANGLES 1000
#endif

static uint8_t *stored_screen = NULL;
static triangle_t *triangles = NULL;
static uint8_t *triangle_colors = NULL;

//

int trimark_init() {
    stored_screen = calloc(sizeof(*stored_screen) * SCREEN_NUM_PIXELS, 1);
    if (!stored_screen) {
        trimark_cleanup();
        return 0;
    }

    triangles = malloc(sizeof(*triangles) * NUM_TRIANGLES);
    if (!triangles) {
        trimark_cleanup();
        return 0;
    }

    triangle_colors = malloc(sizeof(*triangle_colors) * NUM_TRIANGLES);
    if (!triangle_colors) {
        trimark_cleanup();
        return 0;
    }

    triangle_t *triangles_tgt = triangles;
    uint8_t *colors_tgt = triangle_colors;

#if SINGLE_TRIANGLE
    triangle_t *t = triangles_tgt;
    t->x0 = SUBPIXEL_ONE * 180;
    t->y0 = SUBPIXEL_ONE * 2;
    t->x1 = SUBPIXEL_ONE * 2;
    t->y1 = SUBPIXEL_ONE * 80;
    t->x2 = SUBPIXEL_ONE * 220;
    t->y2 = SUBPIXEL_ONE * 160;
    *colors_tgt = 15;
#else
    uint8_t color = FIRST_COLOR;
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

        triangle_t *t = triangles_tgt++;
        t->x[0] = x0;
        t->y[0] = y0;
        t->x[1] = x1;
        t->y[1] = y1;
        t->x[2] = x2;
        t->y[2] = y2;

        *colors_tgt++ = color;

        color++;
        if (color == LAST_COLOR + 1)
            color = FIRST_COLOR;
    }
#endif

    return 1;
}

void trimark_cleanup() {
    free(triangle_colors); triangle_colors = NULL;
    free(triangles); triangles = NULL;
}

#if !SINGLE_TRIANGLE
#   define NUM_RETRIES 3
#   define NUM_RUNS (NUM_RETRIES * TRIANGLE_FUNC_COUNT)
static int rendering_done = 0;
static uint64_t rendering_begin_time[NUM_RUNS] = { 0 };
static uint64_t rendering_end_time[NUM_RUNS] =  { 0 };
#endif

void trimark_update() {
#if SINGLE_TRIANGLE
    if (1) {
        triangles[0].x0++;
        //triangles[0].x1++;
        //triangles[0].x2++;

        //triangles[0].y0++;
        triangles[0].y1++;
        //triangles[0].y2++;

        //triangle_colors[0]++;
    }
#else
    if (!rendering_done) {
        rendering_done = 1;
        for (uint32_t i = 0; i < NUM_RUNS; ++i) {
            rendering_begin_time[i] = time_get_us();
            draw_triangles(triangles, triangle_colors, NUM_TRIANGLES,
                i % TRIANGLE_FUNC_COUNT, stored_screen);
            rendering_end_time[i] = time_get_us();
        }
    }
#endif
}

void trimark_render(uint8_t *screen) {
#if SINGLE_TRIANGLE
    memset(screen, 0, SCREEN_NUM_PIXELS);
    draw_triangles(triangles, triangle_colors, NUM_TRIANGLES, screen);
#else
    memcpy(screen, stored_screen, sizeof(*screen) * SCREEN_NUM_PIXELS);
#endif
}

void trimark_print_results() {
#if !SINGLE_TRIANGLE
    for (uint32_t i = 0; i < NUM_RUNS; ++i) {
        uint8_t triangle_func_index = i % TRIANGLE_FUNC_COUNT;
        const char *const triangle_func_name =
            get_triangle_func_name(triangle_func_index);

        uint64_t us = rendering_end_time[i] - rendering_begin_time[i];
        uint64_t ms = us / 1000;
        uint64_t us_remainder = us - ms * 1000;
        printf("%u: %s %"PRIu64".%03"PRIu64"ms\n",
            (i / TRIANGLE_FUNC_COUNT) + 1,
            triangle_func_name, ms, us_remainder);
    }
#endif
}
