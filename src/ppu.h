#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <SDL2/SDL.h>

typedef enum { OAM, VRAM, HBLANK, VBLANK } Mode;

typedef struct {
    Mode mode;
    uint16_t mode_clock;
    uint16_t line;
    uint32_t framebuffer[144][160];
    uint32_t bg_palette[4], sprite_palette[2][4];
    uint8_t SCX, SCY, LCDC;
    uint8_t WX, WY;
    uint8_t frame_ready;
} PPU;

void ppu_init(PPU* ppu);
void ppu_step(PPU* ppu, int cycles);
void render_frame(SDL_Renderer* renderer, SDL_Texture* texture, uint32_t framebuffer[144][160]);

#endif
