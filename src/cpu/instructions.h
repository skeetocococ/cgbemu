#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cpu.h"

// Instruction pattern prototypes
void ld_rx_ry(CPU* cpu, uint8_t* dest, uint8_t* src);
void ld_rx_hl(CPU* cpu, uint8_t* dest);
void ld_hl_ry(CPU* cpu, uint8_t* src);
void ld_r_n(CPU* cpu, uint8_t* dest);
void ld_r_nn(CPU* cpu, uint8_t* dest);
void ld_nn_r(CPU* cpu, uint8_t* src);
void ld_r_rr(CPU* cpu, uint8_t* dest, uint16_t addr);
void ld_rr_r(CPU* cpu, uint8_t* src, uint16_t addr);
void ld_a_c(CPU* cpu);
void ld_c_a(CPU* cpu);
void ldd_a_hl(CPU* cpu);
void ldd_hl_a(CPU* cpu);
void ldi_a_hl(CPU* cpu);
void ldi_hl_a(CPU* cpu);
void ldh_n_a(CPU* cpu);
void ldh_a_n(CPU* cpu);
void ld_n_nn(CPU* cpu, uint16_t* dest);
void ld_sp_hl(CPU* cpu);
void ldhl_sp_n(CPU* cpu);
void ld_nn_sp(CPU* cpu);
void push_nn(CPU* cpu, uint16_t value);
uint16_t pop_nn(CPU* cpu);
void add_a_n(CPU* cpu, uint8_t value);
void adc_a_n(CPU* cpu, uint8_t value);
void sub_a_n(CPU* cpu, uint8_t value);
void sbc_a_n(CPU* cpu, uint8_t value);
void and_a_n(CPU* cpu, uint8_t value);
void or_a_n(CPU* cpu, uint8_t value);
void xor_a_n(CPU* cpu, uint8_t value);
void cp_a_n(CPU* cpu, uint8_t value);
void inc_n(CPU* cpu, uint8_t* reg);
void dec_n(CPU* cpu, uint8_t* reg);
void add_hl_n(CPU* cpu, uint16_t* reg);
void add_sp_n(CPU* cpu);
void inc_nn(CPU* cpu, uint16_t* reg);
void dec_nn(CPU* cpu, uint16_t* reg);
void swap_n(CPU* cpu, uint8_t* reg);
void swap_hl(CPU* cpu);
void daa_a(CPU* cpu);
void cpl_a(CPU* cpu);
void ccf(CPU* cpu);
void scf(CPU* cpu);
void rl(CPU* cpu, uint8_t* reg);
void rrc(CPU* cpu, uint8_t* reg);
void rrn(CPU* cpu, uint8_t* reg);
void rlc(CPU* cpu, uint8_t* reg);
void sla(CPU* cpu, uint8_t* reg);
void sra(CPU* cpu, uint8_t* reg);
void srl(CPU* cpu, uint8_t* reg);
void bit(uint8_t bit, uint8_t* reg, CPU* cpu);
void res(uint8_t bit, uint8_t* reg);
void set(uint8_t bit, uint8_t* reg);
void jp_nn(CPU* cpu);
uint8_t jp_cc_nn(CPU* cpu, Condition condition);
void jp_hl(CPU* cpu);
void jr_n(CPU* cpu);
uint8_t jr_cc_n(CPU* cpu, Condition condition);
void call_nn(CPU* cpu);
uint8_t call_cc_nn(CPU* cpu, Condition condition);
void rst_n(CPU* cpu, uint8_t value);
void ret(CPU* cpu);
uint8_t ret_cc(CPU* cpu, Condition condition);
void reti(CPU* cpu);

#endif
