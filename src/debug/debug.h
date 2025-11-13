#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

typedef struct {
    uint8_t dbg_cpu;
    uint8_t dbg_ppu;
    uint8_t dbg_boot;
    uint8_t dbg_mem;
} Debug;

extern uint8_t debug;
extern Debug dbg;

#define DBG_PRINT(...) do { \
    if (debug) printf(__VA_ARGS__); \
} while (0)

#endif
