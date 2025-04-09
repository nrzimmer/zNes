#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nes.h"
#include "raylib.h"

typedef struct ROM {
    char *data;
    char *pgr;
    char *chr;
    uint32_t pgr_size;
    uint32_t chr_size;
} ROM;

typedef struct tile {
    int data[8][8];
} tile;

typedef struct chr_bank {
    tile tiles[512];
} chr_bank;

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

const char *mirroring_type_to_string(enum MirroringType mt) {
    if (mt == VERTICAL) {
        return "Vertical";
    }
    return "Horizontal";
}

bool is_bit_set(const unsigned int number, const int bit_position) { return (number & 1 << bit_position) != 0 ? 1 : 0; }

ROM *load_rom(const char *file_path) {
    char NES_HEADER[4] = {0x4E, 0x45, 0x53, 0x1A};

    FILE *rom_file = fopen(file_path, "r");
    if (rom_file == NULL) {
        fprintf(stderr, "Error opening the file.\n");
        exit(1);
    }

    char header[16];
    if (fread(header, 1, 16, rom_file) != 16) {
        fprintf(stderr, "Failed to read rom header.\n");
        exit(1);
    }

    for (int i = 0; i < 4; i++) {
        if (header[i] != NES_HEADER[i]) {
            fprintf(stderr, "Not a NES rom.\n");
            exit(1);
        }
    }

    const uint32_t prg_rom_pages = (unsigned int)header[4];
    const uint32_t chr_rom_pages = (unsigned int)header[5];
    const enum MirroringType mirroring = is_bit_set(header[6], 0) ? VERTICAL : HORIZONTAL;
    const bool has_prg_ram = is_bit_set(header[6], 1);
    const bool has_trainer = is_bit_set(header[6], 2);
    const bool ignore_mirroring = is_bit_set(header[6], 3);
    const uint32_t mapper = (header[6] & 0xF0) >> 4 & (header[7] & 0xF0);

    const bool vs_unisystem = (header[7] & 0x0F) > 0;
    const bool playchoice = is_bit_set(header[7], 1);
    const bool nes2_header = is_bit_set(header[7], 2);
    printf("NES\n"
           "PRG ROM: %dKB (0x%04X)\n"
           "CHR ROM: %dKB (0x%04X)\n"
           "Total ROM: %dKB (0x%04X)\n"
           "Mirroring: %s\n"
           "PRG RAM: %d\n"
           "Has Trainer: %d\n"
           "Ignore Mirroring: %d\n"
           "Vs Unisystem: %d\n"
           "Playchoice: %d\n"
           "NES 2.0 Header: %d\n"
           "Mapper: %d\n",
           prg_rom_pages * 16, prg_rom_pages * 16 * 1024, chr_rom_pages * 8, chr_rom_pages * 8 * 1024,
           (prg_rom_pages * 16) + (chr_rom_pages * 8), ((prg_rom_pages * 16) + (chr_rom_pages * 8)) * 1024,
           mirroring_type_to_string(mirroring), has_prg_ram, has_trainer, ignore_mirroring, vs_unisystem, playchoice, nes2_header, mapper);

    uint32_t pgr_size = prg_rom_pages * 16 * 1024;
    uint32_t chr_size = chr_rom_pages * 8 * 1024;
    ROM *rom = calloc(1, sizeof(ROM));
    rom->data = calloc(1, pgr_size + chr_size);
    rom->pgr = rom->data;
    rom->chr = &rom->data[pgr_size];
    rom->chr_size = chr_size;
    rom->pgr_size = pgr_size;

    if (fread(rom->pgr, 1, pgr_size, rom_file) != pgr_size) {
        fprintf(stderr, "Failed to read rom PRG data.\n");
        exit(1);
    }

    if (fread(rom->chr, 1, chr_size, rom_file) != chr_size) {
        fprintf(stderr, "Failed to read rom CHR data.\n");
        exit(1);
    }

    // Try to trigger EOF
    fgetc(rom_file);
    if (!feof(rom_file)) {
        fprintf(stderr, "Read everything but file still has data...\n");
    }

    fclose(rom_file);

    return rom;
}

