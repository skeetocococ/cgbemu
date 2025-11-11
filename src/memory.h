#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <string.h>

extern uint8_t bootstrap_enabled;
extern uint16_t current_pc_debug;
#define MEM_SIZE 65536
extern uint8_t memory[MEM_SIZE];
extern uint8_t* vram;
extern uint8_t* oam;

extern int vram_block;

extern size_t vram_size;
extern size_t oam_size;

// Timer / Divider registers
#define ADDR_DIV   0xFF04  // Divider
#define ADDR_TIMA  0xFF05  // Timer counter
#define ADDR_TMA   0xFF06  // Timer modulo
#define ADDR_TAC   0xFF07  // Timer control
// Interrupts
#define ADDR_IF    0xFF0F  // Interrupt flag
#define ADDR_IE    0xFFFF  // Interrupt enable
// Ranges
#define ROM0_START 0x0000
#define ROM0_END   0x3FFF
#define ROMX_START 0x4000
#define ROMX_END   0x7FFF
#define VRAM_START 0x8000
#define TILESET1_PART 0x87FF
#define TILESET1_END 0x8FFF
#define TILESET2_START 0x8800
#define TILESET2_END   0x97FF
#define VRAM_END   0x9FFF
#define EXTRAM_START 0xA000
#define EXTRAM_END   0xBFFF
#define WRAM_START 0xC000
#define WRAM_END   0xDFFF
#define ECHO_START 0xE000
#define ECHO_END   0xFDFF
#define OAM_START  0xFE00
#define OAM_END    0xFE9F
#define IO_START   0xFF00
#define IO_END     0xFF7F
#define HRAM_START 0xFF80
#define HRAM_END   0xFFFE
// Joypad
#define ADDR_P1 0xFF00

typedef struct {
    uint8_t active;
    uint16_t src;
    uint8_t index;
} DMA;

extern DMA dma;

void write_byte(uint16_t addr, uint8_t val);
uint8_t read_byte(uint16_t addr);
void dma_step();
void bootstrap(char* rom);
void init_hardware_reg();

#endif // MEMORY_H
