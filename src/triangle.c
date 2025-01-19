#include "triangle.h"
#include "mymath.h"
#include "minmax.h"
#include "screen.h"

#include <assert.h>
#include <stddef.h>

//

#define BLOCK_SIZE_SHIFT 2
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

//

static inline void draw_triangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t x2, int32_t y2, uint8_t color, uint8_t *screen) {
    assert(screen != NULL);

    int32_t max_y = clamp32((max32(y0, max32(y1, y2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS, 0, SCREEN_Y_MAX);
    if (max_y == 0)
        return;

    int32_t min_y = clamp32((min32(y0, min32(y1, y2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS, 0, SCREEN_Y_MAX) & ~BLOCK_MASK;
    if (min_y == max_y)
        return;

    int32_t max_x = (max32(x0, max32(x1, x2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (max_x <= 0)
        return;

    int32_t min_x = (min32(x0, min32(x1, x2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS & ~BLOCK_MASK;
    if (min_x >= SCREEN_X_MAX)
        return;

    int32_t center_x = (max_x + min_x) >> 1;

    max_x = min32(max_x, SCREEN_X_MAX);
    min_x = max32(min_x, 0);
    center_x = clamp32(center_x, 0, SCREEN_X_MAX) & ~BLOCK_MASK;

    //

    int32_t dx0 = x0 - x1;
    int32_t dy0 = y0 - y1;
    int32_t dx1 = x1 - x2;
    int32_t dy1 = y1 - y2;
    int32_t dx2 = x2 - x0;
    int32_t dy2 = y2 - y0;

    //

    int32_t c0, c1, c2;

    {
        int32_t c = imul32(dy0, x0) - imul32(dx0, y0);
        if (dy0 < 0 || (dy0 == 0 && dx0 > 0)) c++;
        dx0 <<= SUBPIXEL_BITS;
        dy0 <<= SUBPIXEL_BITS;
        c0 = c + imul32(dx0, min_y) - imul32(dy0, min_x);
    }

    //

    {
        int32_t c = imul32(dy1, x1) - imul32(dx1, y1);
        if (dy1 < 0 || (dy1 == 0 && dx1 > 0)) c++;
        dx1 <<= SUBPIXEL_BITS;
        dy1 <<= SUBPIXEL_BITS;
        c1 = c + imul32(dx1, min_y) - imul32(dy1, min_x);
    }

    //

    {
        int32_t c = imul32(dy2, x2) - imul32(dx2, y2);
        if (dy2 < 0 || (dy2 == 0 && dx2 > 0)) c++;
        dx2 <<= SUBPIXEL_BITS;
        dy2 <<= SUBPIXEL_BITS;
        c2 = c + imul32(dx2, min_y) - imul32(dy2, min_x);
    }

    //
    // Offsets for empty block testing

    int32_t eo0 = 0;
    if (dy0 < 0) eo0 = eo0 - (dy0 << BLOCK_SIZE_SHIFT);
    if (dx0 > 0) eo0 = eo0 + (dx0 << BLOCK_SIZE_SHIFT);

    int32_t eo1 = 0;
    if (dy1 < 0) eo1 = eo1 - (dy1 << BLOCK_SIZE_SHIFT);
    if (dx1 > 0) eo1 = eo1 + (dx1 << BLOCK_SIZE_SHIFT);

    int32_t eo2 = 0;
    if (dy2 < 0) eo2 = eo2 - (dy2 << BLOCK_SIZE_SHIFT);
    if (dx2 > 0) eo2 = eo2 + (dx2 << BLOCK_SIZE_SHIFT);

    //

    uint8_t *screen_row = screen + mul_by_screen_stride(min_y);
    uint8_t *screen_row_end = screen + mul_by_screen_stride(max_y);

    uint32_t color4 = color | (color << 8) | (color << 16) | (color << 24);

    uint8_t flags = 0;

    while (screen_row < screen_row_end) {
        int32_t x = min_x;

        int32_t cx0 = c0;
        int32_t cx1 = c1;
        int32_t cx2 = c2;

        while (!((cx0 + eo0) > 0 &&
                 (cx1 + eo1) > 0 &&
                 (cx2 + eo2) > 0)) {
            cx0 -= dy0 << BLOCK_SIZE_SHIFT;
            cx1 -= dy1 << BLOCK_SIZE_SHIFT;
            cx2 -= dy2 << BLOCK_SIZE_SHIFT;
            x += BLOCK_SIZE;
            if (x >= max_x)
                break;
        }

        if (x < max_x) {
            int32_t ciy0 = cx0;
            int32_t ciy1 = cx1;
            int32_t ciy2 = cx2;

            for (uint8_t iy = 0; iy < BLOCK_SIZE; ++iy) {
                int32_t hleft = x;

                int32_t cix0 = ciy0;
                int32_t cix1 = ciy1;
                int32_t cix2 = ciy2;

                while ((cix0 | cix1 | cix2) <= 0) {
                    cix0 -= dy0;
                    cix1 -= dy1;
                    cix2 -= dy2;

                    hleft++;
                    if (hleft > max_x) {
                        break;
                    }
                }

                if (hleft <= max_x) {
                    int32_t hright = hleft;

                    while ((cix0 | cix1 | cix2) > 0) {
                        cix0 -= dy0;
                        cix1 -= dy1;
                        cix2 -= dy2;

                        hright++;
                        if (hright > max_x) {
                            break;
                        }
                    }

                    {
                        uint32_t width = hright - hleft;
                        uint32_t width4 = width >> 2;

                        uint32_t *tgt32 = (uint32_t *)(screen_row + hleft);
                        uint32_t *tgt32_end = tgt32 + width4;
                        while (tgt32 < tgt32_end)
                            *tgt32++ = color4;

                        width -= width4 << 2;
                        uint8_t *tgt = (uint8_t *)tgt32;
                        switch (width) {
                            case 3:
                                tgt[0] = color;
                                tgt[1] = color;
                                tgt[2] = color;
                                break;
                            case 2:
                                tgt[0] = color;
                                tgt[1] = color;
                                break;
                            case 1:
                                tgt[0] = color;
                                break;
                            default:
                                break;
                        }
                    }
                }

                ciy0 += dx0;
                ciy1 += dx1;
                ciy2 += dx2;

                screen_row += SCREEN_STRIDE;
                flags |= 1;
            }
        } else {
            if (flags & 1)
                return;
            screen_row += SCREEN_STRIDE << BLOCK_SIZE_SHIFT;
        }

        c0 += dx0 << BLOCK_SIZE_SHIFT;
        c1 += dx1 << BLOCK_SIZE_SHIFT;
        c2 += dx2 << BLOCK_SIZE_SHIFT;
    }
}

void draw_triangles(const int16_t *draw_buffer, uint32_t num, uint8_t *screen) {
    assert(draw_buffer && screen);
    num <<= 3;
    for (uint32_t i = 0; i < num; i += 8) {
        int32_t x0 = draw_buffer[i + 0];
        int32_t y0 = draw_buffer[i + 1];
        int32_t x1 = draw_buffer[i + 2];
        int32_t y1 = draw_buffer[i + 3];
        int32_t x2 = draw_buffer[i + 4];
        int32_t y2 = draw_buffer[i + 5];
        uint8_t color = draw_buffer[i + 6];

        draw_triangle(x0, y0, x1, y1, x2, y2, color, screen);
    }
}

void draw_triangles_sorted(const int16_t *draw_buffer,
    const uint32_t *sort_buffer, uint32_t num, uint8_t *screen) {
    assert(draw_buffer && sort_buffer && screen);
    for (uint32_t i = 0; i < num; ++i) {
        uint32_t j = sort_buffer[i] >> 16;
        assert(j < num);
        j <<= 3;

        int32_t x0 = draw_buffer[j + 0];
        int32_t y0 = draw_buffer[j + 1];
        int32_t x1 = draw_buffer[j + 2];
        int32_t y1 = draw_buffer[j + 3];
        int32_t x2 = draw_buffer[j + 4];
        int32_t y2 = draw_buffer[j + 5];
        uint8_t color = draw_buffer[j + 6];

        draw_triangle(x0, y0, x1, y1, x2, y2, color, screen);
    }
}
