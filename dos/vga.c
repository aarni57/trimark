#include "vga.h"
#include "screen.h"
#include "dosutil.h"

#include <dos.h>
#include <dpmi.h>
#include <string.h>
#include <sys/nearptr.h>
#include <assert.h>
#include <stdlib.h>

//

#define VGA_ADDRESS 0xa0000

#define VGA_INT 0x10

#define VGA_PALETTE_INDEX 0x3c8
#define VGA_PALETTE_DATA 0x3c9
#define VGA_INPUT_STATUS 0x3da

#define VGA_VRETRACE 0x08

//

static struct {
    uint16_t crtc_index; // 0x3d4 or 0x3b4
    uint16_t crtc_data;  // 0x3d5 or 0x3b5
    uint8_t mode;
    uint8_t refresh_rate_option; // 0 = 60Hz, 1 = 70Hz
} vga_state = {
    .crtc_index = 0,
    .crtc_data = 0,
    .mode = 0x3,
    .refresh_rate_option = 1
};

static uint8_t *get_vga_address() {
    return (uint8_t*)__djgpp_conventional_base + VGA_ADDRESS;
}

static void set_60hz() {
    /* setup the VGA: 320x200 @ 60Hz */
    /* from: https://gist.github.com/juj/34306e6da02a8a7043e393f01e013f24 */

    outportw(vga_state.crtc_index, 0x0011); /* Turn off write protect to CRTC registers */
    outportw(vga_state.crtc_index, 0x0b06); /* New vertical total=525 lines, bits 0-7 */
    outportw(vga_state.crtc_index, 0x3e07); /* New vertical total=525 lines, bits 8-9 */

#if 0
    outportw(vga_state.crtc_index, 0xb910); /* Vsync start=scanline 185 */
    outportw(vga_state.crtc_index, 0x8f12); /* Vertical display end=scanline 399, bits 0-7 */
    outportw(vga_state.crtc_index, 0xb815); /* Vertical blanking start=scanline 440, bits 0-7 */
    outportw(vga_state.crtc_index, 0xe216); /* Adjust vblank end position */
    outportw(vga_state.crtc_index, 0x8511); /* Vsync length=2 lines + turn write protect back on */
#else
    outportw(vga_state.crtc_index, 0x0b16); // Adjust vblank end position=scanline 524
    outportw(vga_state.crtc_index, 0xd710); // Vsync start=scanline 215
    outportw(vga_state.crtc_index, 0x8911); // Vsync length=2 lines + turn write protect back on
#endif

    outportw(vga_state.crtc_index, 0x2813); /* 8 pixel chars */
    outportw(vga_state.crtc_index, 0x8e11); /* restore retrace */
}

void vga_init() {
    assert(!vga_is_initialized());
    if (inportb(0x3cc) & 1) {
        vga_state.crtc_index = 0x3d4;
        vga_state.crtc_data = 0x3d5;
    } else {
        vga_state.crtc_index = 0x3b4;
        vga_state.crtc_data = 0x3b5;
    }
}

void vga_cleanup() {
    assert(vga_is_initialized());
    vga_state.crtc_index = 0;
}

int vga_is_initialized() {
    return vga_state.crtc_index != 0;
}

void vga_mode(uint8_t mode) {
    assert(vga_is_initialized());
    vga_state.mode = mode;
    vga_state.refresh_rate_option = 1;

    __dpmi_regs r;
    memset(&r, 0, sizeof(r));
    r.x.ax = mode;
    __dpmi_int(VGA_INT, &r);
}

void vga_set_refresh_rate(uint8_t option) {
    if (vga_state.mode == 0x13 && option != vga_state.refresh_rate_option) {
        disable_interrupts();
        vga_mode(0x13); // Reset

        vga_state.refresh_rate_option = option;
        if (vga_state.refresh_rate_option == 0) {
            set_60hz();
        }

        enable_interrupts();
    }
}

void vga_palette(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {
    assert(vga_is_initialized());
    assert(vga_state.mode == 0x13);
    outportb(VGA_PALETTE_INDEX, i);
    outportb(VGA_PALETTE_DATA, r);
    outportb(VGA_PALETTE_DATA, g);
    outportb(VGA_PALETTE_DATA, b);
}

void vga_palette_from_data(const uint8_t *data, uint32_t offset, uint32_t num_colors) {
    assert(data);
    assert(offset + num_colors <= NUM_COLORS);
    const uint8_t *src = data;
    for (uint32_t i = offset; i < offset + num_colors; ++i) {
        uint8_t r = *src++;
        uint8_t g = *src++;
        uint8_t b = *src++;
        vga_palette((uint8_t)i, r, g, b);
    }
}

void vga_update(const uint8_t *pixels, uint8_t vsync) {
    assert(vga_is_initialized());
    assert(vga_state.mode == 0x13);
    assert(pixels);

    if (vsync) {
        while (inportb(VGA_INPUT_STATUS) & VGA_VRETRACE);
        while (!(inportb(VGA_INPUT_STATUS) & VGA_VRETRACE));
    }

    memcpy(get_vga_address(), pixels, SCREEN_NUM_PIXELS);
}
