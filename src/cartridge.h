#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>

#include "forward.h"

struct CartridgeInfo {
    uint8_t prg_rom_pages;
    uint8_t chr_rom_pages;
    uint32_t prg_rom_size;
    uint32_t chr_rom_size;
    uint8_t mapper;
};

struct INesHeader {
    char magic[4];
    uint8_t prg_rom_pages;
    uint8_t chr_rom_pages;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_pages;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
};

typedef enum MirroringType {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO,
    ONESCREEN_HI,
} MirroringType;

struct Cartridge {
    uint8_t (*cpu_read)(uint16_t addr);
    void (*cpu_write)(uint16_t addr, uint8_t data);
    uint8_t (*ppu_read)(uint16_t addr);
    void (*ppu_write)(uint16_t addr, uint8_t data);

    uint8_t *pgr;
    uint8_t *chr;
    uint32_t pgr_size;
    uint32_t chr_size;
    Mapper *mapper;
    CartridgeInfo *info;
    MirroringType mirror;
};

Cartridge *cartridge_new(const char *path);
void cartridge_free(Cartridge *cart);

#endif // CARTRIDGE_H
