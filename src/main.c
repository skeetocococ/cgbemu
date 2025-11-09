#include "memory.h"
#include "cpu.h"
#include "opcodes.h"
#include "ppu.h"
#include "joypad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <string.h>

void init_hardware_reg()
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

int main(int argc, char** argv)
{ 
    if (argc < 2)
    {
        printf("Usage: %s <game.gb> [boot.gb]\n", argv[0]);
        return 1;
    }
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    
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
    
    FILE* game_rom = fopen(argv[1], "rb");
    if (!game_rom)
    {
        fprintf(stderr, "Failed to open ROM file: %s\n", argv[1]);
        return 1;
    }
    size_t game_size = fread(memory, 1, MEM_SIZE, game_rom);
    fclose(game_rom);
    printf("Game ROM loaded. (%zu bytes)\n", game_size);
    
    CPU cpu;
    PPU ppu;
    init_opcodes();
    cpu_init(&cpu);
    ppu_init(&ppu);
    
    // Handle boot ROM or skip to 0x0100
    if (argc >= 3)
    {
        bootstrap(argv[2]);
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
        init_hardware_reg();
        cpu.PC = 0x0100;
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
    printf("Starting PC: 0x%04X, SP: 0x%04X\n", cpu.PC, cpu.SP);
    
    int running = 1;
    int frame_count = 0;
    SDL_Event event;
    uint16_t last_pc = cpu.PC;
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
            uint16_t pc_before = cpu.PC;
            uint16_t sp_before = cpu.SP;
            uint8_t opcode = read_byte(cpu.PC);
            
            // Debug first 50 instructions
            if (instruction_count < 50)
            {
                printf("Inst %3d: PC=%04X SP=%04X opcode=%02X | AF=%04X BC=%04X DE=%04X HL=%04X\n",
                       instruction_count, pc_before, sp_before, opcode, 
                       cpu.af.AF, cpu.bc.BC, cpu.de.DE, cpu.hl.HL);
                instruction_count++;
            }
            
            cycles += cpu_step(&cpu, &ppu);
            
            // Check whether stuck at 0x009F (boot ROM LCD wait)
            if (pc_before == 0x009F && cpu.PC == 0x009F && !waiting_for_lcd) {
                printf("\n*** CPU stuck at 0x009F - this is the LCD wait loop in boot ROM ***\n");
                printf("Opcode: 0x%02X at 0x009F\n", opcode);
                printf("LCDC register (0xFF40): 0x%02X (bit 7 = LCD on/off)\n", memory[0xFF40]);
                printf("LY register (0xFF44): 0x%02X\n", memory[0xFF44]);
                printf("This loop waits for LY to reach 144, but LCDC bit 7 is 0 (LCD off)\n");
                printf("Boot ROM needs to turn on LCD first!\n");
                waiting_for_lcd = 1;
            }
            
            // Detect if PC went in invalid memory
            if (cpu.PC >= 0xFF00 && cpu.PC < 0xFF80)
            {
                printf("\n!!! CPU crashed! PC is in hardware registers: 0x%04X !!!\n", cpu.PC);
                printf("PC before: 0x%04X, SP before: 0x%04X, opcode was: 0x%02X\n", 
                       pc_before, sp_before, opcode);
                printf("Stack at SP:\n");
                for (int i = 0; i < 8; i++)
                    printf("  [0x%04X] = 0x%02X\n", sp_before + i, read_byte(sp_before + i));
                print_cpu_state(&cpu);
                running = 0;
                break;
            }
        }
        
        if (ppu.frame_ready)
        {
            frame_count++;
            
            // Detect infinite loop
            if (cpu.PC == last_pc)
            {
                stuck_count++;
                if (stuck_count > 300)
                {
                    printf("\n!!! CPU appears stuck in infinite loop at PC=0x%04X !!!\n", cpu.PC);
                    printf("Opcode at PC: 0x%02X\n", read_byte(cpu.PC));
                    printf("Nearby code:\n");
                    for (int i = -4; i <= 4; i++) 
                    {
                        printf("  [0x%04X] = 0x%02X%s\n", 
                               cpu.PC + i, read_byte(cpu.PC + i),
                               i == 0 ? " <-- PC" : "");
                    }
                    print_cpu_state(&cpu);
                    running = 0;
                    break;
                }
            }
            else
            {
                stuck_count = 0;
                last_pc = cpu.PC;
            }
            
            if (frame_count % 60 == 0)
            {
                printf("Frame %d: PC=0x%04X SP=0x%04X LCDC=0x%02X BGP=0x%02X\n", 
                       frame_count, cpu.PC, cpu.SP, memory[0xFF40], memory[0xFF47]);
            }
            
            render_frame(renderer, texture, ppu.framebuffer);
            ppu.frame_ready = 0;
        }
    }
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
