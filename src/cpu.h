#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h"
#include "ppu.h"

// Flags
#define FLAG_Z 0b10000000
#define FLAG_N 0b01000000
#define FLAG_H 0b00100000
#define FLAG_C 0b00010000

// CPU register unions
typedef union {
    struct { uint8_t F, A; };
    uint16_t AF;
} RegAF;

typedef union {
    struct { uint8_t C, B; };
    uint16_t BC;
} RegBC;

typedef union {
    struct { uint8_t E, D; };
    uint16_t DE;
} RegDE;

typedef union {
    struct { uint8_t L, H; };
    uint16_t HL;
} RegHL;

// Registers
#define REG_A cpu->af.A
#define REG_B cpu->bc.B
#define REG_C cpu->bc.C
#define REG_D cpu->de.D
#define REG_E cpu->de.E
#define REG_F cpu->af.F
#define REG_H cpu->hl.H
#define REG_L cpu->hl.L
#define REG_AF cpu->af.AF
#define REG_BC cpu->bc.BC
#define REG_DE cpu->de.DE
#define REG_HL cpu->hl.HL
#define REG_SP cpu->SP
#define REG_PC cpu->PC

// CPU structure
typedef struct {
    RegAF af;
    RegBC bc;
    RegDE de;
    RegHL hl;
    uint16_t SP;
    uint16_t PC;
    uint8_t halted, stopped, halt_bug;
    uint8_t pending_disable_interrupts, pending_enable_interrupts;
    uint8_t IME;
} CPU;

typedef struct {
    uint16_t div_counter;
    uint16_t tima_counter;
    uint32_t cycle_counter;
    uint8_t overflow;
    uint8_t delay;
} Timer;

extern Timer cpu_timer;
typedef enum { NZ, Z, NC, C } Condition;
typedef enum { VBLANK_INT, STAT_INT, TIMER_INT, SERIAL_INT, JOYPAD_INT } Interrupt;

void print_cpu_state(CPU* cpu);
uint16_t cpu_step(CPU* cpu, PPU* ppu);
static inline void request_interrupt(Interrupt interrupt) { memory[ADDR_IF] |= (1 << interrupt); }

#endif // CPU_H
