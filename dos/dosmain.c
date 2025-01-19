#include "bmark.h"
#include "vga.h"
#include "screen.h"

#include <sys/nearptr.h>
#include <dpmi.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>

static int nearptr_enabled = 0;
static int vga_initialized = 0;
static uint8_t *screen = NULL;

static int init() {
    if (!__djgpp_nearptr_enable())
        return 0;
    nearptr_enabled = 1;

    vga_init();
    vga_initialized = 1;

    screen = calloc(sizeof(*screen) * SCREEN_NUM_PIXELS, 1);

    if (!bmark_init())
        return 0;

    return 1;
}

static void cleanup() {
    bmark_cleanup();

    free(screen); screen = NULL;

    if (vga_initialized)
        vga_cleanup();

    if (nearptr_enabled)
        __djgpp_nearptr_disable();
}

int main(int argc, char* argv[]) {
    if (!init()) {
        cleanup();
        return 2;
    }

    vga_mode(0x13);

    while (!kbhit()) {
        bmark_update();
        bmark_render(screen);
        vga_update(screen, 1);
    }

    vga_mode(0x3);
    bmark_print_results();
    cleanup();

    return 0;
}
