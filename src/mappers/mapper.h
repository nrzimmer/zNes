#ifndef MAPPER_H
#define MAPPER_H

#include <stdint.h>

#include "forward.h"

typedef struct Mapper {
    CartridgeInfo *info;
    uint32_t (*cpu)(uint16_t addr);
    uint32_t (*ppu)(uint16_t addr);
} Mapper;

void mapper_free(Mapper *map);

#endif // MAPPER_H
