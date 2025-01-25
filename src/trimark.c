#include "trimark.h"
#include "drawtris.h"
#include "screen.h"
#include "mytime.h"
#include "tridefs.h"
#include "util.h"
#include "random.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//

static int32_t edge(int32_t x0, int32_t y0,
    int32_t x1, int32_t y1,
    int32_t x2, int32_t y2) {
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}

//

#define FIRST_COLOR 32
#define LAST_COLOR 87

static uint8_t *stored_screens[TRIANGLE_FUNC_COUNT] = { NULL };

#define NUM_TRIANGLE_SETS 4
static triangle_t *triangles[NUM_TRIANGLE_SETS] = { NULL };
static uint8_t *triangle_colors[NUM_TRIANGLE_SETS] = { NULL };

typedef struct triangle_set_params_t {
    int32_t min_area, max_area, min_x, min_y, max_x, max_y;
    uint32_t num_triangles;
} triangle_set_params_t;

static const triangle_set_params_t triangle_set_params[NUM_TRIANGLE_SETS] = {
    {
        1200 * 1200, 0,
        0,
        0,
        (SCREEN_WIDTH / 2 - 1) * SUBPIXEL_ONE,
        (SCREEN_HEIGHT / 2 - 1) * SUBPIXEL_ONE,
        128,
    },
    {
        600 * 600, 1200 * 1200,
        (SCREEN_WIDTH / 2) * SUBPIXEL_ONE,
        0,
        (SCREEN_WIDTH - 1) * SUBPIXEL_ONE,
        (SCREEN_HEIGHT / 2 - 1) * SUBPIXEL_ONE,
        256,
    },
    {
        300 * 300, 600 * 600,
        0,
        (SCREEN_HEIGHT / 2) * SUBPIXEL_ONE,
        (SCREEN_WIDTH / 2 - 1) * SUBPIXEL_ONE,
        (SCREEN_HEIGHT - 1) * SUBPIXEL_ONE,
        512,
    },
    {
        16 * 16, 300 * 300,
        (SCREEN_WIDTH / 2) * SUBPIXEL_ONE,
        (SCREEN_HEIGHT / 2) * SUBPIXEL_ONE,
        (SCREEN_WIDTH - 1) * SUBPIXEL_ONE,
        (SCREEN_HEIGHT - 1) * SUBPIXEL_ONE,
        1024,
    },
};

// NUM_RETRIES can be increased to get a reliable measurement on systems that
// have fluctuating CPU speed (for example when running in DOSBox)
#define NUM_RETRIES 1

#define NUM_RUNS (NUM_RETRIES * TRIANGLE_FUNC_COUNT)

static uint64_t rendering_begin_time[NUM_RUNS][NUM_TRIANGLE_SETS] = { 0 };
static uint64_t rendering_end_time[NUM_RUNS][NUM_TRIANGLE_SETS] =  { 0 };

//

