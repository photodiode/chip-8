
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <time.h>
#include <SDL2/SDL.h>

#include "mem.h"
#include "timer.h"
#include "keypad.h"


typedef union {
	struct {
		uint16_t n     :  4;
		uint16_t y     :  4;
		uint16_t x     :  4;
		uint16_t op    :  4;
	};
	struct {
		uint16_t k     :  8;
	};
	struct {
		uint16_t lower :  8;
		uint16_t upper :  8;
	};
	struct {
		uint16_t addr  : 12;
	};
	uint16_t raw;
} op_u;

static struct {
	op_u      ci;    // current instruction
	
	uint16_t  pc;     // program counter
	uint8_t   sp;     // stack pointer

	uint8_t   v[16];  // genereal purpose registers
	uint16_t  i;      // address register
} cpu;

static SDL_mutex* cpu_lock;

static bool       running = true;
static SDL_mutex* running_lock;

static bool       pause = false;
static SDL_mutex* pause_lock;
static SDL_cond*  pause_cond;

static SDL_Thread* cpu_thread;

#include "cpu_instructions.h"

int cpu_function() {

	long delay = 1000000000L / 1000L; // 1kHz
	struct timespec time;
	long elapsed;

	while(1) {

		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec;


		SDL_LockMutex(running_lock);
		if (!running) break;
		SDL_UnlockMutex(running_lock);

		SDL_LockMutex(pause_lock);
		while(pause) SDL_CondWait(pause_cond, pause_lock);
		SDL_UnlockMutex(pause_lock);

		SDL_LockMutex(cpu_lock);
		execute_instruction();
		SDL_UnlockMutex(cpu_lock);


		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec - elapsed;

		nanosleep(&(struct timespec){
			.tv_sec  = 0,
			.tv_nsec = delay - elapsed
		}, NULL);
	}
	
	SDL_UnlockMutex(running_lock);

	return 0;
}

void cpu_reset();
void cpu_initialize() {

	srand(time(NULL));

	cpu_lock     = SDL_CreateMutex();
	running_lock = SDL_CreateMutex();
	pause_lock   = SDL_CreateMutex();
	pause_cond   = SDL_CreateCond();

	cpu_reset();

	cpu_thread = SDL_CreateThread(cpu_function, "cpu_thread", NULL);
}

void unlock_wait_for_keypad() {
	SDL_LockMutex(keypad_cond_lock);
	keypad_pressed = true;
	SDL_CondBroadcast(keypad_cond);
	SDL_UnlockMutex(keypad_cond_lock);
}

void cpu_pause() {
	SDL_LockMutex(pause_lock);
	unlock_wait_for_keypad();
	pause = true;
	SDL_UnlockMutex(pause_lock);
}

void cpu_start() {
	unlock_wait_for_keypad();

	SDL_LockMutex(pause_lock);
	pause = false;
	SDL_CondBroadcast(pause_cond);
	SDL_UnlockMutex(pause_lock);
}

void cpu_reset() {
	cpu_pause();

	SDL_LockMutex(cpu_lock);
	cpu.pc = 0x200;
	cpu.sp = 0;
	cpu.i  = 0;
	memset(cpu.v, 0x00, 16);
	clear_screen();
	SDL_UnlockMutex(cpu_lock);

	cpu_start();
}

void cpu_terminate() {
	cpu_start();

	SDL_LockMutex(running_lock);
	running = false;
	SDL_UnlockMutex(running_lock);

	SDL_WaitThread(cpu_thread, NULL);

	SDL_DestroyMutex(cpu_lock);
	SDL_DestroyMutex(running_lock);
	SDL_DestroyMutex(pause_lock);
	SDL_DestroyCond(pause_cond);
}
