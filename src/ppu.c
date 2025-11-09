#include "ppu.h"
#include "cpu.h"
#include "memory.h"

static uint8_t get_background_color_id(PPU *ppu, int x, int y)
{   
    // scroll
    uint16_t bg_x = (x + ppu->SCX) & 0xFF;
    uint16_t bg_y = (y + ppu->SCY) & 0xFF;

    // Tile indices
    uint8_t tile_x = bg_x / 8;
    uint8_t tile_y = bg_y / 8;

    // Tilemap address
    uint16_t tilemap_base = (ppu->LCDC & 0x08) ? 0x9C00 : 0x9800;
    uint16_t tilemap_index = tile_y * 32 + tile_x;
    uint8_t tile_id = vram[tilemap_base - VRAM_START + tilemap_index];

    // Tile data address
    uint16_t tiledata_base = (ppu->LCDC & 0x10) ? VRAM_START : TILESET2_START;
    uint16_t tile_addr;
    if (tiledata_base == VRAM_START)
        tile_addr = tiledata_base - VRAM_START + tile_id * 16; // Unsigned index
    else 
    {
        // signed indexing (-128..127)
        int8_t signed_id = (int8_t)tile_id;
        tile_addr = 0x9000 - VRAM_START + signed_id * 16;
    }

    // Row inside tile
    int pixel_y = bg_y % 8;
    uint8_t low  = vram[tile_addr + pixel_y * 2];
    uint8_t high = vram[tile_addr + pixel_y * 2 + 1];

    // Pixel inside row
    int pixel_x = bg_x % 8;
    int bit = 7 - pixel_x;
    uint8_t color = ((high >> bit) & 1) << 1 | ((low >> bit) & 1);

    return color; // 0..3
}

static uint8_t get_window_color_id(PPU *ppu, int x, int y)
{
    // Window is disabled or not yet visible
    if (!(ppu->LCDC & 0x20)) return 0;

    int wx = ppu->WX - 7; // WX is offset by 7
    int wy = ppu->WY;

    if (x < wx || y < wy) return 0; // Not inside window yet

    int win_x = x - wx;
    int win_y = y - wy;

    // Tile indices
    uint8_t tile_x = win_x / 8;
    uint8_t tile_y = win_y / 8;

    // Window tilemap base
    uint16_t tilemap_base = (ppu->LCDC & 0x40) ? 0x9C00 : 0x9800;
    uint16_t tilemap_index = tile_y * 32 + tile_x;
    uint8_t tile_id = vram[tilemap_base - VRAM_START + tilemap_index];

    // Tile data base
    uint16_t tiledata_base = (ppu->LCDC & 0x10) ? VRAM_START : TILESET2_START;
    uint16_t tile_addr;
    if (tiledata_base == VRAM_START) 
        tile_addr = VRAM_START - VRAM_START + tile_id * 16;
    else 
    {
        int8_t signed_id = (int8_t)tile_id;
        tile_addr = 0x9000 - VRAM_START + signed_id * 16;
    }

    // Pixel inside tile
    int pixel_y = win_y % 8;
    int pixel_x = win_x % 8;
    uint8_t low  = vram[tile_addr + pixel_y * 2];
    uint8_t high = vram[tile_addr + pixel_y * 2 + 1];
    int bit = 7 - pixel_x;

    uint8_t color = ((high >> bit) & 1) << 1 | ((low >> bit) & 1);
    return color; // 0..3
}

static void render_scanline(PPU* ppu)
{
    int y = ppu->line;
    if (y >= 144) return; // only visible lines
    if (!(ppu->LCDC & 0x80)) return; // LCD off

    uint8_t bg_ids[160]; // sprite priority
    uint32_t fb_line[160]; // final RGB values

    for (int x = 0; x < 160; x++)
    {
        uint8_t color_id = 0;

        if (ppu->LCDC & 0x01)
            color_id = get_background_color_id(ppu, x, y);

        uint32_t final_color = ppu->bg_palette[color_id];

        if (ppu->LCDC & 0x20) // Window enabled
        {
            int wx = ppu->WX - 7;
            int wy = ppu->WY;
            if (x >= wx && y >= wy)
            {
                color_id = get_window_color_id(ppu, x, y);
                final_color = ppu->bg_palette[color_id];
            }
        }

        bg_ids[x] = color_id;    // sprite priority
        fb_line[x] = final_color; // final RGB value
    }

    if (ppu->LCDC & 0x02) // OBJ enabled
    {
        int sprite_height = (ppu->LCDC & 0x04) ? 16 : 8;

        for (int i = 0; i < 40; i++)
        {
            uint8_t y_spr = oam[i*4] - 16;
            uint8_t x_spr = oam[i*4 + 1] - 8;
            uint8_t tile_id = oam[i*4 + 2];
            uint8_t attr = oam[i*4 + 3];

            if (y < y_spr || y >= y_spr + sprite_height) continue;

            int pixel_y = y - y_spr;
            if (attr & 0x40) pixel_y = sprite_height - 1 - pixel_y; // Y flip

            if (sprite_height == 16) tile_id &= 0xFE; // 8x16 sprite, ignore LSB

            uint16_t tile_addr = VRAM_START - VRAM_START + tile_id * 16 + pixel_y * 2;
            uint8_t low  = vram[tile_addr];
            uint8_t high = vram[tile_addr + 1];

            for (int px = 0; px < 8; px++)
            {
                int pixel_x = (attr & 0x20) ? 7 - px : px; // X flip
                uint8_t color_id = ((high >> (7 - pixel_x)) & 1) << 1 | ((low >> (7 - pixel_x)) & 1);
                if (color_id == 0) continue; // transparent

                int fb_x = x_spr + px;
                if (fb_x < 0 || fb_x >= 160) continue;

                // Priority: OBJ-to-BG
                // If bit 7 is set and background color ID is non-zero, skip
                if ((attr & 0x80) && bg_ids[fb_x] != 0) continue;

                int palette_index = (attr & 0x10) ? 1 : 0;
                fb_line[fb_x] = ppu->sprite_palette[palette_index][color_id];
            }
        }
    }

    // Copy line to framebuffer
    for (int x = 0; x < 160; x++)
        ppu->framebuffer[y][x] = fb_line[x];
}

