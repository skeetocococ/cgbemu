#include "cpu.h"
#include "ppu.h"
#include "instructions/instructions.h"
#include "opcodes.h"
#include "memory.h"
#include <stdio.h>

Timer cpu_timer;

void print_cpu_state(CPU* cpu)
{
    printf("AF=%04X BC=%04X DE=%04X HL=%04X SP=%04X PC=%04X IME=%d \n",
           REG_AF, REG_BC, REG_DE, REG_HL, REG_SP, REG_PC, cpu->IME);
}

static void handle_interrupt(CPU* cpu)
{
    uint8_t enabled = memory[ADDR_IF] & memory[ADDR_IE];
    if (!enabled) return;
    cpu->IME = 0;
    push_nn(cpu, REG_PC);
    if (enabled & (1 << 0)) 
    {
        memory[ADDR_IF] &= ~(1 << 0);
        REG_PC = 0x40;
    } else if (enabled & (1 << 1)) 
    {
        memory[ADDR_IF] &= ~(1 << 1);
        REG_PC = 0x48;
    } else if (enabled & (1 << 2)) 
    {
        memory[ADDR_IF] &= ~(1 << 2);
        REG_PC = 0x50;
    } else if (enabled & (1 << 3))
    {
        memory[ADDR_IF] &= ~(1 << 3);
        REG_PC = 0x58;
    } else if (enabled & (1 << 4)) 
    {
        memory[ADDR_IF] &= ~(1 << 4);
        REG_PC = 0x60;
    }
}

static void timer_tick(CPU* cpu, int cycles)
{
    cpu_timer.div_counter += cycles;
    while (cpu_timer.div_counter >= 256) 
    {
        cpu_timer.div_counter -= 256;
        memory[ADDR_DIV]++;
    }

    if (cpu_timer.overflow) 
    {
        cpu_timer.delay -= cycles;
        if (cpu_timer.delay <= 0) 
        {
            memory[ADDR_TIMA] = memory[ADDR_TMA];
            cpu_timer.overflow = 0;
        }
    }

    if (!(memory[ADDR_TAC] & 0x04)) return;

    static const int tac_cycles[4] = {1024, 16, 64, 256};
    uint8_t freq = memory[ADDR_TAC] & 0x03;
    int step = tac_cycles[freq];
    cpu_timer.tima_counter += cycles;

    while (cpu_timer.tima_counter >= step) 
    {
        cpu_timer.tima_counter -= step;
        if (memory[ADDR_TIMA] == 0xFF) 
        {
            memory[ADDR_TIMA] = 0x00;            
            request_interrupt(TIMER_INT);
            cpu_timer.overflow = 1;       
            cpu_timer.delay = 4;        
        } else 
        {
            memory[ADDR_TIMA]++;
        }
    }
}

uint16_t cpu_step(CPU* cpu, PPU* ppu)
{
    if (cpu->halted)
    {
        timer_tick(cpu, 1);
        if (memory[ADDR_IF] & memory[ADDR_IE])
            cpu->halted = 0;
        return 1;
    }

    if (cpu->stopped) return 1;

    uint8_t exec_twice = cpu->halt_bug;
    if (cpu->halt_bug) cpu->halt_bug = 0;
    uint8_t opcode = memory[REG_PC++];
    uint16_t cycles;

    if (opcode == 0xCB)
    {
        uint8_t cb_opcode = memory[REG_PC++];
        cb_opcodes[cb_opcode](cpu);
        cycles = cb_opcode_cycles[cb_opcode];
    }
    else 
    {
        if (opcode == 0xC2 || opcode == 0xCA || opcode == 0xD2 || opcode == 0xDA)
        {
            Condition cond = (opcode == 0xC2) ? NZ : (opcode == 0xCA) ? Z : 
                            (opcode == 0xD2) ? NC : C;
            cycles = jp_cc_nn(cpu, cond);
        }
        else if (opcode == 0x20 || opcode == 0x28 || opcode == 0x30 || opcode == 0x38)
        {
            Condition cond = (opcode == 0x20) ? NZ : (opcode == 0x28) ? Z : 
                            (opcode == 0x30) ? NC : C;
            cycles = jr_cc_n(cpu, cond);
        }
        else if (opcode == 0xC4 || opcode == 0xCC || opcode == 0xD4 || opcode == 0xDC)
        {
            Condition cond = (opcode == 0xC4) ? NZ : (opcode == 0xCC) ? Z : 
                            (opcode == 0xD4) ? NC : C;
            cycles = call_cc_nn(cpu, cond);
        }
        else if (opcode == 0xC0 || opcode == 0xC8 || opcode == 0xD0 || opcode == 0xD8)
        {
            Condition cond = (opcode == 0xC0) ? NZ : (opcode == 0xC8) ? Z : 
                            (opcode == 0xD0) ? NC : C;
            cycles = ret_cc(cpu, cond);
        }
        else
        {
            opcodes[opcode](cpu);
            cycles = opcode_cycles[opcode];
        }
    }

    if (cpu->pending_enable_interrupts) 
    {
        cpu->IME = 1;
        cpu->pending_enable_interrupts = 0;
    }
    if (cpu->pending_disable_interrupts) 
    {
        cpu->IME = 0;
        cpu->pending_disable_interrupts = 0;
    }

    if (exec_twice)
    {
        REG_PC--;
        return cpu_step(cpu, ppu);
    }

    timer_tick(cpu, cycles);
    ppu_step(ppu, cycles);
    if (cpu->IME && (memory[ADDR_IF] & memory[ADDR_IE]))
        handle_interrupt(cpu);
    return cycles;
}

