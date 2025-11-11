#include "instructions.h"
#include "../memory.h"
#include "cpu.h"
#include <stdint.h>

void ld_rx_ry(CPU* cpu, uint8_t* dest, uint8_t* src) { *dest = *src; } // ry into rx
void ld_rx_hl(CPU* cpu, uint8_t* dest) { *dest = read_byte(REG_HL); }
void ld_hl_ry(CPU* cpu, uint8_t* src) { write_byte(REG_HL, *src); }
void ld_r_n(CPU* cpu, uint8_t* dest) { *dest = read_byte(REG_PC++); } // next byte into register

void ld_r_nn(CPU* cpu, uint8_t* dest) 
{
    uint16_t addr = read_byte(REG_PC++) | (read_byte(REG_PC++) << 8);
    *dest = read_byte(addr);
}

void ld_nn_r(CPU* cpu, uint8_t* src)
{
    uint16_t addr = read_byte(REG_PC++) | (read_byte(REG_PC++) << 8);
    write_byte(addr, *src); // register to 16 bit intermediate mem location   
}

void ld_r_rr(CPU* cpu, uint8_t* dest, uint16_t addr) { *dest = read_byte(addr); } // address in register pair into dest reg
void ld_rr_r(CPU* cpu, uint8_t* src, uint16_t addr){ write_byte(addr, *src); } // register into mem location

void ld_a_c(CPU* cpu)
{
    uint16_t addr = 0xFF00 + REG_C;
    REG_A = read_byte(addr); // Value at address 0xFF00 + reg C into reg A
}

void ld_c_a(CPU* cpu)
{
    uint16_t addr = 0xFF00 + REG_C;
    write_byte(addr, REG_A); // Value in reg A to address 0xFF00 + reg C
}

void ldd_a_hl(CPU* cpu)
{
    REG_A = read_byte(REG_HL);
    REG_HL--;
}

void ldd_hl_a(CPU* cpu)
{
    write_byte(REG_HL, REG_A);
    REG_HL--;
}

void ldi_a_hl(CPU* cpu)
{
    REG_A = read_byte(REG_HL);
    REG_HL++;
}

void ldi_hl_a(CPU* cpu)
{
    write_byte(REG_HL, REG_A);
    REG_HL++;
}

void ldh_n_a(CPU* cpu)
{
    uint16_t addr = 0xFF00 + read_byte(REG_PC++);
    write_byte(addr, REG_A);
}

void ldh_a_n(CPU* cpu)
{
    uint16_t addr = 0xFF00 + read_byte(REG_PC++);
    REG_A = read_byte(addr);
}

void ld_n_nn(CPU* cpu, uint16_t* dest) 
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    uint16_t addr = (high << 8) | low;
    *dest = read_byte(addr) | (read_byte(addr + 1) << 8); // address of next two bytes into register
}

void ld_sp_hl(CPU* cpu) { REG_SP = REG_HL; }

void ldhl_sp_n(CPU* cpu)
{
    int8_t n = (int8_t)read_byte(REG_PC++);
    uint16_t result = REG_SP + n;

    REG_F &= ~(FLAG_Z | FLAG_N | FLAG_H | FLAG_C);

    if (((REG_SP & 0xF) + (n & 0xF)) > 0xF) REG_F |= FLAG_H;
    if (((REG_SP & 0xFF) + (n & 0xFF)) > 0xFF) REG_F |= FLAG_C;

    REG_HL = result;
}

void ld_nn_sp(CPU* cpu)
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    uint16_t addr = (high << 8) | low;
    write_byte(addr, REG_SP & 0xFF);
    write_byte(addr + 1, REG_SP >> 8);
}

void push_nn(CPU* cpu, uint16_t value) 
{
    write_byte(--REG_SP, (value >> 8));
    write_byte(--REG_SP, (value & 0xFF));
}

uint16_t pop_nn(CPU* cpu) 
{
    uint16_t low  = read_byte(REG_SP++);
    uint16_t high = read_byte(REG_SP++);
    return (high << 8) | low;
}

void add_a_n(CPU* cpu, uint8_t value)
{
    uint8_t a = REG_A;
    uint16_t result = a + value;
    REG_F = 0; 
    if ((result & 0xFF) == 0) REG_F |= FLAG_Z; // Z
    // N flag (Subtract) -> always 0 for ADD, skip
    if (((a & 0xF) + (value & 0xF)) > 0xF) REG_F |= FLAG_H; // H (Half-Carry) from bit 3-4
    if (result > 0xFF) REG_F |= FLAG_C; // C flag carry out of 7
    REG_A = result & 0xFF;
}

