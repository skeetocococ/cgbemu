#include "memory.h"
#include "../io/joypad.h"
#include "../cpu/cpu.h"
#include "../debug/debug.h"

uint8_t memory[MEM_SIZE];
uint8_t boot_rom[256];
uint8_t* vram = &memory[VRAM_START]; 
uint8_t* oam = &memory[OAM_START];

uint8_t bootstrap_enabled = 0;
uint8_t vram_block = 0;
uint16_t current_pc_debug = 0;

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
    
    // Block writes to VRAM/OAM during DMA (optional - not critical)
    if  (vram_block && addr >= VRAM_START && addr < 0xA000) return;
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
    
    // Block reads from VRAM/OAM during DMA
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

