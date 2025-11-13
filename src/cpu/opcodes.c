#include "opcodes.h"
#include "cpu.h"
#include "instructions.h"
#include "../memory/memory.h"

// MISC
static void nop(CPU* cpu) {} // Do nothing
static void halt(CPU* cpu) 
{ 
    if (!cpu->IME && (memory[ADDR_IF] & memory[ADDR_IE]))
        cpu->halt_bug = 1; 
    else
        cpu->halted = 1;
}
static void stop(CPU* cpu) 
{ 
    cpu->stopped = 1; 
    cpu_timer.div_counter = 0;
    cpu_timer.tima_counter = 0;
}
static void ei(CPU* cpu) { cpu->pending_enable_interrupts = 1; }
static void di(CPU* cpu) { cpu->pending_disable_interrupts = 0; }

// LD
static void ld_bc_u16(CPU* cpu)
{
    uint8_t low  = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    REG_BC = (high << 8) | low; // next two bytes into BC reg pair
}
void ld_de_u16(CPU* cpu) 
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    REG_DE = (high << 8) | low;
}

void ld_hl_u16(CPU* cpu) 
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    REG_HL = (high << 8) | low;
}

void ld_sp_u16(CPU* cpu) 
{
    uint8_t low = read_byte(REG_PC++);
    uint8_t high = read_byte(REG_PC++);
    REG_SP = (high << 8) | low;
}
static void ld_a_b(CPU* cpu) { ld_rx_ry(cpu, &REG_A, &REG_B); }
static void load_a_c(CPU* cpu) { ld_a_c(cpu); }
static void ld_b_c(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_C); }
static void ld_hl_a(CPU* cpu) { ld_hl_ry(cpu, &REG_A); }
static void ld_a_d(CPU* cpu) { ld_rx_ry(cpu, &REG_A, &REG_D); }
static void ld_a_e(CPU* cpu) { ld_rx_ry(cpu, &REG_A, &REG_E); }
static void ld_a_h(CPU* cpu) { ld_rx_ry(cpu, &REG_A, &REG_H); }
static void ld_a_l(CPU* cpu) { ld_rx_ry(cpu, &REG_A, &REG_L); }
static void ld_a_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_A); }
static void ld_b_a(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_A); }
static void ld_b_n(CPU* cpu) { ld_r_n(cpu, &REG_B); }
static void ld_c_n(CPU* cpu) { ld_r_n(cpu, &REG_C); }
static void ld_d_n(CPU* cpu) { ld_r_n(cpu, &REG_D); }
static void ld_e_n(CPU* cpu) { ld_r_n(cpu, &REG_E); }
static void ld_h_n(CPU* cpu) { ld_r_n(cpu, &REG_H); }
static void ld_l_n(CPU* cpu) { ld_r_n(cpu, &REG_L); }
static void ld_a_n(CPU* cpu) { ld_r_n(cpu, &REG_A ); }
static void ld_a_bc(CPU* cpu) { REG_A = read_byte(REG_BC); }
static void ld_bc_a(CPU* cpu) { write_byte(REG_BC, REG_A); }
static void ld_a_de(CPU* cpu) { REG_A = read_byte(REG_DE); }
static void ld_de_a(CPU* cpu) { write_byte(REG_DE, REG_A); }
static void ld_a_hli(CPU* cpu) { ldi_a_hl(cpu); }
static void ld_hli_a(CPU* cpu) { ldi_hl_a(cpu); }
static void ld_a_hld(CPU* cpu) { ldd_a_hl(cpu); }
static void ld_hld_a(CPU* cpu) { ldd_hl_a(cpu); }
static void ldh_n_a_op(CPU* cpu) { ldh_n_a(cpu); }
static void ldh_a_n_op(CPU* cpu) { ldh_a_n(cpu); }
static void ld_sp_hl_op(CPU* cpu) { ld_sp_hl(cpu); }
static void ldhl_sp_n_op(CPU* cpu) { ldhl_sp_n(cpu); }
static void ld_nn_sp_op(CPU* cpu) { ld_nn_sp(cpu); }
static void ld_a_a(CPU* cpu) { ld_rx_ry(cpu, &REG_A, &REG_A); }
static void ld_b_b(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_B); }
static void ld_b_d(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_D); }
static void ld_b_e(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_E); }
static void ld_b_h(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_H); }
static void ld_b_l(CPU* cpu) { ld_rx_ry(cpu, &REG_B, &REG_L); }
static void ld_c_b(CPU* cpu) { ld_rx_ry(cpu, &REG_C, &REG_B); }
static void ld_c_c(CPU* cpu) { ld_rx_ry(cpu, &REG_C, &REG_C); }
static void ld_c_d(CPU* cpu) { ld_rx_ry(cpu, &REG_C, &REG_D); }
static void ld_c_e(CPU* cpu) { ld_rx_ry(cpu, &REG_C, &REG_E); }
static void ld_c_h(CPU* cpu) { ld_rx_ry(cpu, &REG_C, &REG_H); }
static void ld_c_l(CPU* cpu) { ld_rx_ry(cpu, &REG_C, &REG_L); }
static void ld_d_b(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_B); }
static void ld_d_c(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_C); }
static void ld_d_d(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_D); }
static void ld_d_e(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_E); }
static void ld_d_h(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_H); }
static void ld_d_l(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_L); }
static void ld_d_a(CPU* cpu) { ld_rx_ry(cpu, &REG_D, &REG_A); }
static void ld_e_b(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_B); }
static void ld_e_c(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_C); }
static void ld_e_d(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_D); }
static void ld_e_e(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_E); }
static void ld_e_h(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_H); }
static void ld_e_l(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_L); }
static void ld_e_a(CPU* cpu) { ld_rx_ry(cpu, &REG_E, &REG_A); }
static void ld_h_b(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_B); }
static void ld_h_c(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_C); }
static void ld_h_d(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_D); }
static void ld_h_e(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_E); }
static void ld_h_h(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_H); }
static void ld_h_l(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_L); }
static void ld_h_a(CPU* cpu) { ld_rx_ry(cpu, &REG_H, &REG_A); }
static void ld_l_b(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_B); }
static void ld_l_c(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_C); }
static void ld_l_d(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_D); }
static void ld_l_e(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_E); }
static void ld_l_h(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_H); }
static void ld_l_l(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_L); }
static void ld_l_a(CPU* cpu) { ld_rx_ry(cpu, &REG_L, &REG_A); }
static void ld_hl_b(CPU* cpu) { ld_hl_ry(cpu, &REG_B); }
static void ld_hl_c(CPU* cpu) { ld_hl_ry(cpu, &REG_C); }
static void ld_hl_d(CPU* cpu) { ld_hl_ry(cpu, &REG_D); }
static void ld_hl_e(CPU* cpu) { ld_hl_ry(cpu, &REG_E); }
static void ld_hl_h(CPU* cpu) { ld_hl_ry(cpu, &REG_H); }
static void ld_hl_l(CPU* cpu) { ld_hl_ry(cpu, &REG_L); }
static void ld_b_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_B); }
static void ld_c_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_C); }
static void ld_d_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_D); }
static void ld_e_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_E); }
static void ld_h_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_H); }
static void ld_l_hl(CPU* cpu) { ld_rx_hl(cpu, &REG_L); }
static void ld_nn_a_op(CPU* cpu) { ld_nn_r(cpu, &REG_A); }
static void ld_a_nn(CPU* cpu) { ld_r_nn(cpu, &REG_A); }
static void ld_hl_n(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    write_byte(REG_HL, value);
}
// ADD
static void add_a_b(CPU* cpu) { add_a_n(cpu, REG_B); }
static void add_a_c(CPU* cpu) { add_a_n(cpu, REG_C); }
static void add_a_d(CPU* cpu) { add_a_n(cpu, REG_D); }
static void add_a_e(CPU* cpu) { add_a_n(cpu, REG_E); }
static void add_a_h(CPU* cpu) { add_a_n(cpu, REG_H); }
static void add_a_l(CPU* cpu) { add_a_n(cpu, REG_L); }
static void add_a_hl(CPU* cpu) { add_a_n(cpu, read_byte(REG_HL)); }
static void add_a_a(CPU* cpu) { add_a_n(cpu, REG_A); }
static void add_hl_bc(CPU* cpu) { add_hl_n(cpu, &REG_BC); }
static void add_hl_de(CPU* cpu) { add_hl_n(cpu, &REG_DE); }
static void add_hl_hl(CPU* cpu) { add_hl_n(cpu, &REG_HL); }
static void add_hl_sp(CPU* cpu) { add_hl_n(cpu, &REG_SP); }
static void adc_a_b(CPU* cpu) { adc_a_n(cpu, REG_B); }
static void adc_a_c(CPU* cpu) { adc_a_n(cpu, REG_C); }
static void adc_a_d(CPU* cpu) { adc_a_n(cpu, REG_D); }
static void adc_a_e(CPU* cpu) { adc_a_n(cpu, REG_E); }
static void adc_a_h(CPU* cpu) { adc_a_n(cpu, REG_H); }
static void adc_a_l(CPU* cpu) { adc_a_n(cpu, REG_L); }
static void adc_a_hl(CPU* cpu) { adc_a_n(cpu, read_byte(REG_HL)); }
static void adc_a_a(CPU* cpu) { adc_a_n(cpu, REG_A); }
// SUB
static void sub_a_b(CPU* cpu) { sub_a_n(cpu, REG_B); }
static void sub_a_c(CPU* cpu) { sub_a_n(cpu, REG_C); }
static void sub_a_d(CPU* cpu) { sub_a_n(cpu, REG_D); }
static void sub_a_e(CPU* cpu) { sub_a_n(cpu, REG_E); }
static void sub_a_h(CPU* cpu) { sub_a_n(cpu, REG_H); }
static void sub_a_l(CPU* cpu) { sub_a_n(cpu, REG_L); }
static void sub_a_hl(CPU* cpu) { sub_a_n(cpu, read_byte(REG_HL)); }
static void sub_a_a(CPU* cpu) { sub_a_n(cpu, REG_A); }
static void sbc_a_b(CPU* cpu) { sbc_a_n(cpu, REG_B); }
static void sbc_a_c(CPU* cpu) { sbc_a_n(cpu, REG_C); }
static void sbc_a_d(CPU* cpu) { sbc_a_n(cpu, REG_D); }
static void sbc_a_e(CPU* cpu) { sbc_a_n(cpu, REG_E); }
static void sbc_a_h(CPU* cpu) { sbc_a_n(cpu, REG_H); }
static void sbc_a_l(CPU* cpu) { sbc_a_n(cpu, REG_L); }
static void sbc_a_hl(CPU* cpu) { sbc_a_n(cpu, read_byte(REG_HL)); }
static void sbc_a_a(CPU* cpu) { sbc_a_n(cpu, REG_A); }
// INC
static void inc_b(CPU* cpu) { inc_n(cpu, &REG_B); }
static void inc_c(CPU* cpu) { inc_n(cpu, &REG_C); }
static void inc_d(CPU* cpu) { inc_n(cpu, &REG_D); }
static void inc_e(CPU* cpu) { inc_n(cpu, &REG_E); }
static void inc_h(CPU* cpu) { inc_n(cpu, &REG_H); }
static void inc_l(CPU* cpu) { inc_n(cpu, &REG_L); }
static void inc_a(CPU* cpu) { inc_n(cpu, &REG_A); }
static void inc_hl8(CPU* cpu) { 
    uint8_t value = read_byte(REG_HL);
    inc_n(cpu, &value);
    write_byte(REG_HL, value);
}
static void inc_bc(CPU* cpu) { inc_nn(cpu, &REG_BC); }
static void inc_de(CPU* cpu) { inc_nn(cpu, &REG_DE); }
static void inc_hl(CPU* cpu) { inc_nn(cpu, &REG_HL); }
static void inc_sp(CPU* cpu) { inc_nn(cpu, &REG_SP); }
// DEC
static void dec_b(CPU* cpu) { dec_n(cpu, &REG_B); }
static void dec_c(CPU* cpu) { dec_n(cpu, &REG_C); }
static void dec_d(CPU* cpu) { dec_n(cpu, &REG_D); }
static void dec_e(CPU* cpu) { dec_n(cpu, &REG_E); }
static void dec_h(CPU* cpu) { dec_n(cpu, &REG_H); }
static void dec_l(CPU* cpu) { dec_n(cpu, &REG_L); }
static void dec_a(CPU* cpu) { dec_n(cpu, &REG_A); }
static void dec_hl8(CPU* cpu) { 
    uint8_t value = read_byte(REG_HL);
    dec_n(cpu, &value);
    write_byte(REG_HL, value);
}
static void dec_bc(CPU* cpu) { dec_nn(cpu, &REG_BC); }
static void dec_de(CPU* cpu) { dec_nn(cpu, &REG_DE); }
static void dec_hl(CPU* cpu) { dec_nn(cpu, &REG_HL); }
static void dec_sp(CPU* cpu) { dec_nn(cpu, &REG_SP); }
// PUSH
static void push_bc(CPU* cpu) { push_nn(cpu, REG_BC); }
static void push_de(CPU* cpu) { push_nn(cpu, REG_DE); }
static void push_hl(CPU* cpu) { push_nn(cpu, REG_HL); }
static void push_af(CPU* cpu) { push_nn(cpu, REG_AF & 0xFFF0); } // lower 4 bits of F are always 0
// POP
static void pop_bc(CPU* cpu) { REG_BC = pop_nn(cpu); }
static void pop_de(CPU* cpu) { REG_DE = pop_nn(cpu); }
static void pop_hl(CPU* cpu) { REG_HL = pop_nn(cpu); }
static void pop_af(CPU* cpu) { REG_AF = pop_nn(cpu) & 0xFFF0; } // lower 4 bits of F always 0
// AND
static void and_a_b(CPU* cpu) { and_a_n(cpu, REG_B); }
static void and_a_c(CPU* cpu) { and_a_n(cpu, REG_C); }
static void and_a_d(CPU* cpu) { and_a_n(cpu, REG_D); }
static void and_a_e(CPU* cpu) { and_a_n(cpu, REG_E); }
static void and_a_h(CPU* cpu) { and_a_n(cpu, REG_H); }
static void and_a_l(CPU* cpu) { and_a_n(cpu, REG_L); }
static void and_a_hl(CPU* cpu) { and_a_n(cpu, read_byte(REG_HL)); }
static void and_a_a(CPU* cpu) { and_a_n(cpu, REG_A); }
// OR
static void or_a_b(CPU* cpu) { or_a_n(cpu, REG_B); }
static void or_a_c(CPU* cpu) { or_a_n(cpu, REG_C); }
static void or_a_d(CPU* cpu) { or_a_n(cpu, REG_D); }
static void or_a_e(CPU* cpu) { or_a_n(cpu, REG_E); }
static void or_a_h(CPU* cpu) { or_a_n(cpu, REG_H); }
static void or_a_l(CPU* cpu) { or_a_n(cpu, REG_L); }
static void or_a_hl(CPU* cpu) { or_a_n(cpu, read_byte(REG_HL)); }
static void or_a_a(CPU* cpu) { or_a_n(cpu, REG_A); }
// XOR
static void xor_a_b(CPU* cpu) { xor_a_n(cpu, REG_B); }
static void xor_a_c(CPU* cpu) { xor_a_n(cpu, REG_C); }
static void xor_a_d(CPU* cpu) { xor_a_n(cpu, REG_D); }
static void xor_a_e(CPU* cpu) { xor_a_n(cpu, REG_E); }
static void xor_a_h(CPU* cpu) { xor_a_n(cpu, REG_H); }
static void xor_a_l(CPU* cpu) { xor_a_n(cpu, REG_L); }
static void xor_a_hl(CPU* cpu) { xor_a_n(cpu, read_byte(REG_HL)); }
static void xor_a_a(CPU* cpu) { xor_a_n(cpu, REG_A); }
// CP
static void cp_a_b(CPU* cpu) { cp_a_n(cpu, REG_B); }
static void cp_a_c(CPU* cpu) { cp_a_n(cpu, REG_C); }
static void cp_a_d(CPU* cpu) { cp_a_n(cpu, REG_D); }
static void cp_a_e(CPU* cpu) { cp_a_n(cpu, REG_E); }
static void cp_a_h(CPU* cpu) { cp_a_n(cpu, REG_H); }
static void cp_a_l(CPU* cpu) { cp_a_n(cpu, REG_L); }
static void cp_a_hl(CPU* cpu) { cp_a_n(cpu, read_byte(REG_HL)); }
static void cp_a_a(CPU* cpu) { cp_a_n(cpu, REG_A); }
static void cp_a_imm(CPU* cpu) 
{
    uint8_t value = read_byte(REG_PC++);
    cp_a_n(cpu, value);
}
// SWAP
static void swap_a(CPU* cpu) { swap_n(cpu, &REG_A); }
static void swap_b(CPU* cpu) { swap_n(cpu, &REG_B); }
static void swap_c(CPU* cpu) { swap_n(cpu, &REG_C); }
static void swap_d(CPU* cpu) { swap_n(cpu, &REG_D); }
static void swap_e(CPU* cpu) { swap_n(cpu, &REG_E); }
static void swap_h(CPU* cpu) { swap_n(cpu, &REG_H); }
static void swap_l(CPU* cpu) { swap_n(cpu, &REG_L); }
static void swap_hlp(CPU* cpu) { swap_hl(cpu); }
// DAA
static void op_daa(CPU* cpu) { daa_a(cpu); }
// CARRY
static void op_cpl(CPU* cpu) { cpl_a(cpu); }
static void op_ccf(CPU* cpu) { ccf(cpu); }
static void op_scf(CPU* cpu) { scf(cpu); }
// ROTATE
static void rlc_a(CPU* cpu) { rlc(cpu, &REG_A); }
static void rlc_b(CPU* cpu) { rlc(cpu, &REG_B); }
static void rlc_c(CPU* cpu) { rlc(cpu, &REG_C); }
static void rlc_d(CPU* cpu) { rlc(cpu, &REG_D); }
static void rlc_e(CPU* cpu) { rlc(cpu, &REG_E); }
static void rlc_h(CPU* cpu) { rlc(cpu, &REG_H); }
static void rlc_l(CPU* cpu) { rlc(cpu, &REG_L); }
static void rlc_hlp(CPU* cpu)
{ 
    uint8_t value = read_byte(REG_HL);
    rlc(cpu, &value);
    write_byte(REG_HL, value);
}
static void rl_hlp(CPU* cpu)
{
    uint8_t value = read_byte(REG_HL);
    rl(cpu, &value);
    write_byte(REG_HL, value);
}
static void rl_a(CPU* cpu) { rl(cpu, &REG_A); }
static void rl_b(CPU* cpu) { rl(cpu, &REG_B); }
static void rl_c(CPU* cpu) { rl(cpu, &REG_C); }
static void rl_d(CPU* cpu) { rl(cpu, &REG_D); }
static void rl_e(CPU* cpu) { rl(cpu, &REG_E); }
static void rl_h(CPU* cpu) { rl(cpu, &REG_H); }
static void rl_l(CPU* cpu) { rl(cpu, &REG_L); }
static void rrc_a(CPU* cpu) { rrc(cpu, &REG_A); }
static void rrc_b(CPU* cpu) { rrc(cpu, &REG_B); }
static void rrc_c(CPU* cpu) { rrc(cpu, &REG_C); }
static void rrc_d(CPU* cpu) { rrc(cpu, &REG_D); }
static void rrc_e(CPU* cpu) { rrc(cpu, &REG_E); }
static void rrc_h(CPU* cpu) { rrc(cpu, &REG_H); }
static void rrc_l(CPU* cpu) { rrc(cpu, &REG_L); }
static void rrc_hlp(CPU* cpu)
{
    uint8_t value = read_byte(REG_HL);
    rrc(cpu, &value);
    write_byte(REG_HL, value);
}
static void rr_a(CPU* cpu) { rrn(cpu, &REG_A); }
static void rr_b(CPU* cpu) { rrn(cpu, &REG_B); }
static void rr_c(CPU* cpu) { rrn(cpu, &REG_C); }
static void rr_d(CPU* cpu) { rrn(cpu, &REG_D); }
static void rr_e(CPU* cpu) { rrn(cpu, &REG_E); }
static void rr_h(CPU* cpu) { rrn(cpu, &REG_H); }
static void rr_l(CPU* cpu) { rrn(cpu, &REG_L); }
static void rr_hlp(CPU* cpu)
{
    uint8_t value = read_byte(REG_HL);
    rrn(cpu, &value);
    write_byte(REG_HL, value);
}
// Shifts
static void sla_a(CPU* cpu) { sla(cpu, &REG_A); }
static void sla_b(CPU* cpu) { sla(cpu, &REG_B); }
static void sla_c(CPU* cpu) { sla(cpu, &REG_C); }
static void sla_d(CPU* cpu) { sla(cpu, &REG_D); }
static void sla_e(CPU* cpu) { sla(cpu, &REG_E); }
static void sla_h(CPU* cpu) { sla(cpu, &REG_H); }
static void sla_l(CPU* cpu) { sla(cpu, &REG_L); }
static void sla_hlp(CPU* cpu) { uint8_t v = read_byte(REG_HL); sla(cpu, &v); write_byte(REG_HL, v); }
static void sra_a(CPU* cpu) { sra(cpu, &REG_A); }
static void sra_b(CPU* cpu) { sra(cpu, &REG_B); }
static void sra_c(CPU* cpu) { sra(cpu, &REG_C); }
static void sra_d(CPU* cpu) { sra(cpu, &REG_D); }
static void sra_e(CPU* cpu) { sra(cpu, &REG_E); }
static void sra_h(CPU* cpu) { sra(cpu, &REG_H); }
static void sra_l(CPU* cpu) { sra(cpu, &REG_L); }
static void sra_hlp(CPU* cpu) { uint8_t v = read_byte(REG_HL); sra(cpu, &v); write_byte(REG_HL, v); }
static void srl_a(CPU* cpu) { srl(cpu, &REG_A); }
static void srl_b(CPU* cpu) { srl(cpu, &REG_B); }
static void srl_c(CPU* cpu) { srl(cpu, &REG_C); }
static void srl_d(CPU* cpu) { srl(cpu, &REG_D); }
static void srl_e(CPU* cpu) { srl(cpu, &REG_E); }
static void srl_h(CPU* cpu) { srl(cpu, &REG_H); }
static void srl_l(CPU* cpu) { srl(cpu, &REG_L); }
static void srl_hlp(CPU* cpu) { uint8_t v = read_byte(REG_HL); srl(cpu, &v); write_byte(REG_HL, v); }
// BIT
static void bit0_b(CPU* cpu) { bit(0, &REG_B, cpu); }
static void bit0_c(CPU* cpu) { bit(0, &REG_C, cpu); }
static void bit0_d(CPU* cpu) { bit(0, &REG_D, cpu); }
static void bit0_e(CPU* cpu) { bit(0, &REG_E, cpu); }
static void bit0_h(CPU* cpu) { bit(0, &REG_H, cpu); }
static void bit0_l(CPU* cpu) { bit(0, &REG_L, cpu); }
static void bit0_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(0, &val, cpu);
}
static void bit0_a(CPU* cpu) { bit(0, &REG_A, cpu); }