void adc_a_n(CPU* cpu, uint8_t value)
{
    uint8_t a = REG_A;
    uint8_t carry = (REG_F & FLAG_C) ? 1 : 0;
    uint16_t result = a + value + carry;
    REG_F = 0;
    if ((result & 0xFF) == 0) REG_F |= FLAG_Z; // Z
    // N flag (Subtract) -> always 0 for ADD, skip
    if (((a & 0xF) + (value & 0xF) + carry) > 0xF) REG_F |= FLAG_H; // H (Half-Carry) from bit 3-4
    if (result > 0xFF) REG_F |= FLAG_C; // C flag carry out of 7
    REG_A = result & 0xFF;  
}

void sub_a_n(CPU* cpu, uint8_t value)
{
    uint8_t a = REG_A;
    uint16_t result = a - value;
    REG_F = 0;
    REG_F |= FLAG_N; // N flag — Always set for subtraction
    if ((result & 0xFF) == 0) REG_F |= FLAG_Z;
    if ((a & 0xF) < (value & 0xF)) REG_F |= FLAG_H;  // H flag — Set if borrow from bit 4 (check lower nibble)
    if (a < value) REG_F |= FLAG_C; // C flag — Set if full borrow (underflow)
    REG_A = result & 0xFF;
}

void sbc_a_n(CPU* cpu, uint8_t value)
{
    uint8_t a = REG_A;
    uint8_t carry = (REG_F & FLAG_C) ? 1 : 0;
    uint16_t result = a - value - carry;
    REG_F = 0;
    REG_F |= FLAG_N; // N flag — Always set for subtraction
    if ((result & 0xFF) == 0) REG_F |= FLAG_Z;
    if ((a & 0xF) < ((value & 0xF) + carry)) REG_F |= FLAG_H;  // H flag if borrow from bit 4 (check lower nibble)
    if (a < value + carry) REG_F |= FLAG_C; // C flag if full borrow (underflow)
    REG_A = result & 0xFF;
}

void and_a_n(CPU* cpu, uint8_t value)
{
    REG_A &= value;
    REG_F = 0;
    REG_F |= FLAG_H; // H flag set
    if (REG_A == 0) REG_F |= FLAG_Z;
}

void or_a_n(CPU* cpu, uint8_t value)
{
    REG_A |= value;
    REG_F = 0;
    if (REG_A == 0) REG_F |= FLAG_Z;
}

void xor_a_n(CPU* cpu, uint8_t value)
{
    REG_A ^= value;
    REG_F = 0;
    if (REG_A == 0) REG_F |= FLAG_Z;
}

void cp_a_n(CPU* cpu, uint8_t value)
{
    uint8_t result = REG_A - value;
    REG_F = 0;
    REG_F |= FLAG_N;
    if (result == 0) REG_F |= FLAG_Z;
    if ((REG_A & 0xF) < (value & 0xF)) REG_F |= FLAG_H;  // H flag if borrow from bit 4 (check lower nibble)
    if (REG_A < value) REG_F |= FLAG_C; // No borrow
}

void inc_n(CPU* cpu, uint8_t* reg) 
{ 
    uint8_t value = *reg;
    uint8_t result = value + 1;
    // Preserve carry flag
    uint8_t carry = REG_F & FLAG_C;
    // Reset N and H, will recompute H if needed
    REG_F = carry;  
    REG_F &= ~FLAG_N;  // N is reset for INC
    if ((value & 0xF) + 1 > 0xF) REG_F |= FLAG_H; // half-carry
    if (result == 0) REG_F |= FLAG_Z;            // zero flag
    *reg = result;
}

void dec_n(CPU* cpu, uint8_t* reg) 
{ 
    uint8_t value = *reg;
    uint8_t result = value - 1;
    // Preserve carry flag
    uint8_t flags = (REG_F & FLAG_C) | FLAG_N;
    // Set N, compute H for half-borrow
    //REG_F = carry | FLAG_N;  
    if ((value & 0x0F) == 0x00) flags |= FLAG_H; // half-borrow
    if (result == 0) flags |= FLAG_Z;        // zero flag
    REG_F = flags;
    *reg = result;
}

