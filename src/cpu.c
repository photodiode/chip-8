
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <time.h>
#include <pthread.h>
//#include <SDL2/SDL.h>

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

static pthread_mutex_t cpu_lock = PTHREAD_MUTEX_INITIALIZER;

static bool            running      = true;
static pthread_mutex_t running_lock = PTHREAD_MUTEX_INITIALIZER;

static bool            pause      = false;
static pthread_mutex_t pause_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  pause_cond = PTHREAD_COND_INITIALIZER;

static pthread_t cpu_thread;

#include "cpu_instructions.h"

void* cpu_function() {

	long delay = 1000000000L / 1000L; // 1kHz
	struct timespec time;
	long elapsed;

	while(1) {

		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec;


		pthread_mutex_lock(&running_lock);
		if (!running) break;
		pthread_mutex_unlock(&running_lock);

		pthread_mutex_lock(&pause_lock);
		while(pause) pthread_cond_wait(&pause_cond, &pause_lock);
		pthread_mutex_unlock(&pause_lock);

		pthread_mutex_lock(&cpu_lock);
		execute_instruction();
		pthread_mutex_unlock(&cpu_lock);


		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec - elapsed;

		nanosleep(&(struct timespec){
			.tv_sec  = 0,
			.tv_nsec = delay - elapsed
		}, NULL);
	}
	
	pthread_mutex_unlock(&running_lock);

	return NULL;
}

void cpu_reset();
void cpu_initialize() {
	srand(time(NULL));
	pthread_create(&cpu_thread, NULL, cpu_function, NULL);
	cpu_reset();
}

void cpu_pause() {
	pthread_mutex_lock(&pause_lock);
	pause = true;
	pthread_mutex_unlock(&pause_lock);
}

void cpu_start() {
	pthread_mutex_lock(&pause_lock);
	pause = false;
	pthread_cond_broadcast(&pause_cond);
	pthread_mutex_unlock(&pause_lock);
}

void cpu_reset() {
	cpu_pause();

	pthread_mutex_lock(&cpu_lock);
	cpu.pc = 0x200;
	cpu.sp = 0;
	cpu.i  = 0;
	memset(cpu.v, 0x00, 16);
	clear_screen();
	pthread_mutex_unlock(&cpu_lock);

	cpu_start();
}

void cpu_terminate() {
	cpu_start();

	pthread_mutex_lock(&running_lock);
	running = false;
	pthread_mutex_unlock(&running_lock);

	pthread_join(cpu_thread, NULL);
}
