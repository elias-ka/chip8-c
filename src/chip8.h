#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <SDL3/SDL_scancode.h>
#include <stddef.h>
#include <stdint.h>

enum {
    MEMORY_SIZE = 4096,
    STACK_SIZE = 16,
    REGISTER_COUNT = 16,
    KEY_COUNT = 16,
    DISPLAY_WIDTH = 64,
    DISPLAY_HEIGHT = 32,
    DISPLAY_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT,
};

struct chip8 {
    uint8_t memory[MEMORY_SIZE];
    uint16_t stack[STACK_SIZE];
    uint8_t v[REGISTER_COUNT];
    bool display[DISPLAY_SIZE];
    bool keys[KEY_COUNT];
    uint16_t pc, sp, i;
    uint8_t dt, st;
};

void chip8_init(struct chip8 *chip8, const uint8_t *rom_data, size_t rom_size);
void chip8_step(struct chip8 *chip8);
void chip8_set_key(struct chip8 *chip8, SDL_Scancode scancode, bool pressed);

#endif
