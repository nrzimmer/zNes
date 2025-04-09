#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nes6502.h"
#include "raylib.h"


typedef struct ROM {
    char *data;
    char *pgr;
    char *chr;
    int pgr_size;
    int chr_size;
} ROM;

typedef struct tile {
    int data[8][8];
} tile;

typedef struct chr_bank {
    tile tiles[512];
} chr_bank;

typedef enum MirroringType { VERTICAL, HORIZONTAL } MirroringType;

const char *get_filename(const char *path);
void print_usage(const char *executable);
const char *mirroring_type_to_string(MirroringType mt);
bool is_bit_set(unsigned int number, int bit_position);
ROM *load_rom(char *fname);
void unload_rom(ROM *rom);
void draw_tile(chr_bank *bank, int index);

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

const char *mirroring_type_to_string(MirroringType mt) {
    if (mt == VERTICAL) {
        return "Vertical";
    }
    return "Horizontal";
}

bool is_bit_set(unsigned int number, int bit_position) {
    unsigned int bitmask = 1 << bit_position;

    // Perform a bitwise AND operation with the number and the bitmask
    // If the result is non-zero, the bit at bit_position is set
    return ((number & bitmask) != 0) ? 1 : 0;
}

ROM *load_rom(char *fname) {
    char NES_HEADER[4] = {0x4E, 0x45, 0x53, 0x1A};
    uint32_t nPRG_ROM_PAGES;
    uint32_t nCHR_ROM_PAGES;
    MirroringType mtMirroring;
    bool bHasPRG_RAM;
    bool bHasTrainer;
    bool bIgnoreMirroring;
    bool bVsUnisystem;
    bool bPlaychoice;
    bool bNes2Header;
    uint32_t nMapper;

    FILE *fRom = fopen(fname, "r");
    if (fRom == NULL) {
        fprintf(stderr, "Error opening the file.\n");
        exit(1);
    }

    char header[16];
    if (fread(header, 1, 16, fRom) != 16) {
        fprintf(stderr, "Failed to read rom header.\n");
        exit(1);
    }

    for (int i = 0; i < 4; i++) {
        if (header[i] != NES_HEADER[i]) {
            fprintf(stderr, "Not a NES rom.\n");
            exit(1);
        }
    }

    nPRG_ROM_PAGES = header[4];
    nCHR_ROM_PAGES = header[5];
    mtMirroring = is_bit_set(header[6], 0) ? VERTICAL : HORIZONTAL;
    bHasPRG_RAM = is_bit_set(header[6], 1);
    bHasTrainer = is_bit_set(header[6], 2);
    bIgnoreMirroring = is_bit_set(header[6], 3);
    nMapper = (header[6] & 0xF0) >> 4;

    bVsUnisystem = (header[7] & 0x0F) > 0;
    bPlaychoice = is_bit_set(header[7], 1);
    bNes2Header = is_bit_set(header[7], 2);
    nMapper &= header[7] & 0xF0;
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
           nPRG_ROM_PAGES * 16, nPRG_ROM_PAGES * 16 * 1024, nCHR_ROM_PAGES * 8, nCHR_ROM_PAGES * 8 * 1024, (nPRG_ROM_PAGES * 16) + (nCHR_ROM_PAGES * 8), ((nPRG_ROM_PAGES * 16) + (nCHR_ROM_PAGES * 8)) * 1024, mirroring_type_to_string(mtMirroring), bHasPRG_RAM, bHasTrainer, bIgnoreMirroring,
           bVsUnisystem, bPlaychoice, bNes2Header, nMapper);

    uint32_t pgr_size = nPRG_ROM_PAGES * 16 * 1024;
    uint32_t chr_size = nCHR_ROM_PAGES * 8 * 1024;
    ROM *rom = malloc(sizeof(ROM));
    rom->data = malloc(pgr_size + chr_size);
    rom->pgr = rom->data;
    rom->chr = &rom->data[pgr_size];
    rom->chr_size = chr_size;
    rom->pgr_size = pgr_size;

    if (fread(rom->pgr, 1, pgr_size, fRom) != pgr_size) {
        fprintf(stderr, "Failed to read rom PRG data.\n");
        exit(1);
    }

    if (fread(rom->chr, 1, chr_size, fRom) != chr_size) {
        fprintf(stderr, "Failed to read rom CHR data.\n");
        exit(1);
    }

    // Try to trigger EOF
    fgetc(fRom);
    if (!feof(fRom)) {
        fprintf(stderr, "Read everything but file still has data...\n");
    }

    fclose(fRom);

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