void add_hl_n(CPU* cpu, uint16_t* reg)
{
    uint32_t result = REG_HL + *reg;      

    REG_F &= FLAG_Z;
    REG_F &= ~FLAG_N;
    // H: carry from bit 11
    if (((REG_HL & 0x0FFF) + (*reg & 0x0FFF)) > 0x0FFF) REG_F |= FLAG_H;
    // C: carry from bit 15
    if (result > 0xFFFF) REG_F |= FLAG_C;
    REG_HL = (uint16_t)result;
}

void add_sp_n(CPU* cpu)
{
    int8_t n = (int8_t)read_byte(REG_PC++);
    uint16_t sp = REG_SP;
    REG_F = 0;
    // Flags calculated on lower byte only
    if (((sp & 0xF) + (n & 0xF)) > 0xF) REG_F |= FLAG_H;
    if (((sp & 0xFF) + (n & 0xFF)) > 0xFF) REG_F |= FLAG_C;
    REG_SP = sp + n;  // Signed addition
}

void inc_nn(CPU* cpu, uint16_t* reg) { ++(*reg); }
void dec_nn(CPU* cpu, uint16_t* reg) { --(*reg); }

void swap_n(CPU* cpu, uint8_t* reg)
{
    uint8_t value = *reg;
    *reg = (value << 4) | (value >> 4);
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
}

void swap_hl(CPU* cpu)
{
    uint8_t value = read_byte(REG_HL);
    value = (value << 4) | (value >> 4);
    write_byte(REG_HL, value);
    REG_F = (value == 0) ? FLAG_Z : 0;
}

void daa_a(CPU* cpu)
{
    uint8_t a = REG_A;
    uint8_t adjust = 0;
    uint8_t carry = 0;
    if (!(REG_F & FLAG_N))  // after addition
    {
        if ((REG_F & FLAG_H) || (a & 0x0F) > 9)
            adjust |= 0x06;
        if ((REG_F & FLAG_C) || a > 0x99)
        {
            adjust |= 0x60;
            carry = 1;
        }
        a += adjust;
    }
    else  // after subtraction
    {
        if (REG_F & FLAG_H)
            adjust |= 0x06;
        if (REG_F & FLAG_C)
            adjust |= 0x60;
        a -= adjust;
    }
    REG_A = a;
    REG_F &= ~(FLAG_Z | FLAG_H); // H always cleared, Z recalculated
    if (a == 0) REG_F |= FLAG_Z;
    if (carry) REG_F |= FLAG_C;
}

void cpl_a(CPU* cpu)
{
    REG_A ^= 0xFF;
    REG_F |= FLAG_N | FLAG_H;
}

void ccf(CPU* cpu)
{
    REG_F = (REG_F & (FLAG_Z)) | ((REG_F & FLAG_C) ? 0 : FLAG_C);
    REG_F = (REG_F & FLAG_Z) ^ FLAG_C;  // Toggle carry, preserve Z
}

void scf(CPU* cpu) { REG_F = (REG_F & FLAG_Z) | FLAG_C; }

void rrc(CPU* cpu, uint8_t* reg)
{
    uint8_t bit0 = *reg & 0x01;
    *reg = (*reg >> 1) | (bit0 << 7);
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (bit0) REG_F |= FLAG_C;
}

void rrn(CPU* cpu, uint8_t* reg)
{
    uint8_t oldCarry = (REG_F & FLAG_C) ? 1 : 0;
    uint8_t bit0 = *reg & 0x01;

    *reg = (*reg >> 1) | (oldCarry << 7);

    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (bit0) REG_F |= FLAG_C;
}

void rlc(CPU* cpu, uint8_t* reg)
{
    uint8_t bit7 = (*reg & 0x80) >> 7;
    *reg = (*reg << 1) | bit7;
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (bit7) REG_F |= FLAG_C;
}

void rl(CPU* cpu, uint8_t* reg)
{
    uint8_t oldCarry = (REG_F & FLAG_C) ? 1 : 0;
    uint8_t bit7 = (*reg & 0x80) >> 7;
    *reg = (*reg << 1) | oldCarry;
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (bit7) REG_F |= FLAG_C;
}

void sla(CPU* cpu, uint8_t* reg)
{
    uint8_t old = *reg;
    *reg <<= 1;
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (old & 0x80) REG_F |= FLAG_C;
}

void sra(CPU* cpu, uint8_t* reg)
{
    uint8_t old = *reg;
    uint8_t msb = old & 0x80;
    *reg = (old >> 1) | msb;
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (old & 0x01) REG_F |= FLAG_C;
}