static void bit1_b(CPU* cpu) { bit(1, &REG_B, cpu); }
static void bit1_c(CPU* cpu) { bit(1, &REG_C, cpu); }
static void bit1_d(CPU* cpu) { bit(1, &REG_D, cpu); }
static void bit1_e(CPU* cpu) { bit(1, &REG_E, cpu); }
static void bit1_h(CPU* cpu) { bit(1, &REG_H, cpu); }
static void bit1_l(CPU* cpu) { bit(1, &REG_L, cpu); }
static void bit1_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(1, &val, cpu);
}
static void bit1_a(CPU* cpu) { bit(1, &REG_A, cpu); }

static void bit2_b(CPU* cpu) { bit(2, &REG_B, cpu); }
static void bit2_c(CPU* cpu) { bit(2, &REG_C, cpu); }
static void bit2_d(CPU* cpu) { bit(2, &REG_D, cpu); }
static void bit2_e(CPU* cpu) { bit(2, &REG_E, cpu); }
static void bit2_h(CPU* cpu) { bit(2, &REG_H, cpu); }
static void bit2_l(CPU* cpu) { bit(2, &REG_L, cpu); }
static void bit2_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(2, &val, cpu);
}
static void bit2_a(CPU* cpu) { bit(2, &REG_A, cpu); }