void unload_rom(ROM *rom) {
    if (rom != NULL) {
        if (rom->data != NULL) {
            free(rom->data);
        }
        free(rom);
    }
}

void draw_tile(const tile *tile, int pos_x, int pos_y, int scale) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            switch (tile->data[y][x]) {
                case 0:
                    DrawRectangle(pos_x + x * scale, pos_y + y * scale, scale, scale, (Color){255, 255, 255, 255});
                    break;
                case 1:
                    DrawRectangle(pos_x + x * scale, pos_y + y * scale, scale, scale, (Color){255, 0, 0, 255});
                    break;
                case 2:
                    DrawRectangle(pos_x + x * scale, pos_y + y * scale, scale, scale, (Color){255, 0x70, 0, 255});
                    break;
                case 3:
                    DrawRectangle(pos_x + x * scale, pos_y + y * scale, scale, scale, (Color){163, 97, 40, 255});
                    break;
                default:
                    break;
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    Bus *bus = bus_new();
    bus_free(bus);

    char *rom_file = argv[1];
    printf("%s\n", rom_file);

    constexpr int WINDOW_WIDTH = 1024;
    constexpr int WINDOW_HEIGHT = 768;
    const char *WINDOW_TITLE = "Game Window";

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    if (!IsWindowReady()) {
        CloseWindow();
        return -1;
    }

    ROM *rom = load_rom(rom_file);
    chr_bank bank;
    for (int idx = 0; idx < 512; idx++) {
        for (int row = 0; row < 8; row++) {
            const int lo = (unsigned char)rom->chr[(idx * 16) + row];
            const int hi = (unsigned char)rom->chr[(idx * 16) + row + 8];
            bank.tiles[idx].data[row][7] = ((lo & 0x01) > 0 ? (uint8_t)1 : 0) + ((hi & 0x01) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][6] = ((lo & 0x02) > 0 ? (uint8_t)1 : 0) + ((hi & 0x02) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][5] = ((lo & 0x04) > 0 ? (uint8_t)1 : 0) + ((hi & 0x04) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][4] = ((lo & 0x08) > 0 ? (uint8_t)1 : 0) + ((hi & 0x08) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][3] = ((lo & 0x10) > 0 ? (uint8_t)1 : 0) + ((hi & 0x10) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][2] = ((lo & 0x20) > 0 ? (uint8_t)1 : 0) + ((hi & 0x20) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][1] = ((lo & 0x40) > 0 ? (uint8_t)1 : 0) + ((hi & 0x40) > 0 ? (uint8_t)2 : 0);
            bank.tiles[idx].data[row][0] = ((lo & 0x80) > 0 ? (uint8_t)1 : 0) + ((hi & 0x80) > 0 ? (uint8_t)2 : 0);
        }
    }

    BeginDrawing();
    constexpr int TILES = 32;
    constexpr int SCALE = 2;
    constexpr int TILE_SIZE = 8 * SCALE;
    constexpr int BASE = 24;
    ClearBackground(RAYWHITE);
    constexpr int END_X = 511 % TILES * TILE_SIZE + 511 % TILES + 2;
    constexpr int END_Y = 511 / TILES * TILE_SIZE + 511 / TILES + 2;
    DrawRectangle(BASE, BASE, BASE + END_X, BASE + END_Y, BLACK);
    for (int nCurrentTile = 0; nCurrentTile < 512; nCurrentTile++) {
        const int space_x = nCurrentTile % TILES + 1;
        const int space_y = nCurrentTile / TILES + 1;
        const int x = (nCurrentTile % TILES) * TILE_SIZE;
        const int y = (nCurrentTile / TILES) * TILE_SIZE;
        draw_tile(&bank.tiles[nCurrentTile], BASE + space_x + x, BASE + space_y + y, SCALE);
    }
    EndDrawing();

    while (!WindowShouldClose()) {
        BeginDrawing();
        EndDrawing();
    }
    CloseWindow();

    unload_rom(rom);

    return 0;
}
