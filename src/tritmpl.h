#if !defined(ALIGNED_FILL)
#   define ALIGNED_FILL 0
#   define ALIGNED_FILL_DEFINED_HERE
#endif

#if !defined(COUNTER_FILL)
#   define COUNTER_FILL 0
#   define COUNTER_FILL_DEFINED_HERE
#endif

static inline void DRAW_TRIANGLE_FUNC_NAME(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t x2, int32_t y2, uint8_t color, uint8_t *screen) {
    assert(screen != NULL && is_aligned(screen, 4));

    int32_t max_x = (max32(x0, max32(x1, x2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (max_x <= 0)
        return;

    int32_t min_x = (min32(x0, min32(x1, x2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (min_x >= SCREEN_WIDTH)
        return;

    int32_t max_y = (max32(y0, max32(y1, y2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (max_y <= 0)
        return;

    int32_t min_y = (min32(y0, min32(y1, y2)) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (min_y >= SCREEN_HEIGHT)
        return;

    max_x = min32(max_x, SCREEN_WIDTH); // last pixel column to draw + 1
    min_x = max32(min_x, 0);
    max_y = min32(max_y, SCREEN_HEIGHT); // last pixel row to draw + 1
    min_y = max32(min_y, 0);

    //
    // Align min_x, min_y to start from block corner

    min_x &= ~BLOCK_MASK;
    min_y &= ~BLOCK_MASK;

    //
    // Edge deltas

    int32_t dx0 = x0 - x1;
    int32_t dy0 = y0 - y1;
    int32_t dx1 = x1 - x2;
    int32_t dy1 = y1 - y2;
    int32_t dx2 = x2 - x0;
    int32_t dy2 = y2 - y0;

    //
    // Barycentric weights for bounding box corner (min_x, min_y)
    // and top-left fill rule adjustment

    int32_t c0 = imul32(dy0, x0) - imul32(dx0, y0);
    if (dy0 < 0 || (dy0 == 0 && dx0 > 0)) c0++;
    dx0 <<= SUBPIXEL_BITS;
    dy0 <<= SUBPIXEL_BITS;
    c0 += imul32(dx0, min_y) - imul32(dy0, min_x);

    int32_t c1 = imul32(dy1, x1) - imul32(dx1, y1);
    if (dy1 < 0 || (dy1 == 0 && dx1 > 0)) c1++;
    dx1 <<= SUBPIXEL_BITS;
    dy1 <<= SUBPIXEL_BITS;
    c1 += imul32(dx1, min_y) - imul32(dy1, min_x);

    int32_t c2 = imul32(dy2, x2) - imul32(dx2, y2);
    if (dy2 < 0 || (dy2 == 0 && dx2 > 0)) c2++;
    dx2 <<= SUBPIXEL_BITS;
    dy2 <<= SUBPIXEL_BITS;
    c2 += imul32(dx2, min_y) - imul32(dy2, min_x);

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

#if ALIGNED_FILL
    uint16_t color2 = (uint16_t)color | ((uint16_t)color << 8);
    uint32_t color4 = (uint32_t)color2 | ((uint32_t)color2 << 16);
#endif

    while (screen_row < screen_row_end) {
        int32_t x = min_x;

        int32_t cx0 = c0;
        int32_t cx1 = c1;
        int32_t cx2 = c2;

        while (!(cx0 + eo0 >= 0 &&
                 cx1 + eo1 >= 0 &&
                 cx2 + eo2 >= 0)) {
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
                int32_t left = x;

                int32_t cix0 = ciy0;
                int32_t cix1 = ciy1;
                int32_t cix2 = ciy2;

                while ((cix0 | cix1 | cix2) < 0) {
                    cix0 -= dy0;
                    cix1 -= dy1;
                    cix2 -= dy2;

                    left++;
                    if (left >= max_x) {
                        break;
                    }
                }

                if (left < max_x) {
                    int32_t right = left;

                    do {
                        cix0 -= dy0;
                        cix1 -= dy1;
                        cix2 -= dy2;
                        right++;
                    } while (right < max_x && (cix0 | cix1 | cix2) >= 0);

                    uint8_t *tgt = screen_row + left;
                    ONLY_DEBUG(uint8_t *debug_row_end = screen_row + right;)

#if COUNTER_FILL || ALIGNED_FILL
                    int32_t width = right - left;
                    assert(width > 0);
#endif
#if COUNTER_FILL
                    while (width--) {
                        assert(tgt < debug_row_end);
                        *tgt++ = color;
                    }
#else
#   if ALIGNED_FILL
                    if (width <= 4) {
#       if 1
                        switch (width) {
                            case 1:
                                assert(tgt < debug_row_end);
                                tgt[0] = color;
                                break;
                            case 2:
                                assert(tgt + 1 < debug_row_end);
                                tgt[0] = color;
                                tgt[1] = color;
                                break;
                            case 3:
                                assert(tgt + 2 < debug_row_end);
                                tgt[0] = color;
                                tgt[1] = color;
                                tgt[2] = color;
                                break;
                            case 4:
                                assert(tgt + 3 < debug_row_end);
                                tgt[0] = color;
                                tgt[1] = color;
                                tgt[2] = color;
                                tgt[3] = color;
                                break;
                            default:
                                assert(0);
                                break;
                        }
#       else
                        while (width--) {
                            assert(tgt < debug_row_end);
                            *tgt++ = color;
                        }
#       endif
                    } else {
                        int32_t num_unaligned_left = (4 - (left & 3)) & 3;
                        switch (num_unaligned_left) {
                            case 1:
                                assert(tgt < debug_row_end);
                                *tgt++ = color;
                                width--;
                                break;
                            case 2:
                                assert(tgt + 1 < debug_row_end);
                                *(uint16_t *)tgt = color2;
                                tgt += 2;
                                width -= 2;
                                break;
                            case 3:
                                assert(tgt + 2 < debug_row_end);
                                tgt[0] = color;
                                assert(is_aligned(tgt + 1, 2));
                                *(uint16_t *)(tgt + 1) = color2;
                                tgt += 3;
                                width -= 3;
                                break;
                            default:
                                break;
                        }

                        assert(is_aligned(tgt, 4));
                        uint32_t *tgt32 = (uint32_t *)tgt;
                        uint32_t *tgt32_end = (uint32_t *)tgt + (width >> 2);
                        while (tgt32 < tgt32_end) {
                            assert((uint8_t *)tgt32 >= screen_row);
                            assert((uint8_t *)tgt32 + 3 < debug_row_end);
                            *tgt32++ = color4;
                        }

                        tgt = (uint8_t *)tgt32;

                        switch (width & 3) {
                            case 1:
                                assert(tgt < debug_row_end);
                                tgt[0] = color;
                                break;
                            case 2:
                                assert(is_aligned(tgt, 2));
                                assert(tgt + 1 < debug_row_end);
                                *(uint16_t *)tgt = color2;
                                break;
                            case 3:
                                assert(is_aligned(tgt, 2));
                                assert(tgt + 2 < debug_row_end);
                                *(uint16_t *)tgt = color2;
                                tgt[2] = color;
                                break;
                            default:
                                break;
                        }
                    }
#   else
                    uint8_t *tgt_end = screen_row + right;
                    while (tgt < tgt_end) {
                        assert(tgt < debug_row_end);
                        *tgt++ = color;
                    }
#   endif
#endif
                }

                ciy0 += dx0;
                ciy1 += dx1;
                ciy2 += dx2;

                screen_row += SCREEN_WIDTH;
            }
        } else {
            screen_row += SCREEN_WIDTH << BLOCK_SIZE_SHIFT;
        }

        c0 += dx0 << BLOCK_SIZE_SHIFT;
        c1 += dx1 << BLOCK_SIZE_SHIFT;
        c2 += dx2 << BLOCK_SIZE_SHIFT;
    }
}

#if defined(ALIGNED_FILL_DEFINED_HERE)
#   undef ALIGNED_FILL
#   undef ALIGNED_FILL_DEFINED_HERE
#endif

#if defined(COUNTER_FILL_DEFINED_HERE)
#   undef COUNTER_FILL
#   undef COUNTER_FILL_DEFINED_HERE
#endif
