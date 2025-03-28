#include "chip8.h"
#include "util.h"

static const uint8_t FONT_DATA[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static void push(struct chip8 *chip8, uint16_t value)
{
    if (chip8->sp >= STACK_SIZE) {
        die("Stack overflow");
    }
    chip8->stack[chip8->sp++] = value;
}

static uint16_t pop(struct chip8 *chip8)
{
    if (chip8->sp == 0) {
        die("Stack underflow");
    }
    return chip8->stack[--chip8->sp];
}

static void skip_instr(struct chip8 *chip8)
{
    chip8->pc += 2;
}

static void fetch_execute(struct chip8 *chip8)
{

    const uint8_t msb = chip8->memory[chip8->pc++];
    const uint8_t lsb = chip8->memory[chip8->pc++];
    const uint16_t opcode = (msb << 8) | lsb;

    const uint8_t x = (opcode & 0x0F00) >> 8;
    const uint8_t y = (opcode & 0x00F0) >> 4;
    const uint8_t n = opcode & 0x000F;
    const uint8_t nn = opcode & 0x00FF;
    const uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {
    case 0x0000: {
        switch (nn) {
        // CLS — 00E0
        case 0xE0: {
            memset(&chip8->display, 0, DISPLAY_SIZE);
            break;
        }
        // RET — 00EE
        case 0xEE: {
            chip8->pc = pop(chip8);
            break;
        }
        }
        break;
    }
    // JMP — 1NNN
    case 0x1000: {
        chip8->pc = nnn;
        break;
    }
    // CALL NNN — 2NNN
    case 0x2000: {
        push(chip8, chip8->pc);
        chip8->pc = nnn;
        break;
    }
    // SE VX, NN — 3XNN
    case 0x3000: {
        if (chip8->v[x] == nn) {
            skip_instr(chip8);
        }
        break;
    }
    // SNE VX, NN — 4XNN
    case 0x4000: {
        if (chip8->v[x] != nn) {
            skip_instr(chip8);
        }
        break;
    }
    // SE VX, VY — 5XY0
    case 0x5000: {
        if (chip8->v[x] == chip8->v[y]) {
            skip_instr(chip8);
        }
        break;
    }
    // LD VX, NN — 6XNN
    case 0x6000: {
        chip8->v[x] = nn;
        break;
    }
    // ADD VX, NN — 7XNN
    case 0x7000: {
        chip8->v[x] += nn;
        break;
    }
    case 0x8000: {
        switch (n) {
        // LD VX, VY — 8XY0
        case 0x0: {
            chip8->v[x] = chip8->v[y];
            break;
        }
        // OR VX, VY — 8XY1
        case 0x1: {
            chip8->v[x] |= chip8->v[y];
            break;
        }
        // AND VX, VY — 8XY2
        case 0x2: {
            chip8->v[x] &= chip8->v[y];
            break;
        }
        // XOR VX, VY — 8XY3
        case 0x3: {
            chip8->v[x] ^= chip8->v[y];
            break;
        }
        // ADD VX, VY — 8XY4
        case 0x4: {
            const int result = chip8->v[x] + chip8->v[y];
            chip8->v[x] = result & 0xFF;
            chip8->v[0xF] = result > UINT8_MAX;
            break;
        }
        // SUB VX, VY — 8XY5
        case 0x5: {
            const int result = chip8->v[x] - chip8->v[y];
            chip8->v[x] = result & 0xFF;
            chip8->v[0xF] = result > 0;
            break;
        }
        // SHR VX {, VY} — 8XY6
        case 0x6: {
            const uint8_t lsb = chip8->v[x] & 1;
            chip8->v[x] >>= 1;
            chip8->v[0xF] = lsb;
            break;
        }
        // SUBN VX, VY — 8XY7
        case 0x7: {
            const int result = chip8->v[y] - chip8->v[x];
            chip8->v[x] = result & 0xFF;
            chip8->v[0xF] = result > 0;
            break;
        }
        // SHL VX {, VY} — 8XYE
        case 0xE: {
            const uint8_t msb = (chip8->v[x] >> 7) & 1;
            chip8->v[x] <<= 1;
            chip8->v[0xF] = msb;
            break;
        }
        }
        break;
    }
    // SNE VX, VY — 9XY0
    case 0x9000: {
        if (chip8->v[x] != chip8->v[y]) {
            skip_instr(chip8);
        }
        break;
    }
    // LD I, NNN — ANNN
    case 0xA000: {
        chip8->i = nnn;
        break;
    }
    // JMP V0, NNN — BNNN
    case 0xB000: {
        chip8->pc = nnn + chip8->v[0];
        break;
    }
    // RND VX, NN – CXNN
    case 0xC000: {
        chip8->v[x] = (rand() % UINT8_MAX) & nn;
        break;
    }
    // DRW VX, VY, N — DXYN
    case 0xD000: {
        const uint8_t x_pos = chip8->v[x] % DISPLAY_WIDTH;
        const uint8_t y_pos = chip8->v[y] % DISPLAY_HEIGHT;
        uint16_t ireg = chip8->i;
        chip8->v[0xF] = 0;

        for (uint8_t dy = 0; dy < n && ireg + dy < MEMORY_SIZE; ++dy) {
            const uint8_t sprite_data = chip8->memory[ireg + dy];
            const uint16_t row_offset =
                ((y_pos + dy) % DISPLAY_HEIGHT) * DISPLAY_WIDTH;

            for (uint8_t dx = 0; dx < 8; ++dx) {
                if (!(sprite_data & (0x80 >> dx))) {
                    continue;
                }
                const uint16_t pos =
                    row_offset + ((x_pos + dx) % DISPLAY_WIDTH);
                chip8->v[0xF] |= chip8->display[pos];
                chip8->display[pos] ^= 1;
            }
        }
        break;
    }
    case 0xE000: {
        switch (nn) {
        // SKP VX — EX9E
        case 0x9E: {
            if (chip8->keys[chip8->v[x]]) {
                skip_instr(chip8);
            }
            break;
        }
        // SKNP VX — EXA1
        case 0xA1: {
            if (!chip8->keys[chip8->v[x]]) {
                skip_instr(chip8);
            }
            break;
        }
        }
        break;
    }
    case 0xF000: {
        switch (nn) {
        // LD VX, DT — FX07
        case 0x07: {
            chip8->v[x] = chip8->dt;
            break;
        }
        // LD VX, K — FX0A
        case 0x0A: {
            bool key_pressed = false;
            for (size_t i = 0; i < ARRAY_SIZE(chip8->keys); ++i) {
                if (chip8->keys[i]) {
                    key_pressed = true;
                    break;
                }
            }
            if (!key_pressed) {
                chip8->pc -= 2;
            }
            break;
        }
        // LD DT, VX — FX15
        case 0x15: {
            chip8->dt = chip8->v[x];
            break;
        }
        // LD ST, VX — FX18
        case 0x18: {
            chip8->st = chip8->v[x];
            break;
        }
        // ADD I, VX — FX1E
        case 0x1E: {
            chip8->i += chip8->v[x];
            break;
        }
        // LD F, VX — FX29
        case 0x29: {
            chip8->i = chip8->v[x];
            break;
        }
        // LD B, VX — FX33
        case 0x33: {
            const uint8_t vx = chip8->v[x];
            const uint8_t hundreds = vx / 100;
            const uint8_t tens = (vx - hundreds * 100) / 10;
            const uint8_t ones = vx - hundreds * 100 - tens * 10;
            chip8->memory[chip8->i + 0] = hundreds;
            chip8->memory[chip8->i + 1] = tens;
            chip8->memory[chip8->i + 2] = ones;
            break;
        }
        // LD [I], VX — FX55
        case 0x55: {
            memcpy(&chip8->memory[chip8->i], chip8->v, x + 1);
            break;
        }
        // LD VX, [I] — FX65
        case 0x65: {
            memcpy(chip8->v, &chip8->memory[chip8->i], x + 1);
            break;
        }
        }
        break;
    }
    default: {
        die("Unknown opcode: 0x%X", opcode);
    }
    }
}

void chip8_init(struct chip8 *chip8, const uint8_t *rom_data, size_t rom_size)
{
    chip8->pc = 0x200;
    memset(chip8->memory, 0, MEMORY_SIZE);
    memcpy(chip8->memory, FONT_DATA, sizeof(FONT_DATA));
    memcpy(chip8->memory + chip8->pc, rom_data, rom_size);
}

void chip8_step(struct chip8 *chip8)
{
    fetch_execute(chip8);
    if (chip8->dt > 0) {
        chip8->dt--;
    }
    if (chip8->st > 0) {
        // to-do: play sound
        chip8->st--;
    }
}

void chip8_set_key(struct chip8 *chip8, SDL_Scancode scancode, bool pressed)
{
    static const int keymap[SDL_SCANCODE_COUNT] = {
        [SDL_SCANCODE_1] = 0x1, [SDL_SCANCODE_2] = 0x2, [SDL_SCANCODE_3] = 0x3,
        [SDL_SCANCODE_4] = 0xC, [SDL_SCANCODE_Q] = 0x4, [SDL_SCANCODE_W] = 0x5,
        [SDL_SCANCODE_E] = 0x6, [SDL_SCANCODE_R] = 0xD, [SDL_SCANCODE_A] = 0x7,
        [SDL_SCANCODE_S] = 0x8, [SDL_SCANCODE_D] = 0x9, [SDL_SCANCODE_F] = 0xE,
        [SDL_SCANCODE_Z] = 0xA, [SDL_SCANCODE_X] = 0x0, [SDL_SCANCODE_C] = 0xB,
        [SDL_SCANCODE_V] = 0xF};

    if (scancode < SDL_SCANCODE_COUNT && keymap[scancode]) {
        chip8->keys[keymap[scancode]] = pressed;
    }
}
