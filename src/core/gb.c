#include "gb.h"
#include "../memory/memory.h"
#include "../cpu/cpu.h"
#include "../io/ppu.h"
#include "../io/joypad.h"
#include "../debug/debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Options parse_cli(int count, char** args)
{
    Options opts = { NULL, NULL };
    for (int i = 1; i < count; i++) 
    {
        if (strcmp(args[i], "--debug") == 0 
            || strcmp(args[i], "-d") == 0) 
        {
            debug = 1;
            continue;
        }
        if (strcmp(args[i], "-dCPU") == 0)
            dbg.dbg_cpu = 1;
        if (strcmp(args[i], "-dPPU") == 0)
            dbg.dbg_ppu = 1;
        if (strcmp(args[i], "-dBOOT") == 0)
            dbg.dbg_boot = 1;
        if (strcmp(args[i], "-dMEM") == 0)
            dbg.dbg_mem = 1;
    
        if (!opts.game_path) 
        {
            opts.game_path = args[i];
            continue;
        }
    
        if (!opts.boot_path) 
        {
            opts.boot_path = args[i];
            continue;
        }
    }
    
    if (!opts.game_path) 
    {
        printf("Usage: %s [--debug] <game.gb> [boot.gb]\n", args[0]);
        exit(EXIT_FAILURE);
    }
    return opts; 
}

SDL_Context init_sdl()
{
    SDL_Window* window = SDL_CreateWindow("Game Boy Emulator",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      160 * 3,
                                      144 * 3,
                                      SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                     SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     160, 144);
    SDL_Context context = { window, renderer, texture };
    return context;
}

void cleanup_sdl(SDL_Context* context)
{
    SDL_DestroyTexture(context->texture);
    SDL_DestroyRenderer(context->renderer);
    SDL_DestroyWindow(context->window);
    SDL_Quit();
}

static void bootstrap(char* rom)
{
    FILE* bootstrap = fopen(rom, "rb");
    if (!bootstrap)
    {
        fprintf(stderr, "Failed to initialize bootstrap ROM.\n");
        exit(EXIT_FAILURE);
    }
    
    size_t bootstrap_size = fread(boot_rom, 1, 0x0100, bootstrap);
    fclose(bootstrap);
    
    if (bootstrap_size != 0x0100) 
    {
        fprintf(stderr, "Bootstrap ROM incomplete. (read %zu bytes)\n", bootstrap_size);
        exit(EXIT_FAILURE);
    }
    
    printf("Bootstrap ROM loaded. (%zu bytes)\n", bootstrap_size);
    bootstrap_enabled = 1;
}

static void init_hardware_regs()
{
    // Initialize hardware registers to post-boot values
    memory[0xFF05] = 0x00;   // TIMA
    memory[0xFF06] = 0x00;   // TMA
    memory[0xFF07] = 0x00;   // TAC
    memory[0xFF10] = 0x80;   // NR10
    memory[0xFF11] = 0xBF;   // NR11
    memory[0xFF12] = 0xF3;   // NR12
    memory[0xFF14] = 0xBF;   // NR14
    memory[0xFF16] = 0x3F;   // NR21
    memory[0xFF17] = 0x00;   // NR22
    memory[0xFF19] = 0xBF;   // NR24
    memory[0xFF1A] = 0x7F;   // NR30
    memory[0xFF1B] = 0xFF;   // NR31
    memory[0xFF1C] = 0x9F;   // NR32
    memory[0xFF1E] = 0xBF;   // NR33
    memory[0xFF20] = 0xFF;   // NR41
    memory[0xFF21] = 0x00;   // NR42
    memory[0xFF22] = 0x00;   // NR43
    memory[0xFF23] = 0xBF;   // NR30
    memory[0xFF24] = 0x77;   // NR50
    memory[0xFF25] = 0xF3;   // NR51
    memory[0xFF26] = 0xF1;   // NR52
    memory[0xFF40] = 0x91;   // LCDC - LCD enabled, BG on
    memory[0xFF42] = 0x00;   // SCY
    memory[0xFF43] = 0x00;   // SCX
    memory[0xFF44] = 0x00;   // LY
    memory[0xFF45] = 0x00;   // LYC
    memory[0xFF47] = 0xFC;   // BGP - Background palette
    memory[0xFF48] = 0xFF;   // OBP0
    memory[0xFF49] = 0xFF;   // OBP1
    memory[0xFF4A] = 0x00;   // WY
    memory[0xFF4B] = 0x00;   // WX
    memory[0xFF0F] = 0x00;   // IF - Interrupt flags
    memory[0xFFFF] = 0x00;   // IE - Interrupt Enable
}

void boot(CPU* cpu, Options* opts)
{
    if (opts->boot_path != NULL)
    {
        bootstrap(opts->boot_path);
        printf("Boot ROM enabled. First 16 bytes:\n");
        for (int i = 0; i < 16; i++) 
        {
            printf("%02X ", read_byte(i));
            if (i == 7) printf("\n");
        }
        printf("\n");
    }
    else
    {
        // No boot ROM - initialize hardware registers and jump to 0x0100
        init_hardware_regs();
        cpu->PC = 0x0100;
        printf("Skipping boot ROM, starting at 0x0100\n");
        printf("First 16 ROM bytes at 0x0100:\n");
        for (int i = 0; i < 16; i++) 
        {
            printf("%02X ", read_byte(0x0100 + i));
            if (i == 7) printf("\n");
        }
        printf("\n");
    }
    printf("Initial LCDC: 0x%02X\n", memory[0xFF40]);
    printf("Initial SCX: %d, SCY: %d\n", memory[0xFF43], memory[0xFF42]);
    printf("Initial BGP: 0x%02X\n", memory[0xFF47]);
    printf("Starting PC: 0x%04X, SP: 0x%04X\n", REG_PC, REG_SP);
}

