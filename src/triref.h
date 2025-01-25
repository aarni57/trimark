static inline void draw_triangle_ref(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t x2, int32_t y2, uint8_t color, uint8_t *screen) {
    assert(screen != NULL);

#if 0
    int32_t min_x = 0;
    int32_t max_x = SCREEN_WIDTH;
    int32_t min_y = 0;
    int32_t max_y = SCREEN_HEIGHT;
#else
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

    max_x = min32(max_x, SCREEN_WIDTH); // last column to draw + 1
    min_x = max32(min_x, 0);
    max_y = min32(max_y, SCREEN_HEIGHT); // last row to draw + 1
    min_y = max32(min_y, 0);
#endif

    int32_t dx0 = x0 - x1;
    int32_t dy0 = y0 - y1;
    int32_t dx1 = x1 - x2;
    int32_t dy1 = y1 - y2;
    int32_t dx2 = x2 - x0;
    int32_t dy2 = y2 - y0;

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

    uint8_t *screen_row = screen + mul_by_screen_stride(min_y);
    uint8_t *screen_row_end = screen + mul_by_screen_stride(max_y);

    while (screen_row < screen_row_end) {
#if !defined(NDEBUG)
        uint8_t *debug_screen_end = screen_row + SCREEN_WIDTH;
#endif

        int32_t left = min_x;

        int32_t cx0 = c0;
        int32_t cx1 = c1;
        int32_t cx2 = c2;

        while ((cx0 | cx1 | cx2) < 0) {
            cx0 -= dy0;
            cx1 -= dy1;
            cx2 -= dy2;

            left++;
            if (left >= max_x) {
                break;
            }
        }

        if (left < max_x) {
            int32_t right = left;
            do {
                right++;
                cx0 -= dy0;
                cx1 -= dy1;
                cx2 -= dy2;
            } while (right < max_x && (cx0 | cx1 | cx2) >= 0);

            uint8_t *tgt = screen_row + left;
            uint8_t *tgt_end = screen_row + right;
            while (tgt < tgt_end) {
                assert(tgt < debug_screen_end);
                *tgt++ = color;
            }
        }

        c0 += dx0;
        c1 += dx1;
        c2 += dx2;

        screen_row += SCREEN_WIDTH;
    }
}