static void bit3_b(CPU* cpu) { bit(3, &REG_B, cpu); }
static void bit3_c(CPU* cpu) { bit(3, &REG_C, cpu); }
static void bit3_d(CPU* cpu) { bit(3, &REG_D, cpu); }
static void bit3_e(CPU* cpu) { bit(3, &REG_E, cpu); }
static void bit3_h(CPU* cpu) { bit(3, &REG_H, cpu); }
static void bit3_l(CPU* cpu) { bit(3, &REG_L, cpu); }
static void bit3_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(3, &val, cpu);
}
static void bit3_a(CPU* cpu) { bit(3, &REG_A, cpu); }

static void bit4_b(CPU* cpu) { bit(4, &REG_B, cpu); }
static void bit4_c(CPU* cpu) { bit(4, &REG_C, cpu); }
static void bit4_d(CPU* cpu) { bit(4, &REG_D, cpu); }
static void bit4_e(CPU* cpu) { bit(4, &REG_E, cpu); }
static void bit4_h(CPU* cpu) { bit(4, &REG_H, cpu); }
static void bit4_l(CPU* cpu) { bit(4, &REG_L, cpu); }
static void bit4_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(4, &val, cpu);
}
static void bit4_a(CPU* cpu) { bit(4, &REG_A, cpu); }

