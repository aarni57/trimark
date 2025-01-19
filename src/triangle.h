#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>

#define SUBPIXEL_BITS    4
#define SUBPIXEL_ONE     (1 << SUBPIXEL_BITS)
#define SUBPIXEL_MASK    (SUBPIXEL_ONE - 1)

void draw_triangles(const int16_t *draw_buffer, uint32_t num, uint8_t *screen);
void draw_triangles_sorted(const int16_t *draw_buffer,
    const uint32_t *sort_buffer, uint32_t num, uint8_t *screen);

#endif
