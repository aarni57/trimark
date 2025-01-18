#include "demo.h"
#include "triangle.h"

int demo_init() {
    return 1;
}

void demo_cleanup() {
}

void demo_update() {
}

void demo_render(uint8_t *screen) {
    int16_t draw_buffer[32];
    draw_buffer[0 * 8 + 0] = 200 * SUBPIXEL_ONE;
    draw_buffer[0 * 8 + 1] = 10 * SUBPIXEL_ONE;
    draw_buffer[0 * 8 + 2] = -30 * SUBPIXEL_ONE;
    draw_buffer[0 * 8 + 3] = 110 * SUBPIXEL_ONE;
    draw_buffer[0 * 8 + 4] = 350 * SUBPIXEL_ONE;
    draw_buffer[0 * 8 + 5] = 190 * SUBPIXEL_ONE;
    draw_buffer[0 * 8 + 6] = 192;
    draw_buffer[0 * 8 + 7] = 0;

    uint32_t sort_buffer[1];
    sort_buffer[0] = 0;

    draw_triangles(draw_buffer, sort_buffer, 1, screen);
}