static void bit5_b(CPU* cpu) { bit(5, &REG_B, cpu); }
static void bit5_c(CPU* cpu) { bit(5, &REG_C, cpu); }
static void bit5_d(CPU* cpu) { bit(5, &REG_D, cpu); }
static void bit5_e(CPU* cpu) { bit(5, &REG_E, cpu); }
static void bit5_h(CPU* cpu) { bit(5, &REG_H, cpu); }
static void bit5_l(CPU* cpu) { bit(5, &REG_L, cpu); }
static void bit5_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(5, &val, cpu);
}
static void bit5_a(CPU* cpu) { bit(5, &REG_A, cpu); }

static void bit6_b(CPU* cpu) { bit(6, &REG_B, cpu); }
static void bit6_c(CPU* cpu) { bit(6, &REG_C, cpu); }
static void bit6_d(CPU* cpu) { bit(6, &REG_D, cpu); }
static void bit6_e(CPU* cpu) { bit(6, &REG_E, cpu); }
static void bit6_h(CPU* cpu) { bit(6, &REG_H, cpu); }
static void bit6_l(CPU* cpu) { bit(6, &REG_L, cpu); }
static void bit6_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(6, &val, cpu);
}
static void bit6_a(CPU* cpu) { bit(6, &REG_A, cpu); }

static void bit7_b(CPU* cpu) { bit(7, &REG_B, cpu); }
static void bit7_c(CPU* cpu) { bit(7, &REG_C, cpu); }
static void bit7_d(CPU* cpu) { bit(7, &REG_D, cpu); }
static void bit7_e(CPU* cpu) { bit(7, &REG_E, cpu); }
static void bit7_h(CPU* cpu) { bit(7, &REG_H, cpu); }
static void bit7_l(CPU* cpu) { bit(7, &REG_L, cpu); }
static void bit7_hlp(CPU* cpu) 
{
    uint8_t val = read_byte(REG_HL);
    bit(7, &val, cpu);
}
static void bit7_a(CPU* cpu) { bit(7, &REG_A, cpu); }
// RES
static void res0_b(CPU* cpu) { res(0, &REG_B); }
static void res0_c(CPU* cpu) { res(0, &REG_C); }
static void res0_d(CPU* cpu) { res(0, &REG_D); }
static void res0_e(CPU* cpu) { res(0, &REG_E); }
static void res0_h(CPU* cpu) { res(0, &REG_H); }
static void res0_l(CPU* cpu) { res(0, &REG_L); }
static void res0_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);
    res(0, &val);
    write_byte(addr, val);
}
static void res0_a(CPU* cpu) { res(0, &REG_A); }

static void res1_a(CPU* cpu) { REG_A &= ~(1 << 1); }
static void res1_b(CPU* cpu) { REG_B &= ~(1 << 1); }
static void res1_c(CPU* cpu) { REG_C &= ~(1 << 1); }
static void res1_d(CPU* cpu) { REG_D &= ~(1 << 1); }
static void res1_e(CPU* cpu) { REG_E &= ~(1 << 1); }
static void res1_h(CPU* cpu) { REG_H &= ~(1 << 1); }
static void res1_l(CPU* cpu) { REG_L &= ~(1 << 1); }
static void res1_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 1);
    write_byte(addr, val);
}

static void res2_a(CPU* cpu) { REG_A &= ~(1 << 2); }
static void res2_b(CPU* cpu) { REG_B &= ~(1 << 2); }
static void res2_c(CPU* cpu) { REG_C &= ~(1 << 2); }
static void res2_d(CPU* cpu) { REG_D &= ~(1 << 2); }
static void res2_e(CPU* cpu) { REG_E &= ~(1 << 2); }
static void res2_h(CPU* cpu) { REG_H &= ~(1 << 2); }
static void res2_l(CPU* cpu) { REG_L &= ~(1 << 2); }
static void res2_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 2);
    write_byte(addr, val);
}

static void res3_a(CPU* cpu) { REG_A &= ~(1 << 3); }
static void res3_b(CPU* cpu) { REG_B &= ~(1 << 3); }
static void res3_c(CPU* cpu) { REG_C &= ~(1 << 3); }
static void res3_d(CPU* cpu) { REG_D &= ~(1 << 3); }
static void res3_e(CPU* cpu) { REG_E &= ~(1 << 3); }
static void res3_h(CPU* cpu) { REG_H &= ~(1 << 3); }
static void res3_l(CPU* cpu) { REG_L &= ~(1 << 3); }
static void res3_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 3);
    write_byte(addr, val);
}

static void res4_a(CPU* cpu) { REG_A &= ~(1 << 4); }
static void res4_b(CPU* cpu) { REG_B &= ~(1 << 4); }
static void res4_c(CPU* cpu) { REG_C &= ~(1 << 4); }
static void res4_d(CPU* cpu) { REG_D &= ~(1 << 4); }
static void res4_e(CPU* cpu) { REG_E &= ~(1 << 4); }
static void res4_h(CPU* cpu) { REG_H &= ~(1 << 4); }
static void res4_l(CPU* cpu) { REG_L &= ~(1 << 4); }
static void res4_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 4);
    write_byte(addr, val);
}

static void res5_a(CPU* cpu) { REG_A &= ~(1 << 5); }
static void res5_b(CPU* cpu) { REG_B &= ~(1 << 5); }
static void res5_c(CPU* cpu) { REG_C &= ~(1 << 5); }
static void res5_d(CPU* cpu) { REG_D &= ~(1 << 5); }
static void res5_e(CPU* cpu) { REG_E &= ~(1 << 5); }
static void res5_h(CPU* cpu) { REG_H &= ~(1 << 5); }
static void res5_l(CPU* cpu) { REG_L &= ~(1 << 5); }
static void res5_hlp(CPU* cpu)
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 5);
    write_byte(addr, val);
}

static void res6_a(CPU* cpu) { REG_A &= ~(1 << 6); }
static void res6_b(CPU* cpu) { REG_B &= ~(1 << 6); }
static void res6_c(CPU* cpu) { REG_C &= ~(1 << 6); }
static void res6_d(CPU* cpu) { REG_D &= ~(1 << 6); }
static void res6_e(CPU* cpu) { REG_E &= ~(1 << 6); }
static void res6_h(CPU* cpu) { REG_H &= ~(1 << 6); }
static void res6_l(CPU* cpu) { REG_L &= ~(1 << 6); }
static void res6_hlp(CPU* cpu)
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 6);
    write_byte(addr, val);
}

static void res7_a(CPU* cpu) { REG_A &= ~(1 << 7); }
static void res7_b(CPU* cpu) { REG_B &= ~(1 << 7); }
static void res7_c(CPU* cpu) { REG_C &= ~(1 << 7); }
static void res7_d(CPU* cpu) { REG_D &= ~(1 << 7); }
static void res7_e(CPU* cpu) { REG_E &= ~(1 << 7); }
static void res7_h(CPU* cpu) { REG_H &= ~(1 << 7); }
static void res7_l(CPU* cpu) { REG_L &= ~(1 << 7); }
static void res7_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val &= ~(1 << 7);
    write_byte(addr, val);
}
// SET
static void set0_b(CPU* cpu) { set(0, &REG_B); }
static void set0_c(CPU* cpu) { set(0, &REG_C); }
static void set0_d(CPU* cpu) { set(0, &REG_D); }
static void set0_e(CPU* cpu) { set(0, &REG_E); }
static void set0_h(CPU* cpu) { set(0, &REG_H); }
static void set0_l(CPU* cpu) { set(0, &REG_L); }
static void set0_hlp(CPU* cpu)
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);
    set(0, &val);
    write_byte(addr, val);
}
static void set0_a(CPU* cpu) { set(0, &REG_A); }

