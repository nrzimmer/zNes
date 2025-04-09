#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#include "bus.h"
#include "cartridge.h"
#include "ppu.h"

#define BIT_7 (1 << 7)
#define BIT_6 (1 << 6)
#define BIT_5 (1 << 5)
#define BIT_4 (1 << 4)
#define BIT_3 (1 << 3)
#define BIT_2 (1 << 2)
#define BIT_1 (1 << 1)
#define BIT_0 (1 << 0)

#define STATUS_VERTICAL_BLANK BIT_7
#define STATUS_SPRITE_ZERO_HIT BIT_6
#define STATUS_SPRITE_OVERFLOW BIT_5

#define MASK_EMPHASIZE_BLUE BIT_7
#define MASK_EMPHASIZE_GREEN BIT_6
#define MASK_EMPHASIZE_RED BIT_5
#define MASK_ENABLE_SPRITE BIT_4
#define MASK_ENABLE_BACKGROUND BIT_3
#define MASK_SHOW_SPRITE_LEFT BIT_2
#define MASK_SHOW_BACKGROUND_LEFT BIT_1
#define MASK_GRAYSCALE BIT_0

#define CONTROL_NAMETABLE_X BIT_0
#define CONTROL_NAMETABLE_Y BIT_1
#define CONTROL_INCREMENT_MODE BIT_2
#define CONTROL_PATTERN_SPRITE BIT_3
#define CONTROL_PATTERN_BACKGROUND BIT_4
#define CONTROL_SPRITE_SIZE BIT_5
#define CONTROL_ENABLE_NMI BIT_7

#define VRAM_TO_UINT16 *(uint16_t *)

uint8_t ppu_read(uint16_t addr);
void ppu_write(uint16_t addr, uint8_t data);

uint8_t ppu_cpu_read(uint16_t addr);
void ppu_cpu_write(uint16_t addr, uint8_t data);

PPU *ppu;

Color NTSC[0x40] = {
    {84, 84, 84, 255},    {0, 30, 116, 255},    {8, 16, 144, 255},    {48, 0, 136, 255},    {68, 0, 100, 255},    {92, 0, 48, 255},
    {84, 4, 0, 255},      {60, 24, 0, 255},     {32, 42, 0, 255},     {8, 58, 0, 255},      {0, 64, 0, 255},      {0, 60, 0, 255},
    {0, 50, 60, 255},     {0, 0, 0, 255},       {0, 0, 0, 255},       {0, 0, 0, 255},       {152, 150, 152, 255}, {8, 76, 196, 255},
    {48, 50, 236, 255},   {92, 30, 228, 255},   {136, 20, 176, 255},  {160, 20, 100, 255},  {152, 34, 32, 255},   {120, 60, 0, 255},
    {84, 90, 0, 255},     {40, 114, 0, 255},    {8, 124, 0, 255},     {0, 118, 40, 255},    {0, 102, 120, 255},   {0, 0, 0, 255},
    {0, 0, 0, 255},       {0, 0, 0, 255},       {236, 238, 236, 255}, {76, 154, 236, 255},  {120, 124, 236, 255}, {176, 98, 236, 255},
    {228, 84, 236, 255},  {236, 88, 180, 255},  {236, 106, 100, 255}, {212, 136, 32, 255},  {160, 170, 0, 255},   {116, 196, 0, 255},
    {76, 208, 32, 255},   {56, 204, 108, 255},  {56, 180, 204, 255},  {60, 60, 60, 255},    {0, 0, 0, 255},       {0, 0, 0, 255},
    {236, 238, 236, 255}, {168, 204, 236, 255}, {188, 188, 236, 255}, {212, 178, 236, 255}, {236, 174, 236, 255}, {236, 174, 212, 255},
    {236, 180, 176, 255}, {228, 196, 144, 255}, {204, 210, 120, 255}, {180, 222, 120, 255}, {168, 226, 144, 255}, {152, 226, 180, 255},
    {160, 214, 228, 255}, {160, 162, 160, 255}, {0, 0, 0, 255},       {0, 0, 0, 255}};