Texture2D create_sprite_from_code() {
    const int spriteWidth = 8;
    const int spriteHeight = 8;

    // Create a blank image (8x8)
    Image image = GenImageColor(spriteWidth, spriteHeight, BLANK);

    // Draw pixels into the image programmatically
    for (int y = 0; y < spriteHeight; y++) {
        for (int x = 0; x < spriteWidth; x++) {
            // Example: Create a checkerboard pattern
            if ((x + y) % 2 == 0) {
                ImageDrawPixel(&image, x, y, RED); // Red pixels
            } else {
                ImageDrawPixel(&image, x, y, BLUE); // Blue pixels
            }
        }
    }

    // Convert the image to a texture for rendering
    Texture2D texture = LoadTextureFromImage(image);

    // Unload the image from RAM after uploading to GPU
    UnloadImage(image);

    return texture;
}

void draw_generated_sprite(Texture2D texture, Vector2 position) {
    DrawTexture(texture, (int)position.x, (int)position.y, WHITE);
}

void draw_tile(chr_bank *bank, int index) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            bool draw = true;
            switch (bank->tiles[index].data[y][x]) {
                case 0:
                    draw = false;
                    break;
                case 1:
                    SDL_SetRenderDrawColor(bank->renderer, 0xFF, 0, 0, 0xFF);
                    break;
                case 2:
                    SDL_SetRenderDrawColor(bank->renderer, 0xFF, 0x70, 0, 0xFF);
                    break;
                case 3:
                    SDL_SetRenderDrawColor(bank->renderer, 163, 97, 40, 0xFF);
                    break;
            }
            if (draw) {
                SDL_RenderFillRect(bank->renderer, &(SDL_Rect){origin.x + x, origin.y + y, 1, 1});
            }
        }
    }
}

uint8_t ram[0x800];
uint8_t ppu[0x8];
uint8_t apu[0x18];
ROM *rom;

typedef enum NES_IO_SOURCE {
    NES_RAM, NES_PPU, NES_APU, NES_ROM
} NES_IO_SOURCE;

uint16_t mapper1_transform(uint16_t address) {
    /*
        $0000–$07FF 	$0800 	2 KB internal RAM
        $0800–$0FFF 	$0800 	Mirrors of $0000–$07FF
        $1000–$17FF 	$0800
        $1800–$1FFF 	$0800
        $2000–$2007 	$0008 	NES PPU registers
        $2008–$3FFF 	$1FF8 	Mirrors of $2000–$2007 (repeats every 8 bytes)
        $4000–$4017 	$0018 	NES APU and I/O registers
        $4018–$401F 	$0008 	APU and I/O functionality that is normally disabled. See CPU Test Mode.
        $4020–$FFFF
        • $6000–$7FFF
        • $8000–$FFFF 	$BFE0
        $2000
        $8000 	Unmapped. Available for cartridge use.
        Usually cartridge RAM, when present.
        Usually cartridge ROM and mapper registers.
    */
    if (address < 0x2000) {
        return address & 0x7FF;
    }

    if (address < 0x4000) {
        return (address - 0x2000) % 0x08;
    }

    if (address < 0x4020) {
        return (address - 0x4000) % 0x18;
    }

    if (address >= 0x8000) {
        return address & 0x7FFF;
    }

    fprintf(stderr, "Invalid read address: %X\n", address);
    exit(1);
}

uint16_t mapper1_source(uint16_t address) {
    if (address < 0x2000) {
        return NES_RAM;
    }

    if (address < 0x4000) {
        return NES_PPU;
    }

    if (address < 0x4020) {
        return NES_APU;
    }

    if (address >= 0x8000) {
        return NES_ROM;
    }

    fprintf(stderr, "Invalid read address: %X\n", address);
    exit(1);
}