void load_game(Options* opts, char** args)
{
    FILE* game_rom = fopen(opts->game_path, "rb");
    if (!game_rom)
    {
        fprintf(stderr, "Failed to open ROM file: %s\n", args[1]);
        exit(EXIT_FAILURE);
    }
    size_t game_size = fread(memory, 1, MEM_SIZE, game_rom);
    fclose(game_rom);
    printf("Game ROM loaded. (%zu bytes)\n", game_size);
}

void emu_loop(CPU* cpu, PPU* ppu, SDL_Context* context)
{
    int running = 1;
    int frame_count = 0;
    SDL_Event event;
    uint16_t last_pc = REG_PC;
    int stuck_count = 0;
    int instruction_count = 0;
    int waiting_for_lcd = 0;
    
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) 
                running = 0;
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
                handle_input(&event);
        }
        
        int cycles = 0;
        while (cycles < 70224)
        {
            uint16_t pc_before = REG_PC;
            uint16_t sp_before = REG_SP;
            uint8_t opcode = read_byte(REG_PC);
            
            if (dbg.dbg_cpu)
            {
                // Debug first 50 instructions
                if (instruction_count < 50)
                {
                    DBG_PRINT("Inst %3d: PC=%04X SP=%04X opcode=%02X | AF=%04X BC=%04X DE=%04X HL=%04X\n",
                           instruction_count, pc_before, sp_before, opcode, 
                           REG_AF, REG_BC, REG_DE, REG_HL);
                    instruction_count++;
                }
            }
            
            cycles += cpu_step(cpu, ppu);
            
            if (debug)
            {
                // Check whether stuck at 0x009F (boot ROM LCD wait)
                if (dbg.dbg_ppu && pc_before == 0x009F && REG_PC == 0x009F && !waiting_for_lcd) 
                {
                    DBG_PRINT("\n*** CPU stuck at 0x009F - this is the LCD wait loop in boot ROM ***\n");
                    DBG_PRINT("Opcode: 0x%02X at 0x009F\n", opcode);
                    DBG_PRINT("LCDC register (0xFF40): 0x%02X (bit 7 = LCD on/off)\n", memory[0xFF40]);
                    DBG_PRINT("LY register (0xFF44): 0x%02X\n", memory[0xFF44]);
                    DBG_PRINT("This loop waits for LY to reach 144, but LCDC bit 7 is 0 (LCD off)\n");
                    DBG_PRINT("Boot ROM needs to turn on LCD first!\n");
                    waiting_for_lcd = 1;
                }
                
                // Detect if PC went in invalid memory
                if (dbg.dbg_cpu && REG_PC >= 0xFF00 && REG_PC < 0xFF80)
                {
                    DBG_PRINT("\n!!! CPU crashed! PC is in hardware registers: 0x%04X !!!\n", REG_PC);
                    DBG_PRINT("PC before: 0x%04X, SP before: 0x%04X, opcode was: 0x%02X\n", 
                           pc_before, sp_before, opcode);
                    DBG_PRINT("Stack at SP:\n");
                    for (int i = 0; i < 8; i++)
                        DBG_PRINT("  [0x%04X] = 0x%02X\n", sp_before + i, read_byte(sp_before + i));
                    print_cpu_state(cpu);
                    running = 0;
                    break;
                }
            }
        }
        
        if (ppu->frame_ready)
        {
            frame_count++;
            
            // Detect infinite loop
            if (REG_PC == last_pc)
            {
                stuck_count++;
                if (stuck_count > 300)
                {
                    printf("\n!!! CPU appears stuck in infinite loop at PC=0x%04X !!!\n", REG_PC);
                    printf("Opcode at PC: 0x%02X\n", read_byte(REG_PC));
                    printf("Nearby code:\n");
                    for (int i = -4; i <= 4; i++) 
                    {
                        printf("  [0x%04X] = 0x%02X%s\n", 
                               REG_PC + i, read_byte(REG_PC + i),
                               i == 0 ? " <-- PC" : "");
                    }
                    print_cpu_state(cpu);
                    running = 0;
                    break;
                }
            }
            else
            {
                stuck_count = 0;
                last_pc = REG_PC;
            }
            
            if (dbg.dbg_ppu)
            {
                if (frame_count % 60 == 0)
                {
                    DBG_PRINT("Frame %d: PC=0x%04X SP=0x%04X LCDC=0x%02X BGP=0x%02X\n", 
                           frame_count, REG_PC, REG_SP, memory[0xFF40], memory[0xFF47]);
                }
            }
            
            render_frame(context->renderer, context->texture, ppu->framebuffer);
            ppu->frame_ready = 0;
        }
    }
}
