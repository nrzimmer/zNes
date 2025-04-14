#ifndef PPU_H
#define PPU_H

#include <raylib.h>
#include <stdint.h>

#include "forward.h"

typedef struct VRamAddr {
    uint16_t x : 5;
    uint16_t y : 5;
    uint16_t nametable_x : 1;
    uint16_t nametable_y : 1;
    uint16_t fine_y : 3;
    uint16_t unused : 1;
} VRamAddr;

typedef struct Sprite {
    uint8_t y;
    uint8_t id;
    uint8_t attribute;
    uint8_t x;
} Sprite;

struct PPU {
    Cartridge *cart;

    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);

    uint8_t nametable[2][1024];
    uint8_t palette[32];

    uint8_t status;
    uint8_t mask;
    uint8_t control;

    VRamAddr vram_addr;
    VRamAddr temp_vram_addr;

    uint8_t fine_x;

    uint8_t address_latch;
    uint8_t data_buffer;

    int16_t scanline;
    int16_t cycle;
    bool odd_frame;

    uint8_t next_tile_id;
    uint8_t next_tile_attrib;
    uint8_t next_tile_lsb;
    uint8_t next_tile_msb;
    uint16_t pattern_lo;
    uint16_t pattern_hi;
    uint16_t attrib_lo;
    uint16_t attrib_hi;
    bool nmi;
    bool frame_complete;

    Sprite OAM[64];
    uint8_t oam_addr;
    Sprite sprite_data[8];
    uint8_t sprite_count;
    uint8_t sprite_lo[8];
    uint8_t sprite_hi[8];

    bool can_zero_hit;
    bool sprite_zero_rendering;

    uint8_t *OAM_pointer;

    RenderTexture2D texture_screen;
    uint8_t screen_buffer[256][240];
    RenderTexture2D texture_nametable[2];
    RenderTexture2D texture_pattern[2];
};

PPU *ppu_new();
void ppu_free(void);

Color *get_color_by_index(uint8_t index);

void raylib_render_pattern_table(uint8_t i, uint8_t palette);

void ppu_clock(void);
void ppu_reset(void);

void gen_screen_texture(void);

Color get_color_from_palette_ram(uint8_t palette, uint8_t pixel);
Color get_color_from_palette_ram_by_index(const uint8_t index);
#endif // PPU_H