uint8_t nesread(NES_IO_SOURCE src, uint16_t address) {
    switch (src) {
        case NES_RAM: {
            return ram[address];
        }
        case NES_PPU: {
            return ppu[address];
        }
        case NES_APU: {
            return apu[address];
        }
        case NES_ROM: {
            return rom->data[address];
        }
    }
    return 0;
}

char *source_name[4] = {
    "RAM",
    "PPU",
    "APU",
    "ROM"
};

void neswrite(NES_IO_SOURCE src, uint16_t address, uint8_t value) {
    if (src ==  NES_RAM) {
        ram[address] = value;
        return;
    }

    if (src == NES_PPU) {
        ppu[address] = value;
        return;
    }

    fprintf(stderr, "Invalid write source [%s] address: %X\n", source_name[src], address);
    exit(1);
}

uint8_t generic_read(uint16_t address) {
    NES_IO_SOURCE src = mapper1_source(address);
    uint16_t addr = mapper1_transform(address);
    uint8_t value = nesread(src, addr);
    printf("READ [ 0x%04X ][ %s ][ 0x%04X ] -> 0x%02X\n", address, source_name[src], addr, value);
    return value;
}

void generic_write(uint16_t address, uint8_t value) {
    NES_IO_SOURCE src = mapper1_source(address);
    uint16_t addr = mapper1_transform(address);
    printf("WRITE[ 0x%04X ][ %s ][ 0x%04X ] <- 0x%02X\n", address, source_name[src], addr, value);
    neswrite(src, addr, value);
}


void draw(chr_bank *bankp) {
    chr_bank bank = *bankp;
    for (int nCurrentTile = 0; nCurrentTile < 512; nCurrentTile++) {
        for (int i = 0; i < 8; i++) {
            int chrLo, chrHi;
            chrLo = (unsigned char)rom->chr[(nCurrentTile * 16) + i];
            chrHi = (unsigned char)rom->chr[(nCurrentTile * 16) + i + 8];
            bank.tiles[nCurrentTile].data[i][7] = (((chrLo & 0x01) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x01) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][6] = (((chrLo & 0x02) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x02) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][5] = (((chrLo & 0x04) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x04) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][4] = (((chrLo & 0x08) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x08) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][3] = (((chrLo & 0x10) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x10) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][2] = (((chrLo & 0x20) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x20) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][1] = (((chrLo & 0x40) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x40) > 0) ? (uint8_t)2 : 0);
            bank.tiles[nCurrentTile].data[i][0] = (((chrLo & 0x80) > 0) ? (uint8_t)1 : 0) + (((chrHi & 0x80) > 0) ? (uint8_t)2 : 0);
            _DEBUG_LOG("%3d - 0x%02X 0x%02X: %d,%d,%d,%d,%d,%d,%d,%d\n", nCurrentTile, chrLo, chrHi, bank.tiles[nCurrentTile].data[i][0],
                       bank.tiles[nCurrentTile].data[i][1], bank.tiles[nCurrentTile].data[i][2], bank.tiles[nCurrentTile].data[i][3],
                       bank.tiles[nCurrentTile].data[i][4], bank.tiles[nCurrentTile].data[i][5], bank.tiles[nCurrentTile].data[i][6],
                       bank.tiles[nCurrentTile].data[i][7]);
        }
        draw_tile(&bank, nCurrentTile);
        _DEBUG_LOG("\n");
    }
}


int main(int argc, char **argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    char *rom_file = argv[1];
    printf("%s\n", rom_file);

    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    const char* WINDOW_TITLE = "Game Window";

    // Initialize window and OpenGL context
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

    // Check if window was initialized successfully
    if (!IsWindowReady()) {
        // Handle initialization error
        CloseWindow();  // Clean up in case of failure
        return -1;
    }

    rom = load_rom(rom_file);
    set_io(&generic_read, &generic_write);
    reset6502();
    chr_bank bank;
    int TILE_H = 32;
    int TILE_V = 16;
    int TILE_S = 8;
    int SCALE = 3;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    draw(&bank);
    EndDrawing();

    // Window close detection
    while (!WindowShouldClose()) {
        BeginDrawing();
        EndDrawing();
    }

    // Clean up resources when done
    CloseWindow();

    _DEBUG_LOG("%d, %d\n\n", TILE_H * TILE_S, TILE_V * TILE_S);

    unload_rom(rom);

    return 0;
}
