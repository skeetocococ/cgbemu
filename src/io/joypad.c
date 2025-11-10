#include "joypad.h"
#include "../cpu/cpu.h"

Joypad joypad = { 0xFF, 0xFF };

void handle_input(SDL_Event* event)
{
    int pressed = (event->type == SDL_KEYDOWN);
    switch(event->key.keysym.sym)
    {
        // D-pad
        case SDLK_RIGHT:
            if (pressed) joypad.dpad &= ~DPAD_RIGHT;
            else joypad.dpad |= DPAD_RIGHT;
            break;
        case SDLK_LEFT:
            if (pressed) joypad.dpad &= ~DPAD_LEFT;
            else joypad.dpad |= DPAD_LEFT;
            break;
        case SDLK_UP:
            if (pressed) joypad.dpad &= ~DPAD_UP;
            else joypad.dpad |= DPAD_UP;
            break;
        case SDLK_DOWN:
            if (pressed) joypad.dpad &= ~DPAD_DOWN;
            else joypad.dpad |= DPAD_DOWN;
            break;
            
        // Buttons
        case SDLK_z:  // A
            if (pressed) joypad.buttons &= ~BUTTON_A;
            else joypad.buttons |= BUTTON_A;
            break;
        case SDLK_x:  // B
            if (pressed) joypad.buttons &= ~BUTTON_B;
            else joypad.buttons |= BUTTON_B;
            break;
        case SDLK_RETURN:  // Start
            if (pressed) joypad.buttons &= ~BUTTON_START;
            else joypad.buttons |= BUTTON_START;
            break;
        case SDLK_RSHIFT:  // Select
            if (pressed) joypad.buttons &= ~BUTTON_SELECT;
            else joypad.buttons |= BUTTON_SELECT;
            break;
    }
    if (pressed)
        request_interrupt(JOYPAD_INT);
}