void ppu_init(PPU *ppu)
{
    ppu->mode = OAM;
    ppu->mode_clock = 0;
    ppu->line = 0;
    ppu->SCX = 0;
    ppu->SCY = 0;
    ppu->WX = 0;
    ppu->WY = 0;
    ppu->LCDC = 0x91; // LCD enabled, BG enabled, OBJ enabled, 8x8 sprites

    // BG palette: map 0..3 to grayscale RGB
    ppu->bg_palette[0] = 0xFFFFFFFF; // white
    ppu->bg_palette[1] = 0xAAAAAAFF; // light gray
    ppu->bg_palette[2] = 0x555555FF; // dark gray
    ppu->bg_palette[3] = 0x000000FF; // black

    for (int p = 0; p < 2; p++)
    {
        ppu->sprite_palette[p][0] = ppu->bg_palette[0];
        ppu->sprite_palette[p][1] = ppu->bg_palette[1];
        ppu->sprite_palette[p][2] = ppu->bg_palette[2];
        ppu->sprite_palette[p][3] = ppu->bg_palette[3];
    }

    for (int y = 0; y < 144; y++)
        for (int x = 0; x < 160; x++)
            ppu->framebuffer[y][x] = ppu->bg_palette[0];
}

void ppu_step(PPU *ppu, int cycles)
{
    ppu->mode_clock += cycles;
    switch(ppu->mode)
    {
        case OAM:
            if (ppu->mode_clock >= 80)
            {
                ppu->mode_clock -= 80;
                memory[0xFF44] = ppu->line;
                ppu->mode = VRAM;
                // Update STAT mode bits (0b10 for mode 2)
                memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x02;
            }
            break;
        case VRAM:
            if (ppu->mode_clock >= 172)
            {
                ppu->mode_clock -= 172;
                render_scanline(ppu); 
                memory[0xFF44] = ppu->line;
                ppu->mode = HBLANK;
                // Update STAT mode bits (0b00 for mode 0)
                memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x00;
                // HBLANK interrupt
                if (memory[0xFF41] & 0x08)
                    request_interrupt(STAT_INT);
            }
            break;
        case HBLANK:
            if (ppu->mode_clock >= 204)
            {
                ppu->mode_clock -= 204;
                ppu->line++;
                memory[0xFF44] = ppu->line;
                if (ppu->line == 144)
                {
                    ppu->mode = VBLANK;
                    request_interrupt(VBLANK_INT);
                    // Update STAT mode bits (0b01 for mode 1)
                    memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x01;
                }
                else
                {
                    ppu->mode = OAM;
                    // Update STAT mode bits (0b10 for mode 2)
                    memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x02;
                }

                // LY=LYC check
                if (ppu->line == memory[0xFF45] && (memory[0xFF41] & 0x40))
                    request_interrupt(STAT_INT);
            }
            break;
        case VBLANK:
            if (ppu->mode_clock >= 456)
            {
                ppu->mode_clock -= 456;
                ppu->line++;
                memory[0xFF44] = ppu->line;
                if (ppu->line > 153)
                {
                    ppu->line = 0;
                    memory[0xFF44] = 0;
                    ppu->mode = OAM;
                    // Update STAT mode bits (0b10 for mode 2)
                    memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x02;
                }
                // LY=LYC check during VBLANK
                if (ppu->line == memory[0xFF45] && (memory[0xFF41] & 0x40))
                    request_interrupt(STAT_INT);
            }
            break;
    }
}
