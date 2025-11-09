#ifndef OPCODES_H
#define OPCODES_H

#include "instructions/instructions.h"

typedef void (*Opcode)(CPU*);
extern Opcode opcodes[256];
extern Opcode cb_opcodes[256];
const extern uint8_t opcode_cycles[256];
const extern uint8_t cb_opcode_cycles[256];

void init_opcodes();

#endif
