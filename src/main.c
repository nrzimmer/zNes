#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "raylib.h"

Font font;

void print_usage(const char *executable);
void draw_ram(const Bus *bus, int x, int y, uint16_t addr, int rows, int cols);
void draw_code(const Bus *bus, int x, int y, int lines);
void draw_cpu(const Bus *bus, int x, int y);
void draw_string(const char *text, int x, int y, int size, Color c);
void draw_sprite_info(const Bus *bus, int x, int y);

constexpr int FONTSIZE = 14;
const char *FONT_NAME = "/usr/share/fonts/Adwaita/AdwaitaMono-Bold.ttf";
constexpr Color BG_BLUE = {0x00, 0x00, 0x66, 0xFF};

disasm *array_asm;
Bus *main_bus;

bool handle_ui_input(int *scale, int *window_width, int *window_height, Cartridge **cart, int *debugger_x, int *pattern_y, int *nametable_y,
                     bool resize, bool *emulate) {
    if (IsKeyPressed(KEY_KP_ADD)) {
        if (*scale < 4) {
            (*scale)++;
            resize = true;
        }
    }

    if (IsKeyPressed(KEY_KP_SUBTRACT)) {
        if (*scale > 1) {
            (*scale)--;
            resize = true;
        }
    }

    if (resize) {
        resize = false;
        *window_width = 256 * (*scale + 1) + 12;
        *window_height = 240 * *scale;
        *debugger_x = 256 * *scale + 4;
        if (*scale == 1) {
            *pattern_y = 72;
        } else {
            *pattern_y = 340;
        }
        *nametable_y = *pattern_y + 12;

        SetWindowSize(*window_width, *window_height);
    }

    if (IsKeyPressed(KEY_P))
        *emulate = !*emulate;

    if (IsKeyPressed(KEY_R)) {
        bus_reset();

        // Clean pressed keys
        BeginDrawing();
        EndDrawing();
        return true;
    }

    return false;
}

void update_controller_input(Bus *bus) {
    bus->controller[0] = 0x00;
    bus->controller[0] |= IsKeyDown(KEY_X) | IsKeyDown(KEY_S) ? 0x80 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_Z) | IsKeyDown(KEY_A) ? 0x40 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_N) ? 0x20 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_M) ? 0x10 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_UP) ? 0x08 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_DOWN) ? 0x04 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_LEFT) ? 0x02 : 0x00;
    bus->controller[0] |= IsKeyDown(KEY_RIGHT) ? 0x01 : 0x00;
}

