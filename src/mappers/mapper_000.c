#include <stdlib.h>

#include "cartridge.h"
#include "mapper_000.h"

#include "cpu.h"

Mapper *map000;

bool cpu_read_000(const uint16_t addr, uint32_t *mapped_addr, uint8_t *value) {
    *mapped_addr = addr & (map000->info->prg_rom_pages > 1 ? 0x7FFF : 0x3FFF);
    return false;
}

bool cpu_write_000(const uint16_t addr, uint32_t *mapped_addr, uint8_t value) {
    *mapped_addr = addr & (map000->info->prg_rom_pages > 1 ? 0x7FFF : 0x3FFF);
    return false;
}

bool ppu_read_000(const uint16_t addr, uint32_t *mapped_addr, uint8_t *value) {
    *mapped_addr = addr;
    return false;
}

bool ppu_write_000(const uint16_t addr, uint32_t *mapped_addr, uint8_t value) {
    *mapped_addr = addr;
    return false;
}

uint32_t ppu_map_000(const uint16_t addr) { return addr; }

Mapper *new_mapper_000(CartridgeInfo *info) {
    map000 = calloc(1, sizeof(Mapper));
    map000->info = info;
    map000->cpu_read = &cpu_read_000;
    map000->cpu_write = &cpu_write_000;
    map000->ppu_read = &ppu_read_000;
    map000->ppu_write = &ppu_write_000;
    return map000;
}
