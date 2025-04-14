#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#include "forward.h"

struct Bus {
    Cpu *cpu;
    PPU *ppu;
    APU *apu;
    Cartridge *cart;
    uint8_t (*read)(uint16_t addr);
    void (*write)(uint16_t addr, uint8_t data);
    uint32_t clock_count;
    uint8_t ram[2 * 1024];
    uint8_t controller[2];
    uint8_t controller_cache[2];
    uint8_t dma_page;
    uint8_t dma_addr;
    uint8_t dma_data;
    bool dma_odd_cycle;
    bool dma_transfer_active;
    double dAudioSample;
};

Bus *bus_new();
void bus_free();

void set_cart(Cartridge *cart);
void bus_reset();

void SetSampleFrequency(uint32_t sample_rate);
bool bus_clock();

#endif // BUS_H
