#include "memory.h"
#include "joypad.h"
#include "cpu.h"

uint8_t memory[MEM_SIZE];
uint8_t* vram = &memory[VRAM_START]; 
uint8_t* oam = &memory[OAM_START];
size_t vram_size = VRAM_END - VRAM_START;
size_t oam_size = OAM_END - OAM_START;
static uint8_t bootstrap_enabled = 1;

void write_byte(uint16_t addr, uint8_t val)
{
    if (addr == ADDR_DIV) 
    {
        cpu_timer.cycle_counter = 0;
        memory[addr] = 0;  // DIV is also readable, should be 0 after write
        return;
    }
    if (addr == 0xFF46)  // DMA transfer
    {
        memory[addr] = val;
        uint16_t source = val << 8;
        for (int i = 0; i < 0xA0; i++)
            memory[0xFE00 + i] = memory[source + i];
        // Note: Real hardware takes 160 cycles and blocks memory access
        // For now, instant transfer is fine
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
        return;  // Don't write 0x81 to memory, write 0
    }
    if (addr == 0xFF50 && bootstrap_enabled)
    {
        bootstrap_enabled = 0;
        printf("Boot ROM disabled.\n");
    }
    if (addr == 0xA000 || addr == 0xA001)
        printf("\n[Test wrote 0x%02X to 0x%04X]\n", val, addr);
    memory[addr] = val;
}

uint8_t read_byte(uint16_t addr) 
{
    if (bootstrap_enabled && addr < 0x0100)
        return memory[addr];
    if (addr == ADDR_DIV) 
        return (cpu_timer.cycle_counter >> 8) & 0xFF;
    if (addr == 0xFF44)
        return ((cpu_timer.cycle_counter >> 8) % 154);
    if (addr == ADDR_P1)
    {
        uint8_t p1 = memory[ADDR_P1];
        uint8_t result = 0xCF;  // Bits 6-7 always set, bits 4-5 are select
        // Bit 4: Select D-pad (0 = selected)
        if (!(p1 & 0x10))
            result &= (joypad.dpad | 0xF0);
        // Bit 5: Select buttons (0 = selected)
        if (!(p1 & 0x20))
            result &= (joypad.buttons | 0xF0);
        return result;
    }
    return memory[addr];
}

void bootstrap()
{
    FILE* bootstrap = fopen("bootstrap.gb", "rb");
    if (!bootstrap)
    {
        fprintf(stderr, "Failed to initialize bootstrap ROM.\n");
        exit(EXIT_FAILURE);
    }
    size_t bootstrap_size = fread(memory, 1, 0x0100, bootstrap);
    fclose(bootstrap);
    if (bootstrap_size != 0x0100) 
    {
        fprintf(stderr, "Bootstrap ROM incomplete. (read %zu bytes)\n", bootstrap_size);
        exit(EXIT_FAILURE);
    }
    printf("Bootstrap ROM loaded. (%zu bytes)\n", bootstrap_size);
}

