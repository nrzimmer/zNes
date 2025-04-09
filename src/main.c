#include <stdint.h>
#include <stdio.h>
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

int main(int argc, char **argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    // char *rom_file = argv[1];
    // rom_file = "nestest.nes";
    int rom_index = 0;
    char *roms[3] = {"smb.nes", "dk.nes", "nestest.nes"};
    // printf("%s\n", rom_file);

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

    Bus *bus = bus_new();
    Cartridge *cart = cartridge_new(roms[rom_index]);
    bus_insert_cartridge(cart);
    array_asm = disassemble(bus, 0x0000, 0xFFFF);
    bus_reset();

    int debugger_x;
    int pattern_y;
    int nametable_y;

    bool resize = true;
    bool emulate = false;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_KP_ADD)) {
            if (scale < 4) {
                scale++;
                resize = true;
            }
        }

        if (IsKeyPressed(KEY_KP_SUBTRACT)) {
            if (scale > 1) {
                scale--;
                resize = true;
            }
        }

        if (IsKeyPressed(KEY_T)) {
            rom_index = (rom_index + 1) % 3;
            cart = cartridge_new(roms[rom_index]);
            bus_insert_cartridge(cart);
            bus_reset();
            BeginDrawing();
            EndDrawing();
            emulate = true;
            continue;
        }

        if (resize) {
            resize = false;
            window_width = 256 * (scale + 1) + 12;
            window_height = 240 * scale;
            debugger_x = 256 * scale + 4;
            if (scale == 1) {
                pattern_y = 72;
            } else {
                pattern_y = 340;
            }
            nametable_y = pattern_y + 12;

            SetWindowSize(window_width, window_height);
        }

        bus->controller[0] = 0x00;
        bus->controller[0] |= IsKeyDown(KEY_X) | IsKeyDown(KEY_S) ? 0x80 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_Z) | IsKeyDown(KEY_A) ? 0x40 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_N) ? 0x20 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_M) ? 0x10 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_UP) ? 0x08 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_DOWN) ? 0x04 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_LEFT) ? 0x02 : 0x00;
        bus->controller[0] |= IsKeyDown(KEY_RIGHT) ? 0x01 : 0x00;

        BeginTextureMode(bus->ppu->texture_screen);
        uint16_t pc = 0;
        if (emulate) {
            do {
                if (pc != bus->cpu->pc) {
                    pc = bus->cpu->pc;
                    // Log asm instruction to stdout
                    // disasm_addr(bus, pc);
                }
                bus_clock();
            } while (!bus->ppu->frame_complete);
            do {
                bus_clock();

            } while (bus->cpu->cycles != 0);
            bus->ppu->frame_complete = false;
            do {
                bus_clock();
            } while (bus->cpu->cycles == 0);
        } else {
            if (IsKeyPressed(KEY_C)) {
                // Run one full instruction
                do {
                    bus_clock();
                } while (bus->cpu->cycles != 0);
                // Reset frame flag
                bus->ppu->frame_complete = false;
            }

            if (IsKeyPressed(KEY_F)) {
                // Emulate a single frame
                do {
                    bus_clock();
                } while (!bus->ppu->frame_complete);
                // End current instruction
                do {
                    bus_clock();
                } while (bus->cpu->cycles != 0);
                bus->ppu->frame_complete = false;
            }
        }
        EndTextureMode();

        if (IsKeyPressed(KEY_P))
            emulate = !emulate;

        if (IsKeyPressed(KEY_R)) {
            bus_reset();

            // Clean pressed keys
            BeginDrawing();
            EndDrawing();
            continue;
        }

        // Render Pattern Tables
        raylib_render_pattern_table(0, 0);
        raylib_render_pattern_table(1, 0);

        BeginDrawing();
        ClearBackground(BG_BLUE);

        draw_cpu(bus, debugger_x, 2);
        if (scale > 1) {
            // DrawCode(bus, debugger_x, 72, 24);
            draw_sprite_info(bus, debugger_x, 72);
        }

        // Draw Palettes & Pattern Tables
        const int nSwatchSize = 6;
        for (int p = 0; p < 8; p++)     // For each palette
            for (int s = 0; s < 4; s++) // For each index
                DrawRectangle(debugger_x + p * (nSwatchSize * 5) + s * nSwatchSize, pattern_y, nSwatchSize, nSwatchSize,
                              get_color_from_palette_ram(p, s));

        // Draw Selected Palette
        // TODO - first the palettes need to ne populated

        // Draw pattern tables status
        DrawTexture(bus->ppu->texture_pattern[0].texture, debugger_x, nametable_y, WHITE);
        DrawTexture(bus->ppu->texture_pattern[1].texture, debugger_x + 132, nametable_y, WHITE);

        // DrawRam(bus, 0, 0, 0x0000, 16, 16);

        // Draw nametable
        // char buff[32];
        // for (uint8_t y = 0; y < 30; y++)
        //     for (uint8_t x = 0; x < 32; x++) {
        //         sprintf(buff, "%02X", bus->ppu->tblName[0][y*32+x]);
        //         DrawString(buff,x*22,y*22,16, WHITE);
        //     }

        // Draw rendered Frame
        DrawTextureEx(bus->ppu->texture_screen.texture, (Vector2){0, 0}, 0, scale, WHITE);

        // Display the FPS on the screen
        // DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, DARKGRAY);

        EndDrawing();
    }
    CloseWindow();

    // Print ram
    // uint16_t nAddr = 0;
    // char sOffset[1024];
    // for (int row = 0; row < 16; row++) {
    //     sprintf(sOffset, "$%04X", nAddr);
    //     for (int col = 0; col < 16; col++) {
    //         char temp[1024];
    //         sprintf(temp, "%s %02X", sOffset, bus->read(nAddr));
    //         nAddr += 1;
    //         strcpy(sOffset, temp);
    //     }
    //     printf(sOffset);
    //     printf("\n");
    // }

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
