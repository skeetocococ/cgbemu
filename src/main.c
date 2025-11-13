#include "cpu/cpu.h"
#include "cpu/opcodes.h"
#include "io/ppu.h"
#include "core/gb.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

int main(int argc, char** argv)
{ 
    Options opts = parse_cli(argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Context context = init_sdl();
    load_game(&opts, argv);
    
    CPU cpu;
    PPU ppu;

    init_opcodes();
    cpu_init(&cpu);
    ppu_init(&ppu);
    boot(&cpu, &opts);
    emu_loop(&cpu, &ppu, &context);
    sdl_cleanup(&context);
    
    return 0;
}
