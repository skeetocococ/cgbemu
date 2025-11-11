#include "memory.h"
#include "io/joypad.h"
#include "cpu/cpu.h"
#include "debug/debug.h"

uint16_t current_pc_debug = 0;
uint8_t vram_block = 0;

uint8_t memory[MEM_SIZE];
uint8_t* vram = &memory[VRAM_START]; 
uint8_t* oam = &memory[OAM_START];
size_t vram_size = VRAM_END - VRAM_START;
size_t oam_size = OAM_END - OAM_START;

uint8_t bootstrap_enabled = 0;
static uint8_t boot_rom[256];

DMA dma = { 0, 0, 0 };

void write_byte(uint16_t addr, uint8_t val)
{
    if (dbg.dbg_mem && current_pc_debug >= 0x0090 && current_pc_debug <= 0x00B0) 
    {
        DBG_PRINT("WRITE: PC=%04X writing 0x%02X to 0x%04X\n", current_pc_debug, val, addr);
    }
    if (addr == ADDR_DIV) 
    {
        cpu_timer.div_counter = 0;
        memory[addr] = 0;
        return;
    }
    if (addr == 0xFF46)  // DMA transfer
    {
        memory[addr] = val;
        dma.active = 1;
        dma.src = val << 8;
        dma.index = 0;
        return;
    }
    if (addr == 0xFF40) 
    {
        if (dbg.dbg_mem) 
            DBG_PRINT("LCDC write: 0x%02X -> 0xFF40 (current LY=%02X)\n", val, memory[0xFF44]);
        memory[addr] = val;
        return;
    }
    if (addr == 0xFF02 && val == 0x81)
    {
        char c = memory[0xFF01];
        if (c >= 32 && c <= 126)
            putchar(c);
        else if (c == '\n' || c == '\r') 
            putchar('\n');
        fflush(stdout);
        memory[0xFF02] = 0;
        return;
    }
    if (addr == 0xFF50 && bootstrap_enabled)
    {
        bootstrap_enabled = 0;
        printf("Boot ROM disabled.\n");
        return;
    }
    
    // Block writes to VRAM/OAM
    if (vram_block && addr >= VRAM_START && addr < 0xA000) return;
    if (dma.active && addr >= OAM_START && addr < 0xFEA0) return;
    
    memory[addr] = val;

    if (dbg.dbg_mem && (addr == 0xA000 || addr == 0xA001))
        DBG_PRINT("\n[Test wrote 0x%02X to 0x%04X]\n", val, addr);
}

uint8_t read_byte(uint16_t addr) 
{
    if (bootstrap_enabled && addr < 0x0100)
        return boot_rom[addr];
    
    if (addr == ADDR_DIV) 
        return (cpu_timer.div_counter >> 8) & 0xFF;
    
    if (addr == ADDR_P1)
    {
        uint8_t p1 = memory[ADDR_P1];
        uint8_t result = 0xCF;
        
        if (!(p1 & 0x10))
            result &= (joypad.dpad | 0xF0);
        
        if (!(p1 & 0x20))
            result &= (joypad.buttons | 0xF0);
        
        return result;
    }
    
    // Block reads from VRAM/OAM
    if (dma.active && addr >= 0xFE00 && addr < 0xFEA0) return 0xFF;
    if (vram_block && addr >= 0x8000 && addr < 0xA000) return 0xFF;
    
    return memory[addr];
}

void dma_step() 
{ 
    if (!dma.active) return; 
    oam[dma.index] = memory[dma.src + dma.index]; 
    dma.index++; 
    if (dma.index == 0xA0) dma.active = 0; 
}

void bootstrap(char* rom)
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
