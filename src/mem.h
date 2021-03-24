
#ifndef mem_h
#define mem_h

#include <stdint.h>
#include <SDL2/SDL.h>

#define STACK(OFFSET) ((uint16_t*)&mem[0xEA0])[OFFSET]
#define GFX           (&mem[0xF00])

#define WIDTH    64
#define HEIGHT   32
#define GFX_SIZE 256

extern uint8_t    mem[];
extern SDL_mutex* mem_lock;

void mem_initialize();
void mem_terminate();

void mem_load_program(const char* filename);

#endif // mem_h