static void set1_a(CPU* cpu) { REG_A |= (1 << 1); }
static void set1_b(CPU* cpu) { REG_B |= (1 << 1); }
static void set1_c(CPU* cpu) { REG_C |= (1 << 1); }
static void set1_d(CPU* cpu) { REG_D |= (1 << 1); }
static void set1_e(CPU* cpu) { REG_E |= (1 << 1); }
static void set1_h(CPU* cpu) { REG_H |= (1 << 1); }
static void set1_l(CPU* cpu) { REG_L |= (1 << 1); }
static void set1_hlp(CPU* cpu) 
{ 
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 1);
    write_byte(addr, val);
}

static void set2_a(CPU* cpu) { REG_A |= (1 << 2); }
static void set2_b(CPU* cpu) { REG_B |= (1 << 2); }
static void set2_c(CPU* cpu) { REG_C |= (1 << 2); }
static void set2_d(CPU* cpu) { REG_D |= (1 << 2); }
static void set2_e(CPU* cpu) { REG_E |= (1 << 2); }
static void set2_h(CPU* cpu) { REG_H |= (1 << 2); }
static void set2_l(CPU* cpu) { REG_L |= (1 << 2); }
static void set2_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 2);
    write_byte(addr, val);
}

static void set3_a(CPU* cpu) { REG_A |= (1 << 3); }
static void set3_b(CPU* cpu) { REG_B |= (1 << 3); }
static void set3_c(CPU* cpu) { REG_C |= (1 << 3); }
static void set3_d(CPU* cpu) { REG_D |= (1 << 3); }
static void set3_e(CPU* cpu) { REG_E |= (1 << 3); }
static void set3_h(CPU* cpu) { REG_H |= (1 << 3); }
static void set3_l(CPU* cpu) { REG_L |= (1 << 3); }
static void set3_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 3);
    write_byte(addr, val);
}

static void set4_a(CPU* cpu) { REG_A |= (1 << 4); }
static void set4_b(CPU* cpu) { REG_B |= (1 << 4); }
static void set4_c(CPU* cpu) { REG_C |= (1 << 4); }
static void set4_d(CPU* cpu) { REG_D |= (1 << 4); }
static void set4_e(CPU* cpu) { REG_E |= (1 << 4); }
static void set4_h(CPU* cpu) { REG_H |= (1 << 4); }
static void set4_l(CPU* cpu) { REG_L |= (1 << 4); }
static void set4_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 4);
    write_byte(addr, val);
}

static void set5_a(CPU* cpu) { REG_A |= (1 << 5); }
static void set5_b(CPU* cpu) { REG_B |= (1 << 5); }
static void set5_c(CPU* cpu) { REG_C |= (1 << 5); }
static void set5_d(CPU* cpu) { REG_D |= (1 << 5); }
static void set5_e(CPU* cpu) { REG_E |= (1 << 5); }
static void set5_h(CPU* cpu) { REG_H |= (1 << 5); }
static void set5_l(CPU* cpu) { REG_L |= (1 << 5); }
static void set5_hlp(CPU* cpu) 
{
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 5);
    write_byte(addr, val);
}

static void set6_a(CPU* cpu) { REG_A |= (1 << 6); }
static void set6_b(CPU* cpu) { REG_B |= (1 << 6); }
static void set6_c(CPU* cpu) { REG_C |= (1 << 6); }
static void set6_d(CPU* cpu) { REG_D |= (1 << 6); }
static void set6_e(CPU* cpu) { REG_E |= (1 << 6); }
static void set6_h(CPU* cpu) { REG_H |= (1 << 6); }
static void set6_l(CPU* cpu) { REG_L |= (1 << 6); }
static void set6_hlp(CPU* cpu) 
{   
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 6);
    write_byte(addr, val);
}

static void set7_a(CPU* cpu) { REG_A |= (1 << 7); }
static void set7_b(CPU* cpu) { REG_B |= (1 << 7); }
static void set7_c(CPU* cpu) { REG_C |= (1 << 7); }
static void set7_d(CPU* cpu) { REG_D |= (1 << 7); }
static void set7_e(CPU* cpu) { REG_E |= (1 << 7); }
static void set7_h(CPU* cpu) { REG_H |= (1 << 7); }
static void set7_l(CPU* cpu) { REG_L |= (1 << 7); }
static void set7_hlp(CPU* cpu) 
{ 
    uint16_t addr = REG_HL;
    uint8_t val = read_byte(addr);       
    val |= (1 << 7);
    write_byte(addr, val);
}
// JP
static void jp_nn_op(CPU* cpu) { jp_nn(cpu); }
static void jp_nz_nn(CPU* cpu) { jp_cc_nn(cpu, NZ); }
static void jp_z_nn(CPU* cpu)  { jp_cc_nn(cpu, Z); }
static void jp_nc_nn(CPU* cpu) { jp_cc_nn(cpu, NC); }
static void jp_c_nn(CPU* cpu)  { jp_cc_nn(cpu, C); }
static void jp_hl_op(CPU* cpu) { REG_PC = REG_HL; }
static void jr_n_op(CPU* cpu) { jr_n(cpu); }
static void jr_nz_op(CPU* cpu) { jr_cc_n(cpu, NZ); }
static void jr_z_op(CPU* cpu)  { jr_cc_n(cpu, Z); }
static void jr_nc_op(CPU* cpu) { jr_cc_n(cpu, NC); }
static void jr_c_op(CPU* cpu)  { jr_cc_n(cpu, C); }
// CALL
static void call_nn_op(CPU* cpu) { call_nn(cpu); }
static void call_nz_op(CPU* cpu) { call_cc_nn(cpu, NZ); }
static void call_z_op(CPU* cpu)  { call_cc_nn(cpu, Z); }
static void call_nc_op(CPU* cpu) { call_cc_nn(cpu, NC); }
static void call_c_op(CPU* cpu)  { call_cc_nn(cpu, C); }
// RST
static void rst_00(CPU* cpu) { rst_n(cpu, 0x00); }
static void rst_08(CPU* cpu) { rst_n(cpu, 0x08); }
static void rst_10(CPU* cpu) { rst_n(cpu, 0x10); }
static void rst_18(CPU* cpu) { rst_n(cpu, 0x18); }
static void rst_20(CPU* cpu) { rst_n(cpu, 0x20); }
static void rst_28(CPU* cpu) { rst_n(cpu, 0x28); }
static void rst_30(CPU* cpu) { rst_n(cpu, 0x30); }
static void rst_38(CPU* cpu) { rst_n(cpu, 0x38); }
// RET
static void ret_op(CPU* cpu) { ret(cpu); }
static void ret_nz(CPU* cpu) { ret_cc(cpu, NZ); }
static void ret_z(CPU* cpu)  { ret_cc(cpu, Z); }
static void ret_nc(CPU* cpu) { ret_cc(cpu, NC); }
static void ret_c(CPU* cpu)  { ret_cc(cpu, C); }
static void reti_op(CPU* cpu) { reti(cpu); }

static void add_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    add_a_n(cpu, value);
}
static void adc_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    adc_a_n(cpu, value);
}
static void sub_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    sub_a_n(cpu, value);
}
static void sbc_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    sbc_a_n(cpu, value);
}
static void and_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    and_a_n(cpu, value);
}
static void xor_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    xor_a_n(cpu, value);
}
static void or_a_imm(CPU* cpu) {
    uint8_t value = read_byte(REG_PC++);
    or_a_n(cpu, value);
}

Opcode opcodes[256];
Opcode cb_opcodes[256];

const uint8_t opcode_cycles[256] = {
    4, 12, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4,
    4, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4,
    8, 12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,
    8, 12, 8, 8, 12, 12, 12, 4, 8, 8, 8, 8, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
    8, 12, 12, 16, 12, 16, 8, 16, 8, 16, 12, 4, 12, 24, 8, 16,
    8, 12, 12, 0, 12, 16, 8, 16, 8, 16, 12, 0, 12, 0, 8, 16,
    12, 12, 8, 0, 0, 16, 8, 16, 16, 4, 16, 0, 0, 0, 8, 16,
    12, 12, 8, 4, 0, 16, 8, 16, 12, 8, 16, 4, 0, 0, 8, 16
};

const uint8_t cb_opcode_cycles[256] = {
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8
};