PPU *ppu_new(Cartridge *cart) {
    ppu = calloc(1, sizeof(PPU));
    ppu->cart = cart;
    ppu->read = &ppu_cpu_read;
    ppu->write = &ppu_cpu_write;
    ppu_reset();
    ppu->texture_screen = LoadRenderTexture(256, 240);
    ppu->texture_nametable[0] = LoadRenderTexture(256, 240);
    ppu->texture_nametable[1] = LoadRenderTexture(256, 240);
    ppu->texture_pattern[0] = LoadRenderTexture(128, 128);
    ppu->texture_pattern[1] = LoadRenderTexture(128, 128);
    return ppu;
}

void ppu_free(void) {
    free(ppu);
    ppu = nullptr;
}

Color *get_color_by_index(const uint8_t index) { return &NTSC[index]; }

void raylib_render_pattern_table(const uint8_t i, const uint8_t palette) {
    BeginTextureMode(ppu->texture_pattern[i]);
    for (uint16_t y = 0; y < 16; y++) {
        for (uint16_t x = 0; x < 16; x++) {
            const uint16_t offset = y * 256 + x * 16;
            for (uint16_t row = 0; row < 8; row++) {
                uint8_t tile_lsb = ppu_read(i * 0x1000 + offset + row + 0x0000);
                uint8_t tile_msb = ppu_read(i * 0x1000 + offset + row + 0x0008);
                for (uint16_t col = 0; col < 8; col++) {
                    const uint8_t pixel = (tile_msb & 0x01) << 1 | (tile_lsb & 0x01);
                    tile_lsb >>= 1;
                    tile_msb >>= 1;
                    const int pos_y = 127 - (y * 8 + row); // RayLib
                    DrawPixel(x * 8 + (7 - col), pos_y, get_color_from_palette_ram(palette, pixel));
                }
            }
        }
    }
    EndTextureMode();
}

void scroll_x(void) {
    if (ppu->mask & MASK_ENABLE_BACKGROUND || ppu->mask & MASK_ENABLE_SPRITE) {
        if (ppu->vram_addr.x == 31) {
            ppu->vram_addr.x = 0;
            ppu->vram_addr.nametable_x = ~ppu->vram_addr.nametable_x;
        } else {
            ppu->vram_addr.x++;
        }
    }
}

void scroll_y(void) {
    if (ppu->mask & MASK_ENABLE_BACKGROUND || ppu->mask & MASK_ENABLE_SPRITE) {
        if (ppu->vram_addr.fine_y < 7) {
            ppu->vram_addr.fine_y++;
        } else {
            ppu->vram_addr.fine_y = 0;

            if (ppu->vram_addr.y == 29) {
                ppu->vram_addr.y = 0;
                ppu->vram_addr.nametable_y = ~ppu->vram_addr.nametable_y;
            } else if (ppu->vram_addr.y == 31) {
                ppu->vram_addr.y = 0;
            } else {
                ppu->vram_addr.y++;
            }
        }
    }
}

void transfer_x(void) {
    if (ppu->mask & MASK_ENABLE_BACKGROUND || ppu->mask & MASK_ENABLE_SPRITE) {
        ppu->vram_addr.nametable_x = ppu->temp_vram_addr.nametable_x;
        ppu->vram_addr.x = ppu->temp_vram_addr.x;
    }
}

void transfer_y(void) {
    if (ppu->mask & MASK_ENABLE_BACKGROUND || ppu->mask & MASK_ENABLE_SPRITE) {
        ppu->vram_addr.fine_y = ppu->temp_vram_addr.fine_y;
        ppu->vram_addr.nametable_y = ppu->temp_vram_addr.nametable_y;
        ppu->vram_addr.y = ppu->temp_vram_addr.y;
    }
}

