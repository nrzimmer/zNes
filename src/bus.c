#include <stdlib.h>

#include "bus.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"

uint8_t bus_read(uint16_t addr);
void bus_write(uint16_t addr, uint8_t data);

Bus *bus;

Bus *bus_new() {
    bus = calloc(1, sizeof(Bus));
    bus->cpu = cpu_new(bus);
    bus->ppu = ppu_new();
    bus->read = &bus_read;
    bus->write = &bus_write;
    bus->clock_count = 0;
    bus->dma_page = 0x00;
    bus->dma_addr = 0x00;
    bus->dma_data = 0x00;
    bus->dma_odd_cycle = true;
    bus->dma_transfer_active = false;
    return bus;
}

void bus_free() {
    cpu_free(bus->cpu);
    if (bus->ppu != nullptr)
        ppu_free();
    free(bus);
}

uint8_t bus_read(const uint16_t addr) {
    uint8_t data = 0x00;
    if (addr <= 0x1FFF) {
        data = bus->ram[addr & 0x07FF];
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        data = bus->ppu->read(addr & 0x0007);
    } else if (addr >= 0x4016 && addr <= 0x4017) {
        data = (bus->controller_cache[addr & 0x0001] & 0x80) > 0;
        bus->controller_cache[addr & 0x0001] <<= 1;
    } else if (addr >= 0x8000) {
        data = bus->cart->cpu_read(addr);
    }

    return data;
}

void bus_write(const uint16_t addr, const uint8_t data) {
    if (addr <= 0x1FFF) {
        bus->ram[addr & 0x07FF] = data;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        bus->ppu->write(addr & 0x0007, data);
    } else if (addr == 0x4014) {
        bus->dma_page = data;
        bus->dma_addr = 0x00;
        bus->dma_transfer_active = true;
    } else if (addr >= 0x4016 && addr <= 0x4017) {
        bus->controller_cache[addr & 0x0001] = bus->controller[addr & 0x0001];
    } else if (addr >= 0x8000) {
        bus->cart->cpu_write(addr, data);
    }
}

void set_cart(Cartridge *cart) {
    bus->cart = cart;
    bus->ppu->cart = cart;
}

void bus_reset() {
    cpu_reset();
    ppu_reset();
    bus->clock_count = 0;
    bus->dma_page = 0x00;
    bus->dma_addr = 0x00;
    bus->dma_data = 0x00;
    bus->dma_odd_cycle = true;
    bus->dma_transfer_active = false;
}

void bus_clock() {
    ppu_clock();
    if (bus->clock_count % 3 == 0) {
        if (bus->dma_transfer_active) {
            if (bus->dma_odd_cycle) {
                if (bus->clock_count % 2 == 1) {
                    bus->dma_odd_cycle = false;
                }
            } else {
                if (bus->clock_count % 2 == 0) {
                    bus->dma_data = bus->read(bus->dma_page << 8 | bus->dma_addr);
                } else {
                    bus->ppu->OAM_pointer[bus->dma_addr] = bus->dma_data;
                    bus->dma_addr++;
                    if (bus->dma_addr == 0x00) {
                        bus->dma_transfer_active = false;
                        bus->dma_odd_cycle = true;
                    }
                }
            }
        } else {
            cpu_clock();
        }
    }
    if (bus->ppu->nmi) {
        bus->ppu->nmi = false;
        cpu_nmi();
    }

    bus->clock_count++;
}
