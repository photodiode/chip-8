
#ifndef timer_h
#define timer_h

#include <SDL2/SDL.h>

extern uint8_t    timer_delay;
extern uint8_t    timer_sound;
extern SDL_mutex* timer_lock;

void timer_initialize();
void timer_terminate();

void timer_pause();
void timer_start();
void timer_reset();

#endif // timer_h
