#include "demo.h"
#include "screen.h"

#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//

#define WINDOW_SCALE 4

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

    if (!demo_init())
        return 0;

    return 1;
}

static void cleanup() {
    demo_cleanup();

    free(screen_buffer8); screen_buffer8 = NULL;
    free(screen_buffer32); screen_buffer32 = NULL;

    if (my_sdl_screen_texture) SDL_DestroyTexture(my_sdl_screen_texture);
    if (my_sdl_renderer) SDL_DestroyRenderer(my_sdl_renderer);
    if (my_sdl_window) SDL_DestroyWindow(my_sdl_window);

    SDL_Quit();
}

static void convert_screen_buffer() {
    const uint8_t *src = screen_buffer8;
    uint32_t *tgt = screen_buffer32;
    for (uint32_t i = 0; i < SCREEN_NUM_PIXELS; ++i) {
        uint8_t index = *src++;
        uint8_t r = index;
        uint8_t g = index;
        uint8_t b = index;
        *tgt++ = (r << 24) | (g << 16) | (b << 8);
    }
}

int main() {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        return EXIT_FAILURE;
    }

    my_sdl_window = SDL_CreateWindow("retrofiller demo",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
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

    SDL_RenderSetLogicalSize(my_sdl_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
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

        demo_update();
        demo_render(screen_buffer8);

        convert_screen_buffer();

        SDL_UpdateTexture(my_sdl_screen_texture, NULL, screen_buffer32,
            SCREEN_WIDTH * sizeof(*screen_buffer32));

        SDL_RenderClear(my_sdl_renderer);
        SDL_RenderCopy(my_sdl_renderer, my_sdl_screen_texture, NULL, NULL);
        SDL_RenderPresent(my_sdl_renderer);
    }

    cleanup();

    return EXIT_SUCCESS;
}
