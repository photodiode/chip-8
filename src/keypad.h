
#ifndef keypad_h
#define keypad_h

#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

extern bool    keypad[];
extern bool    keypad_pressed;
extern uint8_t keypad_pressed_key;

extern SDL_mutex* keypad_lock;
extern SDL_mutex* keypad_cond_lock;
extern SDL_cond*  keypad_cond;

void keypad_initialize();
void keypad_terminate();
void keypad_get(SDL_Event*);


#endif // keypad_h