void init_opcodes()
{
    for (int i = 0; i < 256; i++) opcodes[i] = nop;
    for (int i = 0; i < 256; i++) cb_opcodes[i] = nop;

    // MISC
    opcodes[0x00] = nop;
    opcodes[0x76] = halt;
    opcodes[0x10] = stop;
    opcodes[0xF3] = di;
    opcodes[0xFB] = ei;

    // 16-bit load
    opcodes[0x01] = ld_bc_u16;  // LD BC, nn
    opcodes[0x11] = ld_de_u16;
    opcodes[0x21] = ld_hl_u16;
    opcodes[0x31] = ld_sp_u16;

    // Memory LD
    opcodes[0x0A] = ld_a_bc;
    opcodes[0x1A] = ld_a_de;
    opcodes[0x02] = ld_bc_a;
    opcodes[0x12] = ld_de_a;
    opcodes[0x2A] = ld_a_hli;
    opcodes[0x22] = ld_hli_a;
    opcodes[0x3A] = ld_a_hld;
    opcodes[0x32] = ld_hld_a;
    opcodes[0xE0] = ldh_n_a_op;
    opcodes[0xF0] = ldh_a_n_op;
    opcodes[0xF9] = ld_sp_hl_op;
    opcodes[0xF8] = ldhl_sp_n_op;
    opcodes[0x08] = ld_nn_sp_op;
    opcodes[0xEA] = ld_nn_a_op;
    opcodes[0xFA] = ld_a_nn;
    opcodes[0x36] = ld_hl_n;
    

    // 16-bit ADD
    opcodes[0x09] = add_hl_bc;
    opcodes[0x19] = add_hl_de;
    opcodes[0x29] = add_hl_hl;
    opcodes[0x39] = add_hl_sp;
    
    // 16-bit INC/DEC
    opcodes[0x03] = inc_bc;
    opcodes[0x13] = inc_de;
    opcodes[0x23] = inc_hl;
    opcodes[0x33] = inc_sp;
    opcodes[0x0B] = dec_bc;
    opcodes[0x1B] = dec_de;
    opcodes[0x2B] = dec_hl;
    opcodes[0x3B] = dec_sp;

    // 8-bit INC/DEC
    opcodes[0x04] = inc_b;
    opcodes[0x05] = dec_b;
    opcodes[0x0C] = inc_c;
    opcodes[0x0D] = dec_c;
    opcodes[0x14] = inc_d;
    opcodes[0x15] = dec_d;
    opcodes[0x1C] = inc_e;
    opcodes[0x1D] = dec_e;
    opcodes[0x24] = inc_h;
    opcodes[0x25] = dec_h;
    opcodes[0x2C] = inc_l;
    opcodes[0x2D] = dec_l;
    opcodes[0x3C] = inc_a;
    opcodes[0x3D] = dec_a;
    opcodes[0x34] = inc_hl8;
    opcodes[0x35] = dec_hl8;

    // 8-bit LD r,n
    opcodes[0x06] = ld_b_n;
    opcodes[0x0E] = ld_c_n;
    opcodes[0x16] = ld_d_n;
    opcodes[0x1E] = ld_e_n;
    opcodes[0x26] = ld_h_n;
    opcodes[0x2E] = ld_l_n;
    opcodes[0x3E] = ld_a_n;

    // 8-bit LD r,r'
    opcodes[0x41] = ld_b_c;
    opcodes[0x47] = ld_b_a;
    opcodes[0x78] = ld_a_b;
    opcodes[0x79] = load_a_c;
    opcodes[0x7A] = ld_a_d;
    opcodes[0x7B] = ld_a_e;
    opcodes[0x7C] = ld_a_h;
    opcodes[0x7D] = ld_a_l;
    opcodes[0x7E] = ld_a_hl;
    opcodes[0x77] = ld_hl_a;
    opcodes[0x40] = ld_b_b;
    opcodes[0x41] = ld_b_c;
    opcodes[0x42] = ld_b_d;
    opcodes[0x43] = ld_b_e;
    opcodes[0x44] = ld_b_h;
    opcodes[0x45] = ld_b_l;
    opcodes[0x46] = ld_b_hl;  // LD B,(HL)
    opcodes[0x47] = ld_b_a;

    opcodes[0x48] = ld_c_b;
    opcodes[0x49] = ld_c_c;
    opcodes[0x4A] = ld_c_d;
    opcodes[0x4B] = ld_c_e;
    opcodes[0x4C] = ld_c_h;
    opcodes[0x4D] = ld_c_l;
    opcodes[0x4E] = ld_c_hl;  // LD C,(HL)
    opcodes[0x4F] = ld_c_a;

    opcodes[0x50] = ld_d_b;
    opcodes[0x51] = ld_d_c;
    opcodes[0x52] = ld_d_d;
    opcodes[0x53] = ld_d_e;
    opcodes[0x54] = ld_d_h;
    opcodes[0x55] = ld_d_l;
    opcodes[0x56] = ld_d_hl;  // LD D,(HL)
    opcodes[0x57] = ld_d_a;

    opcodes[0x58] = ld_e_b;
    opcodes[0x59] = ld_e_c;
    opcodes[0x5A] = ld_e_d;
    opcodes[0x5B] = ld_e_e;
    opcodes[0x5C] = ld_e_h;
    opcodes[0x5D] = ld_e_l;
    opcodes[0x5E] = ld_e_hl;  // LD E,(HL)
    opcodes[0x5F] = ld_e_a;

    opcodes[0x60] = ld_h_b;
    opcodes[0x61] = ld_h_c;
    opcodes[0x62] = ld_h_d;
    opcodes[0x63] = ld_h_e;
    opcodes[0x64] = ld_h_h;
    opcodes[0x65] = ld_h_l;
    opcodes[0x66] = ld_h_hl;  // LD H,(HL)
    opcodes[0x67] = ld_h_a;

    opcodes[0x68] = ld_l_b;
    opcodes[0x69] = ld_l_c;
    opcodes[0x6A] = ld_l_d;
    opcodes[0x6B] = ld_l_e;
    opcodes[0x6C] = ld_l_h;
    opcodes[0x6D] = ld_l_l;
    opcodes[0x6E] = ld_l_hl;  // LD L,(HL)
    opcodes[0x6F] = ld_l_a;

    opcodes[0x70] = ld_hl_b;  // LD (HL),B
    opcodes[0x71] = ld_hl_c;  // LD (HL),C
    opcodes[0x72] = ld_hl_d;  // LD (HL),D
    opcodes[0x73] = ld_hl_e;  // LD (HL),E
    opcodes[0x74] = ld_hl_h;  // LD (HL),H
    opcodes[0x75] = ld_hl_l;  // LD (HL),L
    opcodes[0x77] = ld_hl_a;  // LD (HL),A

    opcodes[0x78] = ld_a_b;
    opcodes[0x79] = ld_a_c;
    opcodes[0x7A] = ld_a_d;
    opcodes[0x7B] = ld_a_e;
    opcodes[0x7C] = ld_a_h;
    opcodes[0x7D] = ld_a_l;
    opcodes[0x7E] = ld_a_hl;  // LD A,(HL)
    opcodes[0x7F] = ld_a_a;

    // 8-bit ADD
    opcodes[0x80] = add_a_b;
    opcodes[0x81] = add_a_c;
    opcodes[0x82] = add_a_d;
    opcodes[0x83] = add_a_e;
    opcodes[0x84] = add_a_h;
    opcodes[0x85] = add_a_l;
    opcodes[0x86] = add_a_hl;
    opcodes[0x87] = add_a_a;
    opcodes[0x88] = adc_a_b;
    opcodes[0x89] = adc_a_c;
    opcodes[0x8A] = adc_a_d;
    opcodes[0x8B] = adc_a_e;
    opcodes[0x8C] = adc_a_h;
    opcodes[0x8D] = adc_a_l;
    opcodes[0x8E] = adc_a_hl;
    opcodes[0x8F] = adc_a_a;
    opcodes[0xC6] = add_a_imm;
    opcodes[0xCE] = adc_a_imm;

    // 8-bit SUB
    opcodes[0x90] = sub_a_b;
    opcodes[0x91] = sub_a_c;
    opcodes[0x92] = sub_a_d;
    opcodes[0x93] = sub_a_e;
    opcodes[0x94] = sub_a_h;
    opcodes[0x95] = sub_a_l;
    opcodes[0x96] = sub_a_hl;
    opcodes[0x97] = sub_a_a;
    opcodes[0x98] = sbc_a_b;
    opcodes[0x99] = sbc_a_c;
    opcodes[0x9A] = sbc_a_d;
    opcodes[0x9B] = sbc_a_e;
    opcodes[0x9C] = sbc_a_h;
    opcodes[0x9D] = sbc_a_l;
    opcodes[0x9E] = sbc_a_hl;
    opcodes[0x9F] = sbc_a_a;
    opcodes[0xD6] = sub_a_imm;   
    opcodes[0xDE] = sbc_a_imm;

    // 8-bit AND
    opcodes[0xA0] = and_a_b;
    opcodes[0xA1] = and_a_c;
    opcodes[0xA2] = and_a_d;
    opcodes[0xA3] = and_a_e;
    opcodes[0xA4] = and_a_h;
    opcodes[0xA5] = and_a_l;
    opcodes[0xA6] = and_a_hl;
    opcodes[0xA7] = and_a_a;
    opcodes[0xE6] = and_a_imm;

    // 8-bit OR
    opcodes[0xB0] = or_a_b;
    opcodes[0xB1] = or_a_c;
    opcodes[0xB2] = or_a_d;
    opcodes[0xB3] = or_a_e;
    opcodes[0xB4] = or_a_h;
    opcodes[0xB5] = or_a_l;
    opcodes[0xB6] = or_a_hl;
    opcodes[0xB7] = or_a_a;
    opcodes[0xF6] = or_a_imm;

    // 8-bit XOR
    opcodes[0xA8] = xor_a_b;
    opcodes[0xA9] = xor_a_c;
    opcodes[0xAA] = xor_a_d;
    opcodes[0xAB] = xor_a_e;
    opcodes[0xAC] = xor_a_h;
    opcodes[0xAD] = xor_a_l;
    opcodes[0xAE] = xor_a_hl;
    opcodes[0xAF] = xor_a_a;
    opcodes[0xEE] = xor_a_imm;

    // 8-bit CP
    opcodes[0xB8] = cp_a_b;
    opcodes[0xB9] = cp_a_c;
    opcodes[0xBA] = cp_a_d;
    opcodes[0xBB] = cp_a_e;
    opcodes[0xBC] = cp_a_h;
    opcodes[0xBD] = cp_a_l;
    opcodes[0xBE] = cp_a_hl;
    opcodes[0xBF] = cp_a_a;
    opcodes[0xFE] = cp_a_imm;

    // Arithmetic/Logic
    opcodes[0x27] = op_daa;    
    opcodes[0x2F] = op_cpl;
    opcodes[0x3F] = op_ccf;
    opcodes[0x37] = op_scf;

    // Control flow
    // JP
    opcodes[0xC3] = jp_nn_op;
    opcodes[0xC2] = nop; // jp_nz_nn
    opcodes[0xCA] = nop; // jp_z_nn
    opcodes[0xD2] = nop; // jp_nc_nn
    opcodes[0xDA] = nop; // jp_c_nn

    opcodes[0xE9] = jp_hl_op;

    opcodes[0x18] = jr_n_op;
    opcodes[0x20] = nop; // jr_nz_op
    opcodes[0x28] = nop; // jr_z_op
    opcodes[0x30] = nop; // jr_nc_op
    opcodes[0x38] = nop; // jr_c_op

    // CALL
    opcodes[0xCD] = call_nn_op;
    opcodes[0xC4] = nop; // call_nz_op
    opcodes[0xCC] = nop; // call_z_op
    opcodes[0xD4] = nop; // call_nc_op
    opcodes[0xDC] = nop; // call_c_op
    
    // PUSH
    opcodes[0xC5] = push_bc;
    opcodes[0xD5] = push_de;
    opcodes[0xE5] = push_hl;
    opcodes[0xF5] = push_af;

    // POP
    opcodes[0xC1] = pop_bc;
    opcodes[0xD1] = pop_de;
    opcodes[0xE1] = pop_hl;
    opcodes[0xF1] = pop_af;

    // RST
    opcodes[0xC7] = rst_00;
    opcodes[0xCF] = rst_08;
    opcodes[0xD7] = rst_10;
    opcodes[0xDF] = rst_18;
    opcodes[0xE7] = rst_20;
    opcodes[0xEF] = rst_28;
    opcodes[0xF7] = rst_30;
    opcodes[0xFF] = rst_38;

    // RET
    opcodes[0xC9] = ret_op;
    opcodes[0xC0] = nop; // ret_nz
    opcodes[0xC8] = nop; // ret_z
    opcodes[0xD0] = nop; // ret_nc
    opcodes[0xD8] = nop; // ret_c
    opcodes[0xD9] = reti_op;

    // CB-prefixed opcodes
    // Rotates
    cb_opcodes[0x00] = rlc_b;
    cb_opcodes[0x01] = rlc_c;
    cb_opcodes[0x02] = rlc_d;
    cb_opcodes[0x03] = rlc_e;
    cb_opcodes[0x04] = rlc_h;
    cb_opcodes[0x05] = rlc_l;
    cb_opcodes[0x06] = rlc_hlp;
    cb_opcodes[0x07] = rlc_a;

    cb_opcodes[0x08] = rrc_b;
    cb_opcodes[0x09] = rrc_c;
    cb_opcodes[0x0A] = rrc_d;
    cb_opcodes[0x0B] = rrc_e;
    cb_opcodes[0x0C] = rrc_h;
    cb_opcodes[0x0D] = rrc_l;
    cb_opcodes[0x0E] = rrc_hlp;
    cb_opcodes[0x0F] = rrc_a;

    cb_opcodes[0x10] = rl_b;
    cb_opcodes[0x11] = rl_c;
    cb_opcodes[0x12] = rl_d;
    cb_opcodes[0x13] = rl_e;
    cb_opcodes[0x14] = rl_h;
    cb_opcodes[0x15] = rl_l;
    cb_opcodes[0x16] = rl_hlp;
    cb_opcodes[0x17] = rl_a;

    cb_opcodes[0x18] = rr_b;
    cb_opcodes[0x19] = rr_c;
    cb_opcodes[0x1A] = rr_d;
    cb_opcodes[0x1B] = rr_e;
    cb_opcodes[0x1C] = rr_h;
    cb_opcodes[0x1D] = rr_l;
    cb_opcodes[0x1E] = rr_hlp;
    cb_opcodes[0x1F] = rr_a;

    // Swap
    cb_opcodes[0x30] = swap_b;
    cb_opcodes[0x31] = swap_c;
    cb_opcodes[0x32] = swap_d;
    cb_opcodes[0x33] = swap_e;
    cb_opcodes[0x34] = swap_h;
    cb_opcodes[0x35] = swap_l;
    cb_opcodes[0x36] = swap_hlp;
    cb_opcodes[0x37] = swap_a;

    // Shifts
    cb_opcodes[0x20] = sla_b;
    cb_opcodes[0x21] = sla_c;
    cb_opcodes[0x22] = sla_d;
    cb_opcodes[0x23] = sla_e;
    cb_opcodes[0x24] = sla_h;
    cb_opcodes[0x25] = sla_l;
    cb_opcodes[0x26] = sla_hlp;
    cb_opcodes[0x27] = sla_a;
    cb_opcodes[0x28] = sra_b;
    cb_opcodes[0x29] = sra_c;
    cb_opcodes[0x2A] = sra_d;
    cb_opcodes[0x2B] = sra_e;
    cb_opcodes[0x2C] = sra_h;
    cb_opcodes[0x2D] = sra_l;
    cb_opcodes[0x2E] = sra_hlp;
    cb_opcodes[0x2F] = sra_a;
    cb_opcodes[0x38] = srl_b;
    cb_opcodes[0x39] = srl_c;
    cb_opcodes[0x3A] = srl_d;
    cb_opcodes[0x3B] = srl_e;
    cb_opcodes[0x3C] = srl_h;
    cb_opcodes[0x3D] = srl_l;
    cb_opcodes[0x3E] = srl_hlp;
    cb_opcodes[0x3F] = srl_a;

    // BIT 0-7
    cb_opcodes[0x40] = bit0_b; cb_opcodes[0x41] = bit0_c; cb_opcodes[0x42] = bit0_d; cb_opcodes[0x43] = bit0_e;
    cb_opcodes[0x44] = bit0_h; cb_opcodes[0x45] = bit0_l; cb_opcodes[0x46] = bit0_hlp; cb_opcodes[0x47] = bit0_a;

    cb_opcodes[0x48] = bit1_b; cb_opcodes[0x49] = bit1_c; cb_opcodes[0x4A] = bit1_d; cb_opcodes[0x4B] = bit1_e;
    cb_opcodes[0x4C] = bit1_h; cb_opcodes[0x4D] = bit1_l; cb_opcodes[0x4E] = bit1_hlp; cb_opcodes[0x4F] = bit1_a;

    cb_opcodes[0x50] = bit2_b; cb_opcodes[0x51] = bit2_c; cb_opcodes[0x52] = bit2_d; cb_opcodes[0x53] = bit2_e;
    cb_opcodes[0x54] = bit2_h; cb_opcodes[0x55] = bit2_l; cb_opcodes[0x56] = bit2_hlp; cb_opcodes[0x57] = bit2_a;

    cb_opcodes[0x58] = bit3_b; cb_opcodes[0x59] = bit3_c; cb_opcodes[0x5A] = bit3_d; cb_opcodes[0x5B] = bit3_e;
    cb_opcodes[0x5C] = bit3_h; cb_opcodes[0x5D] = bit3_l; cb_opcodes[0x5E] = bit3_hlp; cb_opcodes[0x5F] = bit3_a;

    cb_opcodes[0x60] = bit4_b; cb_opcodes[0x61] = bit4_c; cb_opcodes[0x62] = bit4_d; cb_opcodes[0x63] = bit4_e;
    cb_opcodes[0x64] = bit4_h; cb_opcodes[0x65] = bit4_l; cb_opcodes[0x66] = bit4_hlp; cb_opcodes[0x67] = bit4_a;

    cb_opcodes[0x68] = bit5_b; cb_opcodes[0x69] = bit5_c; cb_opcodes[0x6A] = bit5_d; cb_opcodes[0x6B] = bit5_e;
    cb_opcodes[0x6C] = bit5_h; cb_opcodes[0x6D] = bit5_l; cb_opcodes[0x6E] = bit5_hlp; cb_opcodes[0x6F] = bit5_a;

    cb_opcodes[0x70] = bit6_b; cb_opcodes[0x71] = bit6_c; cb_opcodes[0x72] = bit6_d; cb_opcodes[0x73] = bit6_e;
    cb_opcodes[0x74] = bit6_h; cb_opcodes[0x75] = bit6_l; cb_opcodes[0x76] = bit6_hlp; cb_opcodes[0x77] = bit6_a;

    cb_opcodes[0x78] = bit7_b; cb_opcodes[0x79] = bit7_c; cb_opcodes[0x7A] = bit7_d; cb_opcodes[0x7B] = bit7_e;
    cb_opcodes[0x7C] = bit7_h; cb_opcodes[0x7D] = bit7_l; cb_opcodes[0x7E] = bit7_hlp; cb_opcodes[0x7F] = bit7_a;

    // RES 0-7
    cb_opcodes[0x80] = res0_b; cb_opcodes[0x81] = res0_c; cb_opcodes[0x82] = res0_d; cb_opcodes[0x83] = res0_e;
    cb_opcodes[0x84] = res0_h; cb_opcodes[0x85] = res0_l; cb_opcodes[0x86] = res0_hlp; cb_opcodes[0x87] = res0_a;

    cb_opcodes[0x88] = res1_b; cb_opcodes[0x89] = res1_c; cb_opcodes[0x8A] = res1_d; cb_opcodes[0x8B] = res1_e;
    cb_opcodes[0x8C] = res1_h; cb_opcodes[0x8D] = res1_l; cb_opcodes[0x8E] = res1_hlp; cb_opcodes[0x8F] = res1_a;

    cb_opcodes[0x90] = res2_b; cb_opcodes[0x91] = res2_c; cb_opcodes[0x92] = res2_d; cb_opcodes[0x93] = res2_e;
    cb_opcodes[0x94] = res2_h; cb_opcodes[0x95] = res2_l; cb_opcodes[0x96] = res2_hlp; cb_opcodes[0x97] = res2_a;

    cb_opcodes[0x98] = res3_b; cb_opcodes[0x99] = res3_c; cb_opcodes[0x9A] = res3_d; cb_opcodes[0x9B] = res3_e;
    cb_opcodes[0x9C] = res3_h; cb_opcodes[0x9D] = res3_l; cb_opcodes[0x9E] = res3_hlp; cb_opcodes[0x9F] = res3_a;

    cb_opcodes[0xA0] = res4_b; cb_opcodes[0xA1] = res4_c; cb_opcodes[0xA2] = res4_d; cb_opcodes[0xA3] = res4_e;
    cb_opcodes[0xA4] = res4_h; cb_opcodes[0xA5] = res4_l; cb_opcodes[0xA6] = res4_hlp; cb_opcodes[0xA7] = res4_a;

    cb_opcodes[0xA8] = res5_b; cb_opcodes[0xA9] = res5_c; cb_opcodes[0xAA] = res5_d; cb_opcodes[0xAB] = res5_e;
    cb_opcodes[0xAC] = res5_h; cb_opcodes[0xAD] = res5_l; cb_opcodes[0xAE] = res5_hlp; cb_opcodes[0xAF] = res5_a;

    cb_opcodes[0xB0] = res6_b; cb_opcodes[0xB1] = res6_c; cb_opcodes[0xB2] = res6_d; cb_opcodes[0xB3] = res6_e;
    cb_opcodes[0xB4] = res6_h; cb_opcodes[0xB5] = res6_l; cb_opcodes[0xB6] = res6_hlp; cb_opcodes[0xB7] = res6_a;

    cb_opcodes[0xB8] = res7_b; cb_opcodes[0xB9] = res7_c; cb_opcodes[0xBA] = res7_d; cb_opcodes[0xBB] = res7_e;
    cb_opcodes[0xBC] = res7_h; cb_opcodes[0xBD] = res7_l; cb_opcodes[0xBE] = res7_hlp; cb_opcodes[0xBF] = res7_a;

    // SET 0-7
    cb_opcodes[0xC0] = set0_b; cb_opcodes[0xC1] = set0_c; cb_opcodes[0xC2] = set0_d; cb_opcodes[0xC3] = set0_e;
    cb_opcodes[0xC4] = set0_h; cb_opcodes[0xC5] = set0_l; cb_opcodes[0xC6] = set0_hlp; cb_opcodes[0xC7] = set0_a;

    cb_opcodes[0xC8] = set1_b; cb_opcodes[0xC9] = set1_c; cb_opcodes[0xCA] = set1_d; cb_opcodes[0xCB] = set1_e;
    cb_opcodes[0xCC] = set1_h; cb_opcodes[0xCD] = set1_l; cb_opcodes[0xCE] = set1_hlp; cb_opcodes[0xCF] = set1_a;

    cb_opcodes[0xD0] = set2_b; cb_opcodes[0xD1] = set2_c; cb_opcodes[0xD2] = set2_d; cb_opcodes[0xD3] = set2_e;
    cb_opcodes[0xD4] = set2_h; cb_opcodes[0xD5] = set2_l; cb_opcodes[0xD6] = set2_hlp; cb_opcodes[0xD7] = set2_a;

    cb_opcodes[0xD8] = set3_b; cb_opcodes[0xD9] = set3_c; cb_opcodes[0xDA] = set3_d; cb_opcodes[0xDB] = set3_e;
    cb_opcodes[0xDC] = set3_h; cb_opcodes[0xDD] = set3_l; cb_opcodes[0xDE] = set3_hlp; cb_opcodes[0xDF] = set3_a;

    cb_opcodes[0xE0] = set4_b; cb_opcodes[0xE1] = set4_c; cb_opcodes[0xE2] = set4_d; cb_opcodes[0xE3] = set4_e;
    cb_opcodes[0xE4] = set4_h; cb_opcodes[0xE5] = set4_l; cb_opcodes[0xE6] = set4_hlp; cb_opcodes[0xE7] = set4_a;

    cb_opcodes[0xE8] = set5_b; cb_opcodes[0xE9] = set5_c; cb_opcodes[0xEA] = set5_d; cb_opcodes[0xEB] = set5_e;
    cb_opcodes[0xEC] = set5_h; cb_opcodes[0xED] = set5_l; cb_opcodes[0xEE] = set5_hlp; cb_opcodes[0xEF] = set5_a;

    cb_opcodes[0xF0] = set6_b; cb_opcodes[0xF1] = set6_c; cb_opcodes[0xF2] = set6_d; cb_opcodes[0xF3] = set6_e;
    cb_opcodes[0xF4] = set6_h; cb_opcodes[0xF5] = set6_l; cb_opcodes[0xF6] = set6_hlp; cb_opcodes[0xF7] = set6_a;

    cb_opcodes[0xF8] = set7_b; cb_opcodes[0xF9] = set7_c; cb_opcodes[0xFA] = set7_d; cb_opcodes[0xFB] = set7_e;
    cb_opcodes[0xFC] = set7_h; cb_opcodes[0xFD] = set7_l; cb_opcodes[0xFE] = set7_hlp; cb_opcodes[0xFF] = set7_a;
}