void load_shifters(void) {
    ppu->pattern_lo = (ppu->pattern_lo & 0xFF00) | ppu->next_tile_lsb;
    ppu->pattern_hi = (ppu->pattern_hi & 0xFF00) | ppu->next_tile_msb;
    ppu->attrib_lo = (ppu->attrib_lo & 0xFF00) | ((ppu->next_tile_attrib & 0x01) ? 0xFF : 0x00);
    ppu->attrib_hi = (ppu->attrib_hi & 0xFF00) | ((ppu->next_tile_attrib & 0x02) ? 0xFF : 0x00);
}

void shift(void) {
    if (ppu->mask & MASK_ENABLE_BACKGROUND) {
        ppu->pattern_lo <<= 1;
        ppu->pattern_hi <<= 1;
        ppu->attrib_lo <<= 1;
        ppu->attrib_hi <<= 1;
    }
    if (ppu->mask & MASK_ENABLE_SPRITE && ppu->cycle >= 1 && ppu->cycle < 258) {
        for (int i = 0; i < ppu->sprite_count; i++) {
            if (ppu->sprite_data[i].x > 0) {
                ppu->sprite_data[i].x--;
            } else {
                ppu->sprite_lo[i] <<= 1;
                ppu->sprite_hi[i] <<= 1;
            }
        }
    }
}

Color get_color_from_palette_ram(const uint8_t palette, const uint8_t pixel) {
    const uint16_t index = ppu_read(0x3F00 + (palette << 2) + pixel) & 0x3F;
    const Color color = NTSC[index];
    return color;
}

