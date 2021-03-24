
#ifndef keypad_h
#define keypad_h

#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

extern bool       keypad[];
extern uint8_t    keypad_pressed;
extern SDL_mutex* keypad_lock;

void keypad_initialize();
void keypad_terminate();
void keypad_get(SDL_Event*);


#endif // keypad_h
