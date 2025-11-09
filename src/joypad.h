#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct {
    uint8_t buttons;  // A, B, Select, Start (bits 0-3)
    uint8_t dpad;     // Right, Left, Up, Down (bits 0-3)
} Joypad;

extern Joypad joypad;

// Button masks
#define BUTTON_A      0x01
#define BUTTON_B      0x02
#define BUTTON_SELECT 0x04
#define BUTTON_START  0x08
#define DPAD_RIGHT    0x01
#define DPAD_LEFT     0x02
#define DPAD_UP       0x04
#define DPAD_DOWN     0x08

void handle_input(SDL_Event* event);

#endif