uint8_t flip(uint8_t byte) {
    // IA generated xD
    // 0b11100000 -> 0b00000111
    return (byte * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

void ppu_clock(void) {
    if (ppu->scanline >= -1 && ppu->scanline < 240) {
        if (ppu->scanline == 0 && ppu->cycle == 0) {
            ppu->cycle = 1;
        }

        if (ppu->scanline == -1 && ppu->cycle == 1) {
            ppu->status &= ~STATUS_VERTICAL_BLANK;
            ppu->status &= ~STATUS_SPRITE_OVERFLOW;
            ppu->status &= ~STATUS_SPRITE_ZERO_HIT;
            for (int i = 0; i < 8; i++) {
                ppu->sprite_lo[i] = 0;
                ppu->sprite_hi[i] = 0;
            }
        }

        if ((ppu->cycle >= 2 && ppu->cycle < 258) || (ppu->cycle >= 321 && ppu->cycle < 338)) {
            shift();
            switch ((ppu->cycle - 1) % 8) {
                case 0:
                    load_shifters();
                    ppu->next_tile_id = ppu_read(0x2000 | (VRAM_TO_UINT16 & ppu->vram_addr & 0x0FFF));
                    break;
                case 2:
                    ppu->next_tile_attrib = ppu_read(0x23C0 | (ppu->vram_addr.nametable_y << 11) | ppu->vram_addr.nametable_x << 10 |
                                                     ppu->vram_addr.y >> 2 << 3 | ppu->vram_addr.x >> 2);
                    if (ppu->vram_addr.y & 0x02)
                        ppu->next_tile_attrib >>= 4;
                    if (ppu->vram_addr.x & 0x02)
                        ppu->next_tile_attrib >>= 2;
                    ppu->next_tile_attrib &= 0x03;
                    break;
                case 4:
                    ppu->next_tile_lsb = ppu_read(((ppu->control & CONTROL_PATTERN_BACKGROUND) << 8) + ((uint16_t)ppu->next_tile_id << 4) +
                                                  ppu->vram_addr.fine_y);
                    break;
                case 6:
                    ppu->next_tile_msb = ppu_read(((ppu->control & CONTROL_PATTERN_BACKGROUND) << 8) + ((uint16_t)ppu->next_tile_id << 4) +
                                                  ppu->vram_addr.fine_y + 8);
                    break;
                case 7:
                    scroll_x();
                    break;
                default:
                    break;
            }
        }

        if (ppu->cycle == 256) {
            scroll_y();
        }

        if (ppu->cycle == 257) {
            load_shifters();
            transfer_x();
        }

        if (ppu->cycle == 338 || ppu->cycle == 340) {
            ppu->next_tile_id = ppu_read(0x2000 | (VRAM_TO_UINT16 & ppu->vram_addr & 0x0FFF));
        }

        if (ppu->scanline == -1 && ppu->cycle >= 280 && ppu->cycle < 305) {
            transfer_y();
        }
    }

    // Foreground
    if (ppu->cycle == 257 && ppu->scanline >= 0) {
        memset(ppu->sprite_data, 0xFF, 8 * sizeof(Sprite));
        ppu->sprite_count = 0;
        for (uint8_t i = 0; i < 8; i++) {
            ppu->sprite_lo[i] = 0;
            ppu->sprite_hi[i] = 0;
        }
        uint8_t oam = 0;

        ppu->can_zero_hit = false;

        while (oam < 64 && ppu->sprite_count < 9) {
            const int32_t diff = ppu->scanline - (int16_t)ppu->OAM[oam].y;

            if (diff >= 0 && diff < (ppu->control & CONTROL_SPRITE_SIZE ? 16 : 8)) {
                if (ppu->sprite_count < 8) {
                    if (oam == 0) {
                        ppu->can_zero_hit = true;
                    }
                    memcpy(&ppu->sprite_data[ppu->sprite_count], &ppu->OAM[oam], sizeof(Sprite));
                    ppu->sprite_count++;
                }
            }
            oam++;
        }

        if (ppu->sprite_count > 8) {
            ppu->status |= STATUS_SPRITE_OVERFLOW;
        } else {
            ppu->status &= ~STATUS_SPRITE_OVERFLOW;
        }
    }

    if (ppu->cycle == 340) {
        for (uint8_t i = 0; i < ppu->sprite_count; i++) {
            uint16_t sprite_pattern_addr_lo;
            const uint16_t y_position = ppu->scanline - ppu->sprite_data[i].y;
            const bool flipped_vertically = ppu->sprite_data[i].attribute & 0x80;

            if (!(ppu->control & CONTROL_SPRITE_SIZE)) {
                const uint16_t pattern_table_addr = (ppu->control & CONTROL_PATTERN_SPRITE) << 12;
                const uint16_t sprite_id_offset = ppu->sprite_data[i].id << 4;
                uint16_t sprite_row = flipped_vertically ? (7 - y_position) : y_position;

                sprite_pattern_addr_lo = pattern_table_addr | sprite_id_offset | sprite_row;
            } else {
                const uint16_t pattern_bank = (ppu->sprite_data[i].id & 0x01) << 12;
                const uint16_t row_within_tile = y_position & 0x07;
                uint16_t row_offset;
                uint16_t tile;

                if (flipped_vertically) {
                    row_offset = 7 - row_within_tile;

                    if (y_position < 8) {
                        tile = (ppu->sprite_data[i].id & 0xFE) + 1;
                    } else {
                        tile = ppu->sprite_data[i].id & 0xFE;
                    }
                } else {
                    row_offset = row_within_tile;

                    if (y_position < 8) {
                        tile = ppu->sprite_data[i].id & 0xFE;
                    } else {
                        tile = (ppu->sprite_data[i].id & 0xFE) + 1;
                    }
                }
                tile <<= 4;

                sprite_pattern_addr_lo = pattern_bank | tile | row_offset;
            }
            const uint16_t sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;
            uint8_t sprite_pattern_bits_lo = ppu_read(sprite_pattern_addr_lo);
            uint8_t sprite_pattern_bits_hi = ppu_read(sprite_pattern_addr_hi);
            if (ppu->sprite_data[i].attribute & 0x40) {
                sprite_pattern_bits_lo = flip(sprite_pattern_bits_lo);
                sprite_pattern_bits_hi = flip(sprite_pattern_bits_hi);
            }
            ppu->sprite_lo[i] = sprite_pattern_bits_lo;
            ppu->sprite_hi[i] = sprite_pattern_bits_hi;
        }
    }

    if (ppu->scanline >= 241 && ppu->scanline < 261) {
        if (ppu->scanline == 241 && ppu->cycle == 1) {
            ppu->status |= STATUS_VERTICAL_BLANK;
            if (ppu->control & CONTROL_ENABLE_NMI)
                ppu->nmi = true;
        }
    }

    // Background
    uint8_t bg_pixel = 0x00;
    uint8_t bg_palette = 0x00;

    if (ppu->mask & MASK_ENABLE_BACKGROUND) {
        const uint16_t mask = 0x8000 >> ppu->fine_x;

        const uint8_t p0 = (ppu->pattern_lo & mask) ? 1 : 0;
        const uint8_t p1 = (ppu->pattern_hi & mask) ? 1 : 0;
        bg_pixel = (p1 << 1) | p0;

        const uint8_t bg0 = (ppu->attrib_lo & mask) ? 1 : 0;
        const uint8_t bg1 = (ppu->attrib_hi & mask) ? 1 : 0;
        bg_palette = (bg1 << 1) | bg0;
    }

    // Foreground
    uint8_t fg_pixel = 0x00;
    uint8_t fg_palette = 0x00;
    uint8_t fg_priority = 0x00;

    if (ppu->mask & MASK_ENABLE_SPRITE) {
        ppu->sprite_zero_rendering = false;

        for (uint8_t i = 0; i < ppu->sprite_count; i++) {
            if (ppu->sprite_data[i].x != 0) {
                continue;
            }

            const uint8_t fg_pixel_lo = (ppu->sprite_lo[i] & 0x80) ? 1 : 0;
            const uint8_t fg_pixel_hi = (ppu->sprite_hi[i] & 0x80) ? 1 : 0;
            const uint8_t pixel = (fg_pixel_hi << 1) | fg_pixel_lo;

            if (pixel == 0)
                continue;

            fg_pixel = pixel;
            fg_palette = (ppu->sprite_data[i].attribute & 0x03) + 0x04;
            fg_priority = (ppu->sprite_data[i].attribute & 0x20) == 0;

            if (i == 0)
                ppu->sprite_zero_rendering = true;

            break;
        }
    }

    uint8_t pixel = 0x00;
    uint8_t palette = 0x00;

    if (bg_pixel == 0 && fg_pixel == 0) {
        pixel = 0x00;
        palette = 0x00;
    } else if (bg_pixel == 0 && fg_pixel > 0) {
        pixel = fg_pixel;
        palette = fg_palette;
    } else if (bg_pixel > 0 && fg_pixel == 0) {
        pixel = bg_pixel;
        palette = bg_palette;
    } else if (bg_pixel > 0 && fg_pixel > 0) {
        if (fg_priority) {
            pixel = fg_pixel;
            palette = fg_palette;
        } else {
            pixel = bg_pixel;
            palette = bg_palette;
        }

        if (ppu->can_zero_hit && ppu->sprite_zero_rendering) {
            if (ppu->mask & (MASK_ENABLE_BACKGROUND | MASK_ENABLE_SPRITE)) {
                const uint16_t min_visible_cycle = ppu->mask & (MASK_SHOW_BACKGROUND_LEFT | MASK_SHOW_SPRITE_LEFT) ? 1 : 9;

                if (ppu->cycle >= min_visible_cycle && ppu->cycle < 258) {
                    ppu->status |= STATUS_SPRITE_ZERO_HIT;
                }
            }
        }
    }

    if (ppu->scanline >= 0) {
        const Color color = get_color_from_palette_ram(palette, pixel);
        const int posY = (255 - ppu->scanline); // RayLib
        DrawPixel(ppu->cycle - 1, posY, color);
    }

    ppu->cycle++;
    if (ppu->cycle >= 341) {
        ppu->cycle = 0;
        ppu->scanline++;
        if (ppu->scanline >= 261) {
            ppu->scanline = -1;
            ppu->frame_complete = true;
        }
    }
}

void ppu_reset(void) {
    ppu->nmi = false;
    ppu->frame_complete = false;
    ppu->fine_x = 0x00;
    ppu->address_latch = 0x00;
    ppu->data_buffer = 0x00;
    ppu->scanline = 0;
    ppu->cycle = 0;
    ppu->next_tile_id = 0x00;
    ppu->next_tile_attrib = 0x00;
    ppu->next_tile_lsb = 0x00;
    ppu->next_tile_msb = 0x00;
    ppu->pattern_lo = 0x0000;
    ppu->pattern_hi = 0x0000;
    ppu->attrib_lo = 0x0000;
    ppu->attrib_hi = 0x0000;
    ppu->status = 0x00;
    ppu->mask = 0x00;
    ppu->control = 0x00;
    VRAM_TO_UINT16 &ppu->vram_addr = 0x0000;
    VRAM_TO_UINT16 &ppu->temp_vram_addr = 0x0000;
    ppu->oam_addr = 0x00;
    ppu->can_zero_hit = false;
    ppu->sprite_zero_rendering = false;

    ppu->OAM_pointer = (uint8_t *)ppu->OAM;

    // Clean rendering and nametables
    for (int i = 0; i < 1024; i++) {
        ppu->nametable[0][i] = 0;
        ppu->nametable[1][i] = 0;
    }
    for (int i = 0; i < 32; i++)
        ppu->palette[i] = 0;
    BeginTextureMode(ppu->texture_screen);
    ClearBackground(BLACK);
    EndTextureMode();
}

uint8_t ppu_read_debug(const uint16_t addr) {
    uint8_t data = 0x00;
    switch (addr) {
        case 0x0000: // Control
            data = ppu->control;
            break;
        case 0x0001: // Mask
            data = ppu->mask;
            break;
        case 0x0002: // Status
            data = ppu->status;
            break;
        default:
            break;
    }
    return data;
}

uint8_t ppu_read(uint16_t addr) {
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        data = ppu->cart->ppu_read(addr);
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;

        if (ppu->cart->mirror == VERTICAL) {
            // Vertical
            if (/*addr >= 0x0000 &&*/ addr <= 0x03FF)
                data = ppu->nametable[0][addr & 0x03FF];
            if (addr >= 0x0400 && addr <= 0x07FF)
                data = ppu->nametable[1][addr & 0x03FF];
            if (addr >= 0x0800 && addr <= 0x0BFF)
                data = ppu->nametable[0][addr & 0x03FF];
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                data = ppu->nametable[1][addr & 0x03FF];
        } else if (ppu->cart->mirror == HORIZONTAL) {
            // Horizontal
            if (/*addr >= 0x0000 &&*/ addr <= 0x03FF)
                data = ppu->nametable[0][addr & 0x03FF];
            if (addr >= 0x0400 && addr <= 0x07FF)
                data = ppu->nametable[0][addr & 0x03FF];
            if (addr >= 0x0800 && addr <= 0x0BFF)
                data = ppu->nametable[1][addr & 0x03FF];
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                data = ppu->nametable[1][addr & 0x03FF];
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x0010)
            addr = 0x0000;
        if (addr == 0x0014)
            addr = 0x0004;
        if (addr == 0x0018)
            addr = 0x0008;
        if (addr == 0x001C)
            addr = 0x000C;
        data = ppu->palette[addr] & (ppu->mask & MASK_GRAYSCALE ? 0x30 : 0x3F);
    }

    return data;
}

void ppu_write(uint16_t addr, const uint8_t data) {
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        ppu->cart->ppu_write(addr, data);
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        if (ppu->cart->mirror == VERTICAL) {
            // Vertical
            if (addr <= 0x03FF)
                ppu->nametable[0][addr & 0x03FF] = data;
            if (addr >= 0x0400 && addr <= 0x07FF)
                ppu->nametable[1][addr & 0x03FF] = data;
            if (addr >= 0x0800 && addr <= 0x0BFF)
                ppu->nametable[0][addr & 0x03FF] = data;
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                ppu->nametable[1][addr & 0x03FF] = data;
        } else if (ppu->cart->mirror == HORIZONTAL) {
            // Horizontal
            if (addr <= 0x03FF)
                ppu->nametable[0][addr & 0x03FF] = data;
            if (addr >= 0x0400 && addr <= 0x07FF)
                ppu->nametable[0][addr & 0x03FF] = data;
            if (addr >= 0x0800 && addr <= 0x0BFF)
                ppu->nametable[1][addr & 0x03FF] = data;
            if (addr >= 0x0C00 && addr <= 0x0FFF)
                ppu->nametable[1][addr & 0x03FF] = data;
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint8_t addr2 = (uint8_t)(addr & 0x001F);
        if (addr2 == 0x0010)
            addr2 = 0x0000;
        if (addr2 == 0x0014)
            addr2 = 0x0004;
        if (addr2 == 0x0018)
            addr2 = 0x0008;
        if (addr2 == 0x001C)
            addr2 = 0x000C;
        ppu->palette[addr2] = data;
    }
}

uint8_t ppu_cpu_read(uint16_t addr) {
    uint8_t data = 0x00;
    switch (addr) {
        case 0x0002:
            data = (ppu->status & 0xE0) | (ppu->data_buffer & 0x1F);
            ppu->status &= ~STATUS_VERTICAL_BLANK;
            ppu->address_latch = 0;
            break;
        case 0x0004:
            data = ppu->OAM_pointer[ppu->oam_addr];
            break;
        case 0x0007:
            data = ppu->data_buffer;
            VRAM_TO_UINT16 &ppu->data_buffer = ppu_read(VRAM_TO_UINT16 & ppu->vram_addr);
            if (VRAM_TO_UINT16 & ppu->vram_addr >= 0x3F00)
                data = ppu->data_buffer;
            VRAM_TO_UINT16 &ppu->vram_addr += ((ppu->control & CONTROL_INCREMENT_MODE) ? 32 : 1);
            break;
        default:
            break;
    }
    return data;
}

void ppu_cpu_write(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x0000: // Control
            ppu->control = data;
            ppu->temp_vram_addr.nametable_x = ppu->control & CONTROL_NAMETABLE_X;
            ppu->temp_vram_addr.nametable_y = ppu->control & CONTROL_NAMETABLE_Y;
            break;
        case 0x0001: // Mask
            ppu->mask = data;
            break;
        case 0x0003: // OAM Address
            ppu->oam_addr = data;
            break;
        case 0x0004: // OAM Data
            ppu->OAM_pointer[ppu->oam_addr] = data;
            break;
        case 0x0005: // Scroll
            if (ppu->address_latch == 0) {
                ppu->fine_x = data & 0x07;
                ppu->temp_vram_addr.x = data >> 3;
                ppu->address_latch = 1;
            } else {
                ppu->temp_vram_addr.fine_y = data & 0x07;
                ppu->temp_vram_addr.y = data >> 3;
                ppu->address_latch = 0;
            }
            break;
        case 0x0006: // PPU Address
            if (ppu->address_latch == 0) {
                VRAM_TO_UINT16 &ppu->temp_vram_addr = (uint16_t)((data & 0x3F) << 8) | (VRAM_TO_UINT16 & ppu->temp_vram_addr & 0x00FF);
                ppu->address_latch = 1;
            } else {
                VRAM_TO_UINT16 &ppu->temp_vram_addr = (VRAM_TO_UINT16 & ppu->temp_vram_addr & 0xFF00) | data;
                ppu->vram_addr = ppu->temp_vram_addr;
                ppu->address_latch = 0;
            }
            break;
        case 0x0007: // PPU Data
            ppu_write(VRAM_TO_UINT16 & ppu->vram_addr, data);
            VRAM_TO_UINT16 &ppu->vram_addr += ppu->control & CONTROL_INCREMENT_MODE ? 32 : 1;
            break;
        default:
            break;
    }
}
