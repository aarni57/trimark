#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_init();
void vga_cleanup();
int vga_is_initialized();

void vga_mode(uint8_t mode);
void vga_set_refresh_rate(uint8_t option);

void vga_palette(uint8_t i, uint8_t r, uint8_t g, uint8_t b);
void vga_palette_from_data(const uint8_t *data, uint32_t offset, uint32_t num_colors);

void vga_update(const uint8_t *pixels, uint8_t vsync);

#endif