void AudioInputCallback(void *buffer, unsigned int frames) {
    short *d = buffer;
    for (unsigned int i = 0; i < frames; i++) {
        while (!bus_clock()) {
        }
        d[i] = (short)main_bus->dAudioSample;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    char *rom_file = argv[1];

    Cartridge *cart = cartridge_new(rom_file);
    if (cart == nullptr) {
        exit(1);
    }

    int scale = 3;

    int window_width = 256 * (scale + 1) + 12;
    int window_height = 240 * scale;
    const char *WINDOW_TITLE = "zEmu - NES";

    InitWindow(window_width, window_height, WINDOW_TITLE);

    if (!IsWindowReady()) {
        CloseWindow();
        return -1;
    }
    font = LoadFont(FONT_NAME);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    main_bus = bus_new();
    set_cart(cart);
    array_asm = disassemble(main_bus, 0x0000, 0xFFFF);
    bus_reset();

    int debugger_x = 256 * scale + 4;
    int pattern_y;
    int nametable_y;

    bool resize = true;
    bool emulate = true;

    SetTargetFPS(60);
    SetSampleFrequency(44100);

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(512);
    AudioStream stream = LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(stream, AudioInputCallback);
    PlayAudioStream(stream);

    while (!WindowShouldClose()) {
        if (handle_ui_input(&scale, &window_width, &window_height, &cart, &debugger_x, &pattern_y, &nametable_y, resize, &emulate))
            continue;

        update_controller_input(main_bus);

        if (main_bus->ppu->frame_complete) {
            main_bus->ppu->frame_complete = false;
            raylib_render_pattern_table(0, 0);
            raylib_render_pattern_table(1, 0);
            gen_screen_texture();

            BeginDrawing();
            ClearBackground(BG_BLUE);

            draw_cpu(main_bus, debugger_x, 2);
            if (scale > 1) {
                draw_code(main_bus, debugger_x, 72, 24);
                // draw_sprite_info(bus, debugger_x, 72);
            }

            const int nSwatchSize = 6;
            for (int p = 0; p < 8; p++)
                for (int s = 0; s < 4; s++)
                    DrawRectangle(debugger_x + p * (nSwatchSize * 5) + s * nSwatchSize, pattern_y, nSwatchSize, nSwatchSize,
                                  get_color_from_palette_ram(p, s));

            DrawTexture(main_bus->ppu->texture_pattern[0].texture, debugger_x, nametable_y, WHITE);
            DrawTexture(main_bus->ppu->texture_pattern[1].texture, debugger_x + 132, nametable_y, WHITE);

            // DrawRam(bus, 0, 0, 0x0000, 16, 16);
            DrawTextureEx(main_bus->ppu->texture_screen.texture, (Vector2){0, 0}, 0, scale, WHITE);
            // DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, DARKGRAY);

            EndDrawing();
        }
    }
    BeginDrawing();
    EndDrawing();

    StopAudioStream(stream);
    UnloadAudioStream(stream);
    CloseAudioDevice();

    UnloadFont(font);

    CloseWindow();

    bus_free();

    return 0;
}

const char *get_filename(const char *path) {
    const char *filename = path;
    const char *p = path;

    while (*p != '\0') {
        if (*p == '/' || *p == '\\') {
            filename = p + 1;
        }
        p++;
    }

    return filename;
}

void print_usage(const char *executable) { printf("Usage: %s rom\n", get_filename(executable)); }

void draw_ram(const Bus *bus, const int x, const int y, uint16_t addr, int rows, int cols) {
    const int ram_x = x;
    int ram_y = y;
    char buffer[1024];
    for (int row = 0; row < rows; row++) {
        sprintf(buffer, "$%04X", addr);
        for (int col = 0; col < cols; col++) {
            char temp[1024];
            sprintf(temp, "%s %02X", buffer, bus->read(addr));
            addr += 1;
            strcpy(buffer, temp);
        }
        draw_string(buffer, ram_x, ram_y, 18, WHITE);
        ram_y += 20;
    }
}

void draw_cpu(const Bus *bus, const int x, const int y) {
    draw_string("STATUS:", x, y, FONTSIZE, WHITE);
    draw_string("N", x + 60 + 0 * 15, y, FONTSIZE, (bus->cpu->status & N) ? GREEN : RED);
    draw_string("V", x + 60 + 1 * 15, y, FONTSIZE, (bus->cpu->status & V) ? GREEN : RED);
    draw_string("-", x + 60 + 2 * 15, y, FONTSIZE, (bus->cpu->status & U) ? GREEN : RED);
    draw_string("B", x + 60 + 3 * 15, y, FONTSIZE, (bus->cpu->status & B) ? GREEN : RED);
    draw_string("D", x + 60 + 4 * 15, y, FONTSIZE, (bus->cpu->status & D) ? GREEN : RED);
    draw_string("I", x + 60 + 5 * 15, y, FONTSIZE, (bus->cpu->status & I) ? GREEN : RED);
    draw_string("Z", x + 60 + 6 * 15, y, FONTSIZE, (bus->cpu->status & Z) ? GREEN : RED);
    draw_string("C", x + 60 + 7 * 15, y, FONTSIZE, (bus->cpu->status & C) ? GREEN : RED);
    char temp[1024];
    sprintf(temp, "PC: $%04X    SP: $%04X", bus->cpu->pc, bus->cpu->sp);
    draw_string(temp, x, y + FONTSIZE, FONTSIZE, WHITE);
    sprintf(temp, "X: $%02X [%d]   Y: $%02X [%d]", bus->cpu->x, bus->cpu->x, bus->cpu->y, bus->cpu->y);
    draw_string(temp, x, y + FONTSIZE * 2, FONTSIZE, WHITE);
    sprintf(temp, "A: $%02X [%d]", bus->cpu->a, bus->cpu->a);
    draw_string(temp, x, y + FONTSIZE * 3, FONTSIZE, WHITE);
}

void draw_string(const char *text, const int x, const int y, const int size, const Color c) {
    DrawTextEx(font, text, (Vector2){(float)x, (float)y}, (float)size, 1, c);
}

void draw_code(const Bus *bus, const int x, const int y, const int lines) {
    if (array_asm == nullptr)
        return;
    const disasm *inst = &array_asm[bus->cpu->pc];
    int line_y = (lines >> 1) * 10 + y;
    if (inst != nullptr && inst->inst != nullptr) {
        draw_string(inst->inst, x, line_y, FONTSIZE, SKYBLUE);
        while (line_y < (lines * 10) + y) {
            line_y += FONTSIZE;
            if (inst == nullptr)
                break;
            inst = inst->next;
            if (inst != nullptr) {
                draw_string(inst->inst, x, line_y, FONTSIZE, WHITE);
            }
        }
    }

    inst = &array_asm[bus->cpu->pc];
    line_y = (lines >> 1) * 10 + y;
    if (inst != nullptr) {
        while (line_y > y) {
            line_y -= FONTSIZE;
            if (inst == nullptr)
                break;
            inst = inst->prev;
            if (inst != nullptr) {
                draw_string(inst->inst, x, line_y, FONTSIZE, WHITE);
            }
        }
    }
}

void draw_sprite_info(const Bus *bus, const int x, const int y) {
    for (int i = 0; i < 24; i++) {
        char buff[128];
        sprintf(buff, "%02X: (%d, %d) ID: %02X AT: %02X", i, bus->ppu->OAM_pointer[i * 4 + 3], bus->ppu->OAM_pointer[i * 4 + 0],
                bus->ppu->OAM_pointer[i * 4 + 1], bus->ppu->OAM_pointer[i * 4 + 2]);
        draw_string(buff, x, y + i * FONTSIZE, FONTSIZE, WHITE);
    }
}
