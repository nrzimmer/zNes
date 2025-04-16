#ifndef MAPPER_H
#define MAPPER_H

#include <stdint.h>

#include "forward.h"

typedef struct Mapper {
    CartridgeInfo *info;
    bool (*cpu_read)(uint16_t addr, uint32_t *mapped_addr, uint8_t *value);
    bool (*cpu_write)(uint16_t addr, uint32_t *mapped_addr, uint8_t value);
    bool (*ppu_read)(uint16_t addr, uint32_t *mapped_addr, uint8_t *value);
    bool (*ppu_write)(uint16_t addr, uint32_t *mapped_addr, uint8_t value);
} Mapper;

void mapper_free(Mapper *map);

#endif // MAPPER_H
