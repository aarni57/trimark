// Standard scanline rasterizer template

#define TRAVERSE_EDGE MAKE_TRAVERSE_EDGE_FUNC_NAME(DRAW_TRIANGLE_FUNC_NAME)

#define DDA_BITS 12

static void TRAVERSE_EDGE(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t *left_edge_buffer, int32_t *right_edge_buffer) {
    if (y0 > y1) {
        swap32(x0, x1);
        swap32(y0, y1);
    }

    int32_t iy0 = (y0 + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    int32_t iy1 = (y1 + SUBPIXEL_MASK) >> SUBPIXEL_BITS;

    int32_t height = y1 - y0;
    if (height == 0)
        return;

    int32_t x_step = idiv32((x1 - x0) << (DDA_BITS + SUBPIXEL_BITS), height);
    int32_t x = x0 << DDA_BITS;

    int32_t prestep = (iy0 << SUBPIXEL_BITS) - y0;
    x += imul32(x_step, prestep) >> SUBPIXEL_BITS;

    int32_t y = iy0;
    while (y <= iy1) {
        int32_t ix = x >> DDA_BITS;
        if (left_edge_buffer[y] > ix)
            left_edge_buffer[y] = ix;
        if (right_edge_buffer[y] < ix)
            right_edge_buffer[y] = ix;

        x += x_step;
        y++;
    }
}

#undef DDA_BITS

static inline void DRAW_TRIANGLE_FUNC_NAME(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t x2, int32_t y2, uint8_t color, uint8_t *screen) {
    assert(screen != NULL && is_aligned(screen, 4));

    int32_t min_y = (min32(min32(y0, y1), y2) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (min_y > SCREEN_HEIGHT)
        return;

    int32_t max_y = (max32(max32(y0, y1), y2) + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    if (max_y <= 0)
        return;

    min_y = max32(min_y, 0);
    max_y = min32(max_y, SCREEN_HEIGHT);

    // TODO: Find the top point and traverse left and right sides separately

    for (int32_t y = min_y; y < max_y; ++y) {
        left_edge_buffer[y] = INT32_MAX;
        right_edge_buffer[y] = INT32_MIN;
    }

    TRAVERSE_EDGE(x0, y0, x1, y1, left_edge_buffer, right_edge_buffer);
    TRAVERSE_EDGE(x1, y1, x2, y2, left_edge_buffer, right_edge_buffer);
    TRAVERSE_EDGE(x2, y2, x0, y0, left_edge_buffer, right_edge_buffer);

    uint8_t *screen_row = screen + mul_by_screen_stride(min_y);

    uint8_t y = min_y;
    while (y < (uint8_t)max_y) {
        assert(y < SCREEN_HEIGHT);
        assert(left_edge_buffer[y] != INT32_MAX);
        assert(right_edge_buffer[y] != INT32_MIN);
        int32_t left = max32((left_edge_buffer[y] + SUBPIXEL_MASK) >> SUBPIXEL_BITS, 0);
        int32_t right = min32((right_edge_buffer[y] + SUBPIXEL_MASK) >> SUBPIXEL_BITS, SCREEN_WIDTH);
        for (int32_t x = left; x < right; x++) {
            screen_row[x] = color;
        }

        screen_row += SCREEN_WIDTH;
        y++;
    }
}

#undef TRAVERSE_EDGE
