#include <stdlib.h>

#include "cartridge.h"
#include "mapper_000.h"

Mapper *map;

uint32_t cpu_map_000(const uint16_t addr) { return addr & (map->info->prg_rom_pages > 1 ? 0x7FFF : 0x3FFF); }

uint32_t ppu_map_000(const uint16_t addr) { return addr; }

Mapper *new_mapper_000(CartridgeInfo *info) {
    map = calloc(1, sizeof(Mapper));
    map->info = info;
    map->cpu = &cpu_map_000;
    map->ppu = &ppu_map_000;
    return map;
}
