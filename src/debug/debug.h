#ifndef DEBUG_H
#define DEBUG_H

typedef struct {
    int dbg_cpu;
    int dbg_ppu;
    int dbg_boot;
    int dbg_mem;
} Debug;

extern int debug;
extern Debug dbg;

#define DBG_PRINT(...) do { \
    if (debug) printf(__VA_ARGS__); \
} while (0)

#endif
