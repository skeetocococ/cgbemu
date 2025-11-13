#ifndef GB_H
#define GB_H

#include "../cpu/cpu.h"
#include <SDL2/SDL.h>

typedef struct {
    char* game_path;
    char* boot_path;
} Options;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} SDL_Context;

Options parse_cli(int count, char** args);
SDL_Context init_sdl();
void sdl_cleanup(SDL_Context* context);
void boot(CPU* cpu, Options* opts);
void load_game(Options* opts, char** args);
void emu_loop(CPU* cpu, PPU* ppu, SDL_Context* context);

#endif