void generate_random_triangles(triangle_t *triangles_tgt, uint8_t *colors_tgt,
    int32_t min_area, int32_t max_area, int32_t min_x, int32_t min_y,
    int32_t max_x, int32_t max_y, uint32_t num) {
    uint32_t seed = 0xcafebabe;
    uint8_t color = FIRST_COLOR;
    for (uint32_t i = 0; i < num; ++i) {
        int32_t x0, y0, x1, y1, x2, y2;
        int32_t area = 0;
        do {
            x0 = random32_range(&seed, min_x, max_x);
            y0 = random32_range(&seed, min_y, max_y);
            x1 = random32_range(&seed, min_x, max_x);
            y1 = random32_range(&seed, min_y, max_y);
            x2 = random32_range(&seed, min_x, max_x);
            y2 = random32_range(&seed, min_y, max_y);

            if (edge(x0, y0, x1, y1, x2, y2) < 0) {
                swap32(x0, x1);
                swap32(y0, y1);
            }

            area = edge(x0, y0, x1, y1, x2, y2);
            assert(area >= 0);
        } while (!((min_area == 0 || area >= min_area) &&
            (max_area == 0 || area <= max_area)));

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
}

//

int trimark_init() {
    for (uint32_t i = 0; i < TRIANGLE_FUNC_COUNT; ++i) {
        stored_screens[i] = calloc(SCREEN_NUM_PIXELS, sizeof(*stored_screens[i]));
        if (!stored_screens[i]) {
            trimark_cleanup();
            return 0;
        }
    }

    for (uint32_t i = 0; i < NUM_TRIANGLE_SETS; ++i) {
        const triangle_set_params_t *p = &triangle_set_params[i];

        triangles[i] = malloc(sizeof(*triangles[i]) * p->num_triangles);
        if (!triangles[i]) {
            trimark_cleanup();
            return 0;
        }

        triangle_colors[i] = malloc(sizeof(*triangle_colors[i]) * p->num_triangles);
        if (!triangle_colors[i]) {
            trimark_cleanup();
            return 0;
        }

        generate_random_triangles(triangles[i], triangle_colors[i],
            p->min_area, p->max_area, p->min_x, p->min_y, p->max_x, p->max_y,
            p->num_triangles);
    }

    return 1;
}

void trimark_cleanup() {
    for (uint32_t i = 0; i < NUM_TRIANGLE_SETS; ++i) {
        free(triangle_colors[i]); triangle_colors[i] = NULL;
        free(triangles[i]); triangles[i] = NULL;
    }

    for (uint32_t i = 0; i < TRIANGLE_FUNC_COUNT; ++i) {
        free(stored_screens[i]); stored_screens[i] = NULL;
    }
}

#if TEST_RASTERIZATION_DIFFERENCES
static uint32_t rasterization_differences = 0;
#endif

void trimark_run() {
    printf("Triangle rasterization benchmark running\n");
    printf("This may take a while...\n");

    for (uint32_t i = 0; i < NUM_RUNS; ++i) {
        uint32_t func_index = i % TRIANGLE_FUNC_COUNT;
        uint8_t *screen = stored_screens[func_index];
        for (uint32_t j = 0; j < NUM_TRIANGLE_SETS; ++j) {
            rendering_begin_time[i][j] = time_get_us();
            draw_triangles(triangles[j], triangle_colors[j],
                triangle_set_params[j].num_triangles, func_index, screen);
            rendering_end_time[i][j] = time_get_us();
        }
    }

#if TEST_RASTERIZATION_DIFFERENCES
    uint8_t *ref = stored_screens[TRIANGLE_FUNC_COUNT - 1];
    for (uint32_t i = 0; i < TRIANGLE_FUNC_COUNT - 1; ++i) {
        uint8_t *cmp = stored_screens[i];
        for (uint32_t j = 0; j < SCREEN_NUM_PIXELS; ++j) {
            if (cmp[j] != ref[j]) {
                rasterization_differences++;
            }
        }
    }
#endif
}

void trimark_blit(uint8_t *screen) {
#if 0
    memset(screen, 0, SCREEN_NUM_PIXELS);
    draw_triangles(triangles, triangle_colors, NUM_TRIANGLES, screen);
#else
    memcpy(screen, stored_screens[0], sizeof(*screen) * SCREEN_NUM_PIXELS);
#endif
}

void trimark_print_results() {
    for (uint32_t i = 0; i < NUM_TRIANGLE_SETS; ++i) {
        const triangle_set_params_t *p = &triangle_set_params[i];
        printf("set %u: %u triangles, min area: %d, max area: %d\n",
            i, p->num_triangles, p->min_area >> 8, p->max_area >> 8);
    }

    printf("\n");

    uint64_t min_times[TRIANGLE_FUNC_COUNT][NUM_TRIANGLE_SETS];
    for (uint32_t i = 0; i < TRIANGLE_FUNC_COUNT; ++i) {
        for (uint32_t j = 0; j < NUM_TRIANGLE_SETS; ++j) {
            min_times[i][j] = 100000000000ULL;
        }
    }

    for (uint32_t i = 0; i < NUM_RUNS; ++i) {
        uint8_t func_index = i % TRIANGLE_FUNC_COUNT;
        for (uint32_t j = 0; j < NUM_TRIANGLE_SETS; ++j) {
            uint64_t us = rendering_end_time[i][j] - rendering_begin_time[i][j];
            if (us < min_times[func_index][j])
                min_times[func_index][j] = us;
        }
    }

    uint64_t min_times_total[TRIANGLE_FUNC_COUNT];
    for (uint32_t i = 0; i < TRIANGLE_FUNC_COUNT; ++i) {
        min_times_total[i] = 0;
        for (uint32_t j = 0; j < NUM_TRIANGLE_SETS; ++j) {
            min_times_total[i] += min_times[i][j];
        }
    }

    for (uint32_t i = 0; i < TRIANGLE_FUNC_COUNT; ++i) {
        const char *const triangle_func_name =
            get_triangle_func_name(i);

        {
            uint64_t us = min_times_total[i];
            uint64_t ms = (us + 500) / 1000;
            printf("%s total: %"PRIu64"ms",
                triangle_func_name, ms);
        }

        for (uint32_t j = 0; j < NUM_TRIANGLE_SETS; ++j) {
            uint64_t us = min_times[i][j];
            uint64_t ms = (us + 50) / 1000;
            uint64_t us_remainder = (us - ms * 1000 + 50) / 100;
            printf(", %u: %"PRIu64".%01"PRIu64"ms",
                j, ms, us_remainder);
        }

        printf("\n");
    }

#if TEST_RASTERIZATION_DIFFERENCES
    if (rasterization_differences != 0) {
        printf("Num rasterization differences to ref: %u\n",
            rasterization_differences);
    }
#endif
}
