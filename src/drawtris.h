#ifndef DRAWTRIS_H
#define DRAWTRIS_H

#include <stdint.h>

enum {
    // Barycentric
    TRIANGLE_FUNC_REF,
    TRIANGLE_FUNC_4,
    TRIANGLE_FUNC_4_AF,
    TRIANGLE_FUNC_4_CF,
    TRIANGLE_FUNC_8,
    TRIANGLE_FUNC_8_AF,
    TRIANGLE_FUNC_8_CF,

    // Standard
    TRIANGLE_FUNC_STANDARD,

    TRIANGLE_FUNC_COUNT
};

const char *get_triangle_func_name(uint8_t index);

typedef struct triangle_t {
    int16_t x[3], y[3];
} triangle_t;

void draw_triangles(const triangle_t *triangles, const uint8_t *colors,
    uint32_t num, uint8_t func_index, uint8_t *screen);

void draw_triangles_sorted(const triangle_t *triangles, const uint8_t *colors,
    const uint32_t *sort_buffer, uint32_t num, uint8_t func_index,
    uint8_t *screen);

#endif
