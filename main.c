#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// #define DEBUG_LOG
#ifdef DEBUG_LOG
#define _DEBUG_LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define _DEBUG_LOG(format, ...) ((void)0)
#endif

typedef struct ROM {
    char *pgr;
    char *chr;
} ROM;

typedef struct tile {
    int data[8][8];
} tile;

typedef struct chr_bank {
    tile tiles[512];
    SDL_Texture *texture;
    SDL_Renderer *renderer;
} chr_bank;

typedef enum MirroringType { VERTICAL, HORIZONTAL } MirroringType;

const char *get_filename(const char *path);
void print_usage(const char *executable);
void sdl_check_return(int code);
const char *mirroring_type_to_string(MirroringType mt);
bool is_bit_set(unsigned int number, int bit_position);
ROM *load_rom(char *fname);
void unload_rom(ROM *rom);
SDL_Rect get_tile_from_index(int index);
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

void sdl_check_return(int code) {
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(code);
    }
}
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
    return ((number & bitmask) != 0) ? true : false;
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
           "PRG ROM: %dKB\n"
           "CHR ROM: %dKB\n"
           "Mirroring: %s\n"
           "PRG RAM: %d\n"
           "Has Trainer: %d\n"
           "Ignore Mirroring: %d\n"
           "Vs Unisystem: %d\n"
           "Playchoice: %d\n"
           "NES 2.0 Header: %d\n"
           "Mapper: %d\n",
           nPRG_ROM_PAGES * 16, nCHR_ROM_PAGES * 8, mirroring_type_to_string(mtMirroring), bHasPRG_RAM, bHasTrainer, bIgnoreMirroring, bVsUnisystem,
           bPlaychoice, bNes2Header, nMapper);

    uint32_t prg_size = nPRG_ROM_PAGES * 16 * 1024;
    uint32_t chr_size = nCHR_ROM_PAGES * 8 * 1024;
    ROM *rom = malloc(sizeof(ROM));
    rom->chr = malloc(chr_size);
    rom->pgr = malloc(prg_size);

    if (fread(rom->pgr, 1, prg_size, fRom) != prg_size) {
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
        if (rom->chr != NULL) {
            free(rom->chr);
        }
        if (rom->pgr != NULL) {
            free(rom->pgr);
        }
        free(rom);
    }
}

SDL_Rect get_tile_from_index(int index) {
    int x = index % 32;
    int y = index / 32;
    SDL_Rect rect = {x * 8, y * 8, 8, 8};
    return rect;
}

void draw_tile(chr_bank *bank, int index) {
    SDL_Rect origin = get_tile_from_index(index);
    _DEBUG_LOG("X: %03d, Y: %03d\n", origin.x, origin.y);
    SDL_SetRenderDrawColor(bank->renderer, 0xFF, 0xFF, 0xFF, 0);
    SDL_RenderFillRect(bank->renderer, &(SDL_Rect){origin.x, origin.y, 8, 8});
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

int main(int argc, char **argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    char *rom_file = argv[1];
    printf("%s\n", rom_file);

    int r = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    sdl_check_return(r);

    SDL_Window *window = SDL_CreateWindow("Test", 100, 100, 1200, 800, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    ROM *rom = load_rom(rom_file);
    chr_bank bank;
    int TILE_H = 32;
    int TILE_V = 16;
    int TILE_S = 8;
    bank.renderer = renderer;
    bank.texture = SDL_CreateTexture(bank.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, TILE_V * TILE_S, TILE_H * TILE_S);
    SDL_SetRenderTarget(bank.renderer, bank.texture);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(bank.renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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

    _DEBUG_LOG("%d, %d\n\n", TILE_H * TILE_S, TILE_V * TILE_S);

    SDL_SetRenderTarget(bank.renderer, NULL);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool quit = 0;
    while (!quit) {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    quit = 1;
                } break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 0xFF);
        SDL_RenderClear(renderer);
        SDL_Rect src = {0, 0, TILE_H * TILE_S, TILE_V * TILE_S};
        SDL_Rect dst = {50, 50, TILE_H * TILE_S * 4, TILE_V * TILE_S * 4};
        SDL_RenderCopy(renderer, bank.texture, &src, &dst);

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();

    unload_rom(rom);

    return 0;
}
