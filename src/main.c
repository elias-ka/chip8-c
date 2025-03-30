#include "chip8.h"
#include <SDL3/SDL.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

int main(int argc, const char **argv)
{
    srand(time(NULL));

    if (argc < 2) {
        die("Usage: %s <program>\n", argv[0]);
    }

    const char *prog_path = argv[1];
    FILE *file = fopen(prog_path, "rb");
    if (!file) {
        die("%s", strerror(errno));
    }

    fseek(file, 0, SEEK_END);
    const size_t prog_size = (size_t)ftell(file);
    rewind(file);

    if (prog_size > MEMORY_SIZE) {
        die("Program exceeds the maximum size of 4096 bytes.");
    }

    uint8_t *prog_data = xcalloc(prog_size, 1);
    const uint32_t bytes_read = fread(prog_data, 1, prog_size, file);
    if (bytes_read < prog_size || ferror(file)) {
        die("Could not read file");
    }
    fclose(file);

    struct chip8 chip8 = {0};
    chip8_init(&chip8, prog_data, prog_size);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        die("SDL_Init: %s", SDL_GetError());
    }

    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("CHIP-8", 640, 320, 0, &win, &renderer)) {
        die("SDL_CreateWindowAndRenderer: %s", SDL_GetError());
    }
    SDL_SetRenderVSync(renderer, 1);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!texture) {
        die("SDL_CreateTexture: %s", SDL_GetError());
    }
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT: {
                quit = true;
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                chip8_set_key(&chip8, event.key.scancode,
                              event.type == SDL_EVENT_KEY_DOWN);
                break;
            }
            }
        }

        for (int i = 0; i < 15; ++i) {
            chip8_step(&chip8);
        }

        uint32_t pixels[DISPLAY_SIZE];
        for (int i = 0; i < DISPLAY_SIZE; i++) {
            pixels[i] = chip8.display[i] ? 0xFFFFFFFF : 0xFF000000;
        }
        SDL_UpdateTexture(texture, NULL, pixels,
                          DISPLAY_WIDTH * sizeof(uint32_t));
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    free(prog_data);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
