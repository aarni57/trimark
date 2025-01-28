#include "trimark.h"
#include "screen.h"

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//

static SDL_Window *my_sdl_window = NULL;
static SDL_Renderer *my_sdl_renderer = NULL;
static SDL_Texture *my_sdl_screen_texture = NULL;

static uint32_t *screen_buffer32 = NULL;
static uint8_t *screen_buffer8 = NULL;

//

static int mouse_captured = 1;

static void update_mouse_capture() {
    SDL_ShowCursor(!mouse_captured);
    SDL_SetRelativeMouseMode(mouse_captured);
}

static int window_fullscreen = 0;

static void update_window_fullscreen() {
    SDL_SetWindowFullscreen(my_sdl_window, window_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

static int init() {
    screen_buffer32 = malloc(sizeof(*screen_buffer32) * SCREEN_NUM_PIXELS);
    screen_buffer8 = malloc(sizeof(*screen_buffer8) * SCREEN_NUM_PIXELS);
    memset(screen_buffer8, 0, sizeof(*screen_buffer8) * SCREEN_NUM_PIXELS);

    if (!trimark_init())
        return 0;

    return 1;
}

static void cleanup() {
    trimark_cleanup();

    free(screen_buffer8); screen_buffer8 = NULL;
    free(screen_buffer32); screen_buffer32 = NULL;

    if (my_sdl_screen_texture) SDL_DestroyTexture(my_sdl_screen_texture);
    if (my_sdl_renderer) SDL_DestroyRenderer(my_sdl_renderer);
    if (my_sdl_window) SDL_DestroyWindow(my_sdl_window);

    SDL_Quit();
}

//

typedef struct palette_color_t {
    uint8_t r, g, b;
} palette_color_t;

static palette_color_t vga_palette[NUM_COLORS] = {
    // Default VGA palette taken from Allegro 4 sources
    { 0,  0,  0 },  { 0,  0,  42 }, { 0,  42, 0 },  { 0,  42, 42 },
    { 42, 0,  0 },  { 42, 0,  42 }, { 42, 21, 0 },  { 42, 42, 42 },
    { 21, 21, 21 }, { 21, 21, 63 }, { 21, 63, 21 }, { 21, 63, 63 },
    { 63, 21, 21 }, { 63, 21, 63 }, { 63, 63, 21 }, { 63, 63, 63 },
    { 0,  0,  0 },  { 5,  5,  5 },  { 8,  8,  8 },  { 11, 11, 11 },
    { 14, 14, 14 }, { 17, 17, 17 }, { 20, 20, 20 }, { 24, 24, 24 },
    { 28, 28, 28 }, { 32, 32, 32 }, { 36, 36, 36 }, { 40, 40, 40 },
    { 45, 45, 45 }, { 50, 50, 50 }, { 56, 56, 56 }, { 63, 63, 63 },
    { 0,  0,  63 }, { 16, 0,  63 }, { 31, 0,  63 }, { 47, 0,  63 },
    { 63, 0,  63 }, { 63, 0,  47 }, { 63, 0,  31 }, { 63, 0,  16 },
    { 63, 0,  0 },  { 63, 16, 0 },  { 63, 31, 0 },  { 63, 47, 0 },
    { 63, 63, 0 },  { 47, 63, 0 },  { 31, 63, 0 },  { 16, 63, 0 },
    { 0,  63, 0 },  { 0,  63, 16 }, { 0,  63, 31 }, { 0,  63, 47 },
    { 0,  63, 63 }, { 0,  47, 63 }, { 0,  31, 63 }, { 0,  16, 63 },
    { 31, 31, 63 }, { 39, 31, 63 }, { 47, 31, 63 }, { 55, 31, 63 },
    { 63, 31, 63 }, { 63, 31, 55 }, { 63, 31, 47 }, { 63, 31, 39 },
    { 63, 31, 31 }, { 63, 39, 31 }, { 63, 47, 31 }, { 63, 55, 31 },
    { 63, 63, 31 }, { 55, 63, 31 }, { 47, 63, 31 }, { 39, 63, 31 },
    { 31, 63, 31 }, { 31, 63, 39 }, { 31, 63, 47 }, { 31, 63, 55 },
    { 31, 63, 63 }, { 31, 55, 63 }, { 31, 47, 63 }, { 31, 39, 63 },
    { 45, 45, 63 }, { 49, 45, 63 }, { 54, 45, 63 }, { 58, 45, 63 },
    { 63, 45, 63 }, { 63, 45, 58 }, { 63, 45, 54 }, { 63, 45, 49 },
    { 63, 45, 45 }, { 63, 49, 45 }, { 63, 54, 45 }, { 63, 58, 45 },
    { 63, 63, 45 }, { 58, 63, 45 }, { 54, 63, 45 }, { 49, 63, 45 },
    { 45, 63, 45 }, { 45, 63, 49 }, { 45, 63, 54 }, { 45, 63, 58 },
    { 45, 63, 63 }, { 45, 58, 63 }, { 45, 54, 63 }, { 45, 49, 63 },
    { 0,  0,  28 }, { 7,  0,  28 }, { 14, 0,  28 }, { 21, 0,  28 },
    { 28, 0,  28 }, { 28, 0,  21 }, { 28, 0,  14 }, { 28, 0,  7 },
    { 28, 0,  0 },  { 28, 7,  0 },  { 28, 14, 0 },  { 28, 21, 0 },
    { 28, 28, 0 },  { 21, 28, 0 },  { 14, 28, 0 },  { 7,  28, 0 },
    { 0,  28, 0 },  { 0,  28, 7 },  { 0,  28, 14 }, { 0,  28, 21 },
    { 0,  28, 28 }, { 0,  21, 28 }, { 0,  14, 28 }, { 0,  7,  28 },
    { 14, 14, 28 }, { 17, 14, 28 }, { 21, 14, 28 }, { 24, 14, 28 },
    { 28, 14, 28 }, { 28, 14, 24 }, { 28, 14, 21 }, { 28, 14, 17 },
    { 28, 14, 14 }, { 28, 17, 14 }, { 28, 21, 14 }, { 28, 24, 14 },
    { 28, 28, 14 }, { 24, 28, 14 }, { 21, 28, 14 }, { 17, 28, 14 },
    { 14, 28, 14 }, { 14, 28, 17 }, { 14, 28, 21 }, { 14, 28, 24 },
    { 14, 28, 28 }, { 14, 24, 28 }, { 14, 21, 28 }, { 14, 17, 28 },
    { 20, 20, 28 }, { 22, 20, 28 }, { 24, 20, 28 }, { 26, 20, 28 },
    { 28, 20, 28 }, { 28, 20, 26 }, { 28, 20, 24 }, { 28, 20, 22 },
    { 28, 20, 20 }, { 28, 22, 20 }, { 28, 24, 20 }, { 28, 26, 20 },
    { 28, 28, 20 }, { 26, 28, 20 }, { 24, 28, 20 }, { 22, 28, 20 },
    { 20, 28, 20 }, { 20, 28, 22 }, { 20, 28, 24 }, { 20, 28, 26 },
    { 20, 28, 28 }, { 20, 26, 28 }, { 20, 24, 28 }, { 20, 22, 28 },
    { 0,  0,  16 }, { 4,  0,  16 }, { 8,  0,  16 }, { 12, 0,  16 },
    { 16, 0,  16 }, { 16, 0,  12 }, { 16, 0,  8 },  { 16, 0,  4 },
    { 16, 0,  0 },  { 16, 4,  0 },  { 16, 8,  0 },  { 16, 12, 0 },
    { 16, 16, 0 },  { 12, 16, 0 },  { 8,  16, 0 },  { 4,  16, 0 },
    { 0,  16, 0 },  { 0,  16, 4 },  { 0,  16, 8 },  { 0,  16, 12 },
    { 0,  16, 16 }, { 0,  12, 16 }, { 0,  8,  16 }, { 0,  4,  16 },
    { 8,  8,  16 }, { 10, 8,  16 }, { 12, 8,  16 }, { 14, 8,  16 },
    { 16, 8,  16 }, { 16, 8,  14 }, { 16, 8,  12 }, { 16, 8,  10 },
    { 16, 8,  8 },  { 16, 10, 8 },  { 16, 12, 8 },  { 16, 14, 8 },
    { 16, 16, 8 },  { 14, 16, 8 },  { 12, 16, 8 },  { 10, 16, 8 },
    { 8,  16, 8 },  { 8,  16, 10 }, { 8,  16, 12 }, { 8,  16, 14 },
    { 8,  16, 16 }, { 8,  14, 16 }, { 8,  12, 16 }, { 8,  10, 16 },
    { 11, 11, 16 }, { 12, 11, 16 }, { 13, 11, 16 }, { 15, 11, 16 },
    { 16, 11, 16 }, { 16, 11, 15 }, { 16, 11, 13 }, { 16, 11, 12 },
    { 16, 11, 11 }, { 16, 12, 11 }, { 16, 13, 11 }, { 16, 15, 11 },
    { 16, 16, 11 }, { 15, 16, 11 }, { 13, 16, 11 }, { 12, 16, 11 },
    { 11, 16, 11 }, { 11, 16, 12 }, { 11, 16, 13 }, { 11, 16, 15 },
    { 11, 16, 16 }, { 11, 15, 16 }, { 11, 13, 16 }, { 11, 12, 16 },
    { 0,  0,  0 },  { 0,  0,  0 },  { 0,  0,  0 },  { 0,  0,  0 },
    { 0,  0,  0 },  { 0,  0,  0 },  { 0,  0,  0 },  { 63, 63, 63 }
};

static void convert_screen_buffer() {
    const uint8_t *src = screen_buffer8;
    uint32_t *tgt = screen_buffer32;
    for (uint32_t i = 0; i < SCREEN_NUM_PIXELS; ++i) {
        uint8_t index = *src++;
        const palette_color_t *palette_color = &vga_palette[index];
        uint8_t r = (palette_color->r << 2) | (palette_color->r >> 4);
        uint8_t g = (palette_color->g << 2) | (palette_color->g >> 4);
        uint8_t b = (palette_color->b << 2) | (palette_color->b >> 4);
        *tgt++ = (r << 24) | (g << 16) | (b << 8);
    }
}

#define SCREEN_LOGICAL_WIDTH SCREEN_WIDTH
#define SCREEN_LOGICAL_HEIGHT (SCREEN_HEIGHT * 6 / 5)

#define WINDOW_SCALE 2
#define WINDOW_WIDTH (SCREEN_LOGICAL_WIDTH * WINDOW_SCALE)
#define WINDOW_HEIGHT (SCREEN_LOGICAL_HEIGHT * WINDOW_SCALE)

int main() {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        return EXIT_FAILURE;
    }

    my_sdl_window = SDL_CreateWindow("retrofiller demo",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (my_sdl_window == NULL) {
        cleanup();
        return EXIT_FAILURE;
    }

    SDL_SetWindowMinimumSize(my_sdl_window, SCREEN_WIDTH, SCREEN_HEIGHT);

    my_sdl_renderer = SDL_CreateRenderer(my_sdl_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (my_sdl_renderer == NULL) {
        cleanup();
        return EXIT_FAILURE;
    }

#if 0
    SDL_RendererInfo *renderer_info =
        (SDL_RendererInfo*)malloc(sizeof(SDL_RendererInfo));
    SDL_GetRendererInfo(my_sdl_renderer, renderer_info);
    printf("SDL Renderer: %s\n", renderer_info->name);
#endif

    SDL_RenderSetLogicalSize(my_sdl_renderer, SCREEN_LOGICAL_WIDTH, SCREEN_LOGICAL_HEIGHT);
    SDL_SetRenderDrawColor(my_sdl_renderer,
        32, 32, 32, 255);

    my_sdl_screen_texture = SDL_CreateTexture(
        my_sdl_renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);

    if (!init()) {
        cleanup();
        return EXIT_FAILURE;
    }

    trimark_run();

    int quit = 0;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    if (event.type == SDL_KEYDOWN) {
                        switch (event.key.keysym.sym) {
                            case SDLK_F9:
                                window_fullscreen ^= 1;
                                update_window_fullscreen();
                                break;

                            case SDLK_F10:
                                mouse_captured ^= 1;
                                update_mouse_capture();
                                break;

                            default:
                                break;
                        }
                    }

                    break;
                }

                case SDL_MOUSEMOTION: {
                    if (mouse_captured) {
                        // event.motion.xrel;
                        // event.motion.yrel;
                    }

                    break;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    if (mouse_captured) {
                        // event.button.button
                    }

                    break;
                }

                case SDL_MOUSEBUTTONUP: {
                    if (mouse_captured) {
                        // event.button.button
                    }

                    break;
                }

                case SDL_QUIT:
                    quit = 1;
                    break;

                default:
                    break;
            }
        }

        trimark_update();
        trimark_blit(screen_buffer8);

        convert_screen_buffer();

        SDL_UpdateTexture(my_sdl_screen_texture, NULL, screen_buffer32,
            SCREEN_WIDTH * sizeof(*screen_buffer32));

        SDL_RenderClear(my_sdl_renderer);
        SDL_RenderCopy(my_sdl_renderer, my_sdl_screen_texture, NULL, NULL);
        SDL_RenderPresent(my_sdl_renderer);
    }

    trimark_print_results();
    cleanup();

    return EXIT_SUCCESS;
}
