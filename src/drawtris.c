#include "drawtris.h"
#include "mymath.h"
#include "minmax.h"
#include "screen.h"
#include "util.h"
#include "tridefs.h"

#include <assert.h>
#include <stddef.h>

//

#define BLOCK_SIZE_SHIFT 1
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 1

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_2_aligned
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 1
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 0

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_2
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 2
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 1

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_4_aligned
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 2
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 0

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_4
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 3
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 1

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_8_aligned
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 3
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 0

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_8
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 4
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 1

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_16_aligned
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define BLOCK_SIZE_SHIFT 4
#define BLOCK_SIZE       (1 << BLOCK_SIZE_SHIFT)
#define BLOCK_MASK       (BLOCK_SIZE - 1)

#define ALIGNED_FILL 0

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_16
#include "tritmpl.h"

#undef BLOCK_SIZE_SHIFT
#undef BLOCK_SIZE
#undef BLOCK_MASK
#undef ALIGNED_FILL
#undef DRAW_TRIANGLE_FUNC_NAME

//

#define DRAW_TRIANGLE_FUNC_NAME draw_triangle_no_blocks
#include "tritmpl2.h"

#undef DRAW_TRIANGLE_FUNC_NAME

//

typedef void (*draw_triangle_func)(int32_t x0, int32_t y0, int32_t x1, int32_t y1,
    int32_t x2, int32_t y2, uint8_t color, uint8_t *screen);

static const draw_triangle_func draw_triangle_funcs[] = {
    draw_triangle_2_aligned,
    draw_triangle_2,
    draw_triangle_4_aligned,
    draw_triangle_4,
    draw_triangle_8_aligned,
    draw_triangle_8,
    draw_triangle_16_aligned,
    draw_triangle_16,
    draw_triangle_no_blocks,
};

static const char *const triangle_func_names[] = {
    "2a ",
    "2  ",
    "4a ",
    "4  ",
    "8a ",
    "8  ",
    "16a",
    "16 ",
    "nb ",
};

const char *get_triangle_func_name(uint8_t index) {
    assert(index < TRIANGLE_FUNC_COUNT);
    return triangle_func_names[index];
}

void draw_triangles(const triangle_t *triangles, const uint8_t *colors,
    uint32_t num, uint8_t func_index, uint8_t *screen) {
    assert(triangles && colors && screen);
    draw_triangle_func func = draw_triangle_funcs[func_index];
    for (uint32_t i = 0; i < num; i++) {
        const triangle_t *t = &triangles[i];
        func(t->x[0], t->y[0], t->x[1], t->y[1], t->x[2], t->y[2], colors[i], screen);
    }
}

void draw_triangles_sorted(const triangle_t *triangles, const uint8_t *colors,
    const uint32_t *sort_buffer, uint32_t num, uint8_t func_index,
    uint8_t *screen) {
    assert(triangles && colors && sort_buffer && screen);
    draw_triangle_func func = draw_triangle_funcs[func_index];
    for (uint32_t i = 0; i < num; ++i) {
        uint32_t j = sort_buffer[i] >> 16;
        assert(j < num);
        j <<= 3;
        const triangle_t *t = &triangles[j];
        func(t->x[0], t->y[0], t->x[1], t->y[1], t->x[2], t->y[2], colors[j], screen);
    }
}
