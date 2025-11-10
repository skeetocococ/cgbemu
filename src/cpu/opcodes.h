#ifndef OPCODES_H
#define OPCODES_H

#include "instructions.h"

typedef void (*Opcode)(CPU*);
extern Opcode opcodes[256];
extern Opcode cb_opcodes[256];
extern const uint8_t opcode_cycles[256];
extern const uint8_t cb_opcode_cycles[256];

void init_opcodes();

#endif
