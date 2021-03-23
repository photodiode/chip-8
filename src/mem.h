
#ifndef mem_h
#define mem_h

#include <stdint.h>
#include <pthread.h>

#define STACK(OFFSET) ((uint16_t*)&mem[0xEA0])[OFFSET]
#define GFX           (&mem[0xF00])

#define WIDTH    64
#define HEIGHT   32
#define GFX_SIZE 256

extern uint8_t         mem[];
extern pthread_mutex_t mem_lock;

void mem_initialize();
void mem_load_program(const char* filename);

#endif // mem_h
