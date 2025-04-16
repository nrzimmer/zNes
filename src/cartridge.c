#include <stdio.h>
#include <stdlib.h>

#include "cartridge.h"
#include "mappers/mapper_000.h"
#include "mappers/mapper_002.h"

uint8_t cart_cpu_read(uint16_t addr);
void cart_cpu_write(uint16_t addr, uint8_t data);
uint8_t cart_ppu_read(uint16_t addr);
void cart_ppu_write(uint16_t addr, uint8_t data);

Cartridge *cart;

Cartridge *cartridge_new(const char *path) {
    FILE *rom_file = fopen(path, "r");
    if (rom_file == NULL) {
        fprintf(stderr, "Error opening the file.\n");
        return nullptr;
    }

    INesHeader header;
    if (fread((char *)&header, 1, 16, rom_file) != 16) {
        fprintf(stderr, "Failed to read rom header.\n");
        return nullptr;
    }

    const uint8_t NES_HEADER[4] = {0x4E, 0x45, 0x53, 0x1A};
    for (int i = 0; i < 4; i++) {
        if (header.magic[i] != NES_HEADER[i]) {
            fprintf(stderr, "Not a NES rom.\n");
            return nullptr;
        }
    }

    if (header.mapper1 & 0x04)
        fseek(rom_file, 512, SEEK_CUR); // 512-byte trainer

    cart = calloc(1, sizeof(Cartridge));
    CartridgeInfo *info = calloc(1, sizeof(CartridgeInfo));
    cart->info = info;
    cart->mirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;
    info->mapper = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);

    uint8_t ines_version = 1;
    if ((header.mapper2 & 0x0C) == 0x08)
        ines_version = 2;

    if (ines_version == 0) {
        /*
        const uint32_t prg_rom_pages = (unsigned int) header[4];
        const uint32_t chr_rom_pages = (unsigned int) header[5];
        const MirroringType mirroring = is_bit_set(header[6], 0) ? VERTICAL : HORIZONTAL;
        const bool has_prg_ram = is_bit_set(header[6], 1);
        const bool has_trainer = is_bit_set(header[6], 2);
        const bool ignore_mirroring = is_bit_set(header[6], 3);
        const uint32_t mapper = (header[6] & 0xF0) >> 4 & (header[7] & 0xF0);

        const bool vs_unisystem = (header[7] & 0x0F) > 0;
        const bool playchoice = is_bit_set(header[7], 1);
        const bool nes2_header = is_bit_set(header[7], 2);
        */
        fprintf(stderr, "Not implemented support for iNes Header v0.\n");
        return nullptr;
    }

    if (ines_version == 1) {
        info->prg_rom_pages = header.prg_rom_pages;
        info->prg_rom_size = header.prg_rom_pages * 16 * 1024;
        cart->pgr = calloc(1, info->prg_rom_size);
        uint32_t read = fread(cart->pgr, 1, info->prg_rom_size, rom_file);
        if (read != info->prg_rom_size) {
            fprintf(stderr, "Failed to read rom PGR data.\n Expected %d bytes but only got %d bytes.\n", info->prg_rom_size, read);
            return nullptr;
        }

        info->chr_rom_pages = header.chr_rom_pages;
        info->chr_rom_size = header.chr_rom_pages * 8 * 1024;
        cart->chr = calloc(1, info->chr_rom_size);
        read = fread(cart->chr, 1, info->chr_rom_size, rom_file);
        if (read != info->chr_rom_size) {
            fprintf(stderr, "Failed to read rom CHR data.\n Expected %d bytes but only got %d bytes.\n", info->chr_rom_size, read);
            return nullptr;
        }
    }

    if (ines_version == 2) {
        fprintf(stderr, "Not implemented support for iNes Header v2.\n");
        return nullptr;
    }

    fgetc(rom_file);
    if (!feof(rom_file)) {
        fprintf(stderr, "WARN: Read everything but file still has data...\n");
    }

    fclose(rom_file);

    switch (info->mapper) {
        case 0:
            cart->mapper = new_mapper_000(info);
        case 2:
            cart->mapper = new_mapper_002(info);
            break;
        default:
            fprintf(stderr, "Mapper %d not implemented yet.\n", info->mapper);
            return nullptr;
    }

    cart->cpu_read = &cart_cpu_read;
    cart->cpu_write = &cart_cpu_write;
    cart->ppu_read = &cart_ppu_read;
    cart->ppu_write = &cart_ppu_write;
    return cart;
}

void cartridge_free(Cartridge *cart) {
    free(cart->info);
    free(cart->pgr);
    free(cart->chr);
    free(cart);
}

uint8_t cart_cpu_read(const uint16_t addr) {
    uint32_t mapped_addr;
    uint8_t value;
    if (cart->mapper->cpu_read(addr, &mapped_addr, &value)) {
        return value;
    }
    return cart->pgr[mapped_addr];
}

void cart_cpu_write(const uint16_t addr, const uint8_t data) {
    uint32_t mapped_addr;
    if (cart->mapper->cpu_write(addr, &mapped_addr, data)) {
        return;
    }
    cart->pgr[mapped_addr] = data;
}

uint8_t cart_ppu_read(const uint16_t addr) {
    uint32_t mapped_addr;
    uint8_t value;
    if (cart->mapper->ppu_read(addr, &mapped_addr, &value)) {
        return value;
    }
    return cart->chr[mapped_addr];
}

void cart_ppu_write(const uint16_t addr, const uint8_t data) {
    uint32_t mapped_addr;
    if (cart->mapper->ppu_write(addr, &mapped_addr, data)) {
        return;
    }
    cart->chr[mapped_addr] = data;
}
