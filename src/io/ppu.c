#include "ppu.h"
#include "../cpu/cpu.h"
#include "../memory.h"
#include "../debug/debug.h"
#include <SDL2/SDL.h>

static uint8_t get_background_color_id(PPU *ppu, int x, int y)
{
    // Scroll
    uint16_t bg_x = (x + ppu->SCX) & 0xFF;
    uint16_t bg_y = (y + ppu->SCY) & 0xFF;

    // Tile index inside the map
    uint8_t tile_x = bg_x / 8;
    uint8_t tile_y = bg_y / 8;

    uint16_t tilemap_base = (ppu->LCDC & 0x08) ? 0x9C00 : 0x9800;
    uint16_t tilemap_addr = tilemap_base + tile_y * 32 + tile_x;

    uint8_t tile_id = memory[tilemap_addr];

    // Tile data base
    uint8_t unsigned_mode = (ppu->LCDC & 0x10) != 0;

    uint16_t tile_addr;

    if (unsigned_mode) 
    {
        // 0x8000 mode, unsigned indexing
        tile_addr = 0x8000 + tile_id * 16;
    } else 
    {
        // 0x8800 mode, signed indexing
        int8_t id_signed = (int8_t)tile_id;
        tile_addr = 0x9000 + (id_signed * 16);
    }

    // Pixel row inside the tile
    int row = bg_y & 7;

    uint8_t low  = memory[tile_addr + row * 2];
    uint8_t high = memory[tile_addr + row * 2 + 1];

    int bit = 7 - (bg_x & 7);
    return ((high >> bit) & 1) << 1 | ((low >> bit) & 1);
}

static void render_scanline(PPU* ppu)
{
    int y = ppu->line;
    if (y >= 144) return; // only visible lines
    if (!(ppu->LCDC & 0x80)) 
    {
        // LCD off - display white
        for (int x = 0; x < 160; x++)
            ppu->framebuffer[y][x] = 0xFFFFFFFF;
        return;
    }

    if (debug && y == 0)
    {
        DBG_PRINT("=== Rendering scanline 0 ===\n");
        DBG_PRINT("LCDC=%02X BGP=%02X\n", ppu->LCDC, memory[0xFF47]);
        DBG_PRINT("SCX=%d SCY=%d\n", ppu->SCX, ppu->SCY);
        
        // Check first tile
        uint16_t tilemap_base = (ppu->LCDC & 0x08) ? 0x9C00 : 0x9800;
        DBG_PRINT("Tilemap base: 0x%04X\n", tilemap_base);
        DBG_PRINT("First tile ID: 0x%02X\n", memory[tilemap_base]);
        
        // Check tile data
        uint8_t tile_id = memory[tilemap_base];
        uint16_t tile_addr;
        if (ppu->LCDC & 0x10) 
        {
            tile_addr = 0x8000 + tile_id * 16;
        } else 
        {
            tile_addr = 0x9000 + ((int8_t)tile_id * 16);
        }
        DBG_PRINT("First tile address: 0x%04X\n", tile_addr);
        DBG_PRINT("First tile data: %02X %02X\n", memory[tile_addr], memory[tile_addr+1]);
        
        uint8_t color_id = get_background_color_id(ppu, 0, 0);
        DBG_PRINT("First pixel color_id: %d\n", color_id);
        DBG_PRINT("Palette mapping: 0->%d 1->%d 2->%d 3->%d\n",
               (memory[0xFF47] >> 0) & 3,
               (memory[0xFF47] >> 2) & 3,
               (memory[0xFF47] >> 4) & 3,
               (memory[0xFF47] >> 6) & 3);
    }

    uint8_t bg_ids[160]; // for sprite priority
    uint32_t fb_line[160]; // final RGB values

    uint8_t bgp = memory[0xFF47];
    static const uint32_t colors[4] = {
        0xFFFFFFFF, // White (lightest)
        0xAAAAAAFF, // Light gray
        0x555555FF, // Dark gray
        0x000000FF  // Black (darkest)
    };

    // Render background
    for (int x = 0; x < 160; x++)
    {
        uint8_t color_id = 0;

        if (ppu->LCDC & 0x01)
            color_id = get_background_color_id(ppu, x, y);

        // Map color_id through palette
        uint8_t shade = (bgp >> (color_id * 2)) & 0x03;
        uint32_t final_color = colors[shade];

        bg_ids[x] = color_id;
        fb_line[x] = final_color;
    }

    // Render window (if enabled)
    if (ppu->LCDC & 0x20) 
    {
        int wx = ppu->WX - 7;
        int wy = ppu->WY;
        
        if (y >= wy) 
        {
            for (int x = 0; x < 160; x++) 
            {
                if (x >= wx) 
                {
                    int win_x = x - wx;
                    int win_y = y - wy;

                    uint8_t tile_x = win_x / 8;
                    uint8_t tile_y = win_y / 8;

                    uint16_t tilemap_base = (ppu->LCDC & 0x40) ? 0x9C00 : 0x9800;
                    uint16_t tilemap_addr = tilemap_base + tile_y * 32 + tile_x;

                    uint8_t tile_id = memory[tilemap_addr];

                    uint16_t tile_addr;
                    if (ppu->LCDC & 0x10) 
                        tile_addr = 0x8000 + tile_id * 16;
                    else 
                    {
                        int8_t signed_id = (int8_t)tile_id;
                        tile_addr = 0x9000 + signed_id * 16;
                    }

                    int row = win_y & 7;
                    uint8_t low  = memory[tile_addr + row * 2];
                    uint8_t high = memory[tile_addr + row * 2 + 1];

                    int bit = 7 - (win_x & 7);
                    uint8_t color_id = ((high >> bit) & 1) << 1 | ((low >> bit) & 1);
                    
                    uint8_t shade = (bgp >> (color_id * 2)) & 0x03;
                    fb_line[x] = colors[shade];
                    bg_ids[x] = color_id;
                }
            }
        }
    }

    // Render sprites (if enabled)
    if (ppu->LCDC & 0x02) 
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

            if (sprite_height == 16) tile_id &= 0xFE;

            uint16_t tile_addr = 0x8000 + tile_id * 16 + pixel_y * 2;
            uint8_t low  = memory[tile_addr];
            uint8_t high = memory[tile_addr + 1];

            for (int px = 0; px < 8; px++)
            {
                int pixel_x = (attr & 0x20) ? 7 - px : px; // X flip
                uint8_t color_id = ((high >> (7 - pixel_x)) & 1) << 1 | ((low >> (7 - pixel_x)) & 1);
                if (color_id == 0) continue; // transparent

                int fb_x = x_spr + px;
                if (fb_x < 0 || fb_x >= 160) continue;

                // Priority: OBJ-to-BG
                if ((attr & 0x80) && bg_ids[fb_x] != 0) continue;

                int palette_index = (attr & 0x10) ? 1 : 0;
                uint8_t obp = memory[palette_index ? 0xFF49 : 0xFF48];
                uint8_t shade = (obp >> (color_id * 2)) & 0x03;
                fb_line[fb_x] = colors[shade];
            }
        }
    }

    // Copy line to framebuffer
    for (int x = 0; x < 160; x++)
        ppu->framebuffer[y][x] = fb_line[x];
}

