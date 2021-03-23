
#ifndef keypad_h
#define keypad_h

#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>
#include <SDL2/SDL.h>

extern bool            keypad[];
extern uint8_t         keypad_pressed;
extern pthread_mutex_t keypad_lock;

void keypad_get(SDL_Event*);


#endif // keypad_h
