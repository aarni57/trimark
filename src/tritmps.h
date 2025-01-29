// Standard rasterizer template using DDA line algorithm

#define TRAVERSE_EDGE MAKE_TRAVERSE_EDGE_FUNC_NAME(DRAW_TRIANGLE_FUNC_NAME)

#define DDA_BITS 12

static void TRAVERSE_EDGE(
    int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t dx, int32_t *edge_buffer) {
    assert(y0 < y1);

    int32_t iy0 = (y0 + SUBPIXEL_MASK) >> SUBPIXEL_BITS;
    int32_t iy1 = min32((y1 + SUBPIXEL_MASK) >> SUBPIXEL_BITS, SCREEN_HEIGHT);

    int32_t prestep = (iy0 << SUBPIXEL_BITS) - y0;
    int32_t x = (x0 << DDA_BITS) + (imul32(dx, prestep) >> SUBPIXEL_BITS);

    int32_t y = iy0;
    if (y < 0) {
        x += dx * -y;
        y = 0;
    }

    while (y < iy1) {
        assert(y >= 0 && y < SCREEN_HEIGHT);
        edge_buffer[y] = x;
        x += dx;
        y++;
    }
}

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

    if (min_y == max_y)
        return;

    if (y0 > y1) {
        swap32(x0, x1);
        swap32(y0, y1);
    }

    if (y0 > y2) {
        swap32(x0, x2);
        swap32(y0, y2);
    }

    if (y1 > y2) {
        swap32(x1, x2);
        swap32(y1, y2);
    }

    assert(y0 <= y1 && y1 <= y2);

    if (y0 == y1) {
        // Flat top
        int32_t dx02 = idiv32((x2 - x0) << (DDA_BITS + SUBPIXEL_BITS), y2 - y0);
        int32_t dx12 = idiv32((x2 - x1) << (DDA_BITS + SUBPIXEL_BITS), y2 - y1);
        if (x0 < x1) {
            TRAVERSE_EDGE(x0, y0, x2, y2, dx02, left_edge_buffer);
            TRAVERSE_EDGE(x1, y1, x2, y2, dx12, right_edge_buffer);
        } else {
            TRAVERSE_EDGE(x0, y0, x2, y2, dx02, right_edge_buffer);
            TRAVERSE_EDGE(x1, y1, x2, y2, dx12, left_edge_buffer);
        }
    } else if (y1 == y2) {
        // Flat bottom
        int32_t dx01 = idiv32((x1 - x0) << (DDA_BITS + SUBPIXEL_BITS), y1 - y0);
        int32_t dx02 = idiv32((x2 - x0) << (DDA_BITS + SUBPIXEL_BITS), y2 - y0);
        if (x1 < x2) {
            TRAVERSE_EDGE(x0, y0, x1, y1, dx01, left_edge_buffer);
            TRAVERSE_EDGE(x0, y0, x2, y2, dx02, right_edge_buffer);
        } else {
            TRAVERSE_EDGE(x0, y0, x1, y1, dx01, right_edge_buffer);
            TRAVERSE_EDGE(x0, y0, x2, y2, dx02, left_edge_buffer);
        }
    } else {
        int32_t dx01 = idiv32((x1 - x0) << (DDA_BITS + SUBPIXEL_BITS), y1 - y0);
        int32_t dx12 = idiv32((x2 - x1) << (DDA_BITS + SUBPIXEL_BITS), y2 - y1);
        int32_t dx02 = idiv32((x2 - x0) << (DDA_BITS + SUBPIXEL_BITS), y2 - y0);
        if (dx01 < dx02) {
            TRAVERSE_EDGE(x0, y0, x1, y1, dx01, left_edge_buffer);
            TRAVERSE_EDGE(x1, y1, x2, y2, dx12, left_edge_buffer);
            TRAVERSE_EDGE(x0, y0, x2, y2, dx02, right_edge_buffer);
        } else {
            TRAVERSE_EDGE(x0, y0, x1, y1, dx01, right_edge_buffer);
            TRAVERSE_EDGE(x1, y1, x2, y2, dx12, right_edge_buffer);
            TRAVERSE_EDGE(x0, y0, x2, y2, dx02, left_edge_buffer);
        }
    }

    uint8_t *screen_row = screen + mul_by_screen_stride(min_y);

    int32_t y = min_y;
    while (y < max_y) {
        assert(y < SCREEN_HEIGHT);
        int32_t left = max32(
            (left_edge_buffer[y] + (SUBPIXEL_MASK << DDA_BITS))
            >> (SUBPIXEL_BITS + DDA_BITS), 0);
        int32_t right = min32(
            (right_edge_buffer[y] + (SUBPIXEL_MASK << DDA_BITS))
            >> (SUBPIXEL_BITS + DDA_BITS), SCREEN_WIDTH);
        for (int32_t x = left; x < right; x++) {
            screen_row[x] = color;
        }

        screen_row += SCREEN_WIDTH;
        y++;
    }
}

#undef DDA_BITS
#undef TRAVERSE_EDGE
