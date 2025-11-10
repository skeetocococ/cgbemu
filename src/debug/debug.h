#ifndef DEBUG_H
#define DEBUG_H

extern int debug;
#define DBG_PRINT(...) do { \
    if (debug) printf(__VA_ARGS__); \
} while (0)

#endif