void srl(CPU* cpu, uint8_t* reg)
{
    uint8_t old = *reg;
    *reg >>= 1;
    REG_F = 0;
    if (*reg == 0) REG_F |= FLAG_Z;
    if (old & 0x01) REG_F |= FLAG_C;
}

void bit(uint8_t bit, uint8_t* reg, CPU* cpu)
{
    uint8_t carry = REG_F & FLAG_C;  // Preserve carry
    REG_F = FLAG_H | carry;          // H set, N cleared
    if (!(*reg & (1 << bit))) REG_F |= FLAG_Z;
}

void res(uint8_t bit, uint8_t* reg) { *reg &= ~(1 << bit); }
void set(uint8_t bit, uint8_t* reg) { *reg |= (1 << bit); }

void jp_nn(CPU* cpu)
{
    uint8_t low  = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    REG_PC = (high << 8) | low;
}

uint8_t jp_cc_nn(CPU* cpu, Condition condition)
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    uint16_t addr = (high << 8) | low;

    uint8_t taken = 0;
    switch (condition)
    {
        case NZ:
            if (!(REG_F & FLAG_Z)) 
            {
                REG_PC = addr;
                taken = 1;
            }
            break;
        case Z:
            if (REG_F & FLAG_Z) 
            {
                REG_PC = addr;
                taken = 1;
            }
            break;
        case NC:
            if (!(REG_F & FLAG_C)) 
            {
                REG_PC = addr;
                taken = 1;
            }
            break;
        case C:
            if (REG_F & FLAG_C) 
            {
                REG_PC = addr;
                taken = 1;
            }
            break;
    }
    return taken ? 16 : 12;
}

void jp_hl(CPU* cpu) { REG_PC = REG_HL; }

void jr_n(CPU* cpu)
{
    int8_t offset = read_byte(REG_PC++);
    REG_PC += offset;
}

uint8_t jr_cc_n(CPU* cpu, Condition condition)
{
    int8_t offset = read_byte(REG_PC++);
    uint8_t taken = 0;
    switch (condition)
    {
        case NZ:
            if (!(REG_F & FLAG_Z)) 
            {
                REG_PC += offset;
                taken = 1;
            }
            break;
        case Z:
            if (REG_F & FLAG_Z) 
            {
                REG_PC += offset;
                taken = 1;
            }
            break;
        case NC:
            if (!(REG_F & FLAG_C)) 
            {
                REG_PC += offset;
                taken = 1;
            }
            break;
        case C:
            if (REG_F & FLAG_C) 
            {
                REG_PC += offset;
                taken = 1;
            }
            break;
    }
    return taken ? 12 : 8;
}

void call_nn(CPU* cpu)
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    uint16_t addr = (high << 8) | low;
    push_nn(cpu, REG_PC);
    REG_PC = addr;
}

uint8_t call_cc_nn(CPU* cpu, Condition condition)
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    uint16_t addr = (high << 8) | low;

    switch (condition)
    {
        case NZ:
            if (!(REG_F & FLAG_Z)) goto call;
            break;
        case Z:
            if (REG_F & FLAG_Z) goto call;
            break;
        case NC:
            if (!(REG_F & FLAG_C)) goto call;
            break;
        case C:
            if (REG_F & FLAG_C) goto call;
            break;
    }
    return 12;
call:
    push_nn(cpu, REG_PC);
    REG_PC = addr;
    return 24;
}

void rst_n(CPU* cpu, uint8_t value)
{
    push_nn(cpu, REG_PC);
    REG_PC = value;
}

void ret(CPU* cpu) { REG_PC = pop_nn(cpu); }

uint8_t ret_cc(CPU* cpu, Condition condition)
{
    uint8_t taken = 0;
    switch (condition)
    {
        case NZ:
            if (!(REG_F & FLAG_Z)) 
            {
                REG_PC = pop_nn(cpu);
                taken = 1;
            }
            break;
        case Z:
            if (REG_F & FLAG_Z) 
            {
                REG_PC = pop_nn(cpu);
                taken = 1;
            }
            break;
        case NC:
            if (!(REG_F & FLAG_C)) 
            {
                REG_PC = pop_nn(cpu);
                taken = 1;
            }
            break;
        case C:
            if (REG_F & FLAG_C) 
            {
                REG_PC = pop_nn(cpu);
                taken = 1;
            }
            break;
    }
    return taken ? 20 : 8;
}

void reti(CPU* cpu)
{
    REG_PC = pop_nn(cpu);
    cpu->IME = 1;
}