void render_frame(SDL_Renderer* renderer, SDL_Texture* texture, uint32_t framebuffer[144][160])
{
    SDL_UpdateTexture(texture, NULL, framebuffer, 160 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void ppu_init(PPU *ppu)
{
    ppu->mode = OAM;
    ppu->mode_clock = 0;
    ppu->frame_ready = 0;
    ppu->line = 0;
    ppu->SCX = 0;
    ppu->SCY = 0;
    ppu->WX = 0;
    ppu->WY = 0;

    // Initialize framebuffer to white
    for (int y = 0; y < 144; y++)
        for (int x = 0; x < 160; x++)
            ppu->framebuffer[y][x] = 0xFFFFFFFF;
}

void ppu_step(PPU *ppu, int cycles)
{
    ppu->LCDC = memory[0xFF40];
    ppu->SCX = memory[0xFF43];
    ppu->SCY = memory[0xFF42];
    ppu->WX = memory[0xFF4B];
    ppu->WY = memory[0xFF4A];

    ppu->mode_clock += cycles;

    if (ppu->LCDC & 0x80)  // LCD enabled
    {
        switch(ppu->mode)
        {
            case OAM:
                if (ppu->mode_clock >= 80)
                {
                    ppu->mode_clock -= 80;
                    ppu->mode = VRAM;
                    memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x03;  // Mode 3
                }
                break;
                
            case VRAM:
                if (ppu->mode_clock >= 172)
                {
                    ppu->mode_clock -= 172;
                    render_scanline(ppu);
                    ppu->mode = HBLANK;
                    memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x00;  // Mode 0
                    
                    // STAT interrupt for HBLANK
                    if (memory[0xFF41] & 0x08)
                        request_interrupt(STAT_INT);
                }
                break;
                
            case HBLANK:
                if (ppu->mode_clock >= 204)
                {
                    ppu->mode_clock -= 204;
                    ppu->line++;
                    memory[0xFF44] = ppu->line;  // Update LY register
                    
                    if (ppu->line == 144)  // Last visible scanline
                    {
                        ppu->mode = VBLANK;
                        request_interrupt(VBLANK_INT);  // VBLANK interrupt
                        ppu->frame_ready = 1;
                        memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x01;  // Mode 1
                    }
                    else
                    {
                        ppu->mode = OAM;
                        memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x02;  // Mode 2
                    }
                    
                    // LY=LYC coincidence interrupt
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
                    
                    if (ppu->line > 153)  // End of VBLANK
                    {
                        ppu->line = 0;
                        ppu->mode = OAM;
                        memory[0xFF41] = (memory[0xFF41] & 0xFC) | 0x02;  // Mode 2
                    }
                    
                    // LY=LYC coincidence interrupt
                    if (ppu->line == memory[0xFF45] && (memory[0xFF41] & 0x40))
                        request_interrupt(STAT_INT);
                }
                break;
        }
    }
    else  // LCD disabled
    {
        // When LCD is off, reset to initial state but still update LY
        ppu->mode = OAM;
        ppu->mode_clock = 0;
        
        // Basic LY update for boot ROM compatibility
        if (ppu->mode_clock >= 456) {
            ppu->mode_clock -= 456;
            ppu->line++;
            if (ppu->line > 153) {
                ppu->line = 0;
            }
            memory[0xFF44] = ppu->line;
        }
    }
}
