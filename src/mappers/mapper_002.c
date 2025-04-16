#include <stdlib.h>

#include "cartridge.h"
#include "mapper_002.h"

#include "cpu.h"

Mapper *map002;

uint8_t map002_bank_select = 0x00;

bool cpu_read_002(const uint16_t addr, uint32_t *mapped_addr, uint8_t *value) {
    if (addr >= 0x8000 && addr <= 0xBFFF) {
        *mapped_addr = map002_bank_select * 0x4000 + (addr & 0x3FFF);
        return false;
    }

    if (addr >= 0xC000 && addr <= 0xFFFF) {
        *mapped_addr = (map002->info->prg_rom_pages - 1) * 0x4000 + (addr & 0x3FFF);
        return false;
    }
    return false;
}

bool cpu_write_002(const uint16_t addr, uint32_t *mapped_addr, uint8_t value) {
    map002_bank_select = value & 0x0F;
    return true;
}

bool ppu_read_002(const uint16_t addr, uint32_t *mapped_addr, uint8_t *value) {
    *mapped_addr = addr;
    return false;
}

bool ppu_write_002(const uint16_t addr, uint32_t *mapped_addr, uint8_t value) {
    *mapped_addr = addr;
    return false;
}

uint32_t ppu_map_002(const uint16_t addr) { return addr; }

Mapper *new_mapper_002(CartridgeInfo *info) {
    map002 = calloc(1, sizeof(Mapper));
    map002->info = info;
    map002->cpu_read = &cpu_read_002;
    map002->cpu_write = &cpu_write_002;
    map002->ppu_read = &ppu_read_002;
    map002->ppu_write = &ppu_write_002;
    return map002;
}
