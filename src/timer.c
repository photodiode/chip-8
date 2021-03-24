
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>

#include <time.h>
#include <SDL2/SDL.h>


uint8_t    timer_delay = 0;
uint8_t    timer_sound = 0;
SDL_mutex* timer_lock;

static bool       running = true;
static SDL_mutex* running_lock;

static bool       pause = false;
static SDL_mutex* pause_lock;
static SDL_cond*  pause_cond;

static SDL_Thread* timer_thread;

int timer_function() {

	long delay = 1000000000L / 60L; // 60 Hz
	struct timespec time;
	long elapsed;

	while(1) {

		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec;


		SDL_LockMutex(timer_lock);
		if (timer_delay > 0) timer_delay--;
		if (timer_sound > 0) timer_sound--;
		SDL_UnlockMutex(timer_lock);
		
		SDL_LockMutex(running_lock);
		if (!running) break;
		SDL_UnlockMutex(running_lock);

		SDL_LockMutex(pause_lock);
		while(pause) SDL_CondWait(pause_cond, pause_lock);
		SDL_UnlockMutex(pause_lock);


		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec - elapsed;

		nanosleep(&(struct timespec){
			.tv_sec  = 0,
			.tv_nsec = delay - elapsed
		}, NULL);
	}

	return 0;
}

void timer_initialize() {
	timer_lock   = SDL_CreateMutex();
	running_lock = SDL_CreateMutex();
	pause_lock   = SDL_CreateMutex();
	pause_cond   = SDL_CreateCond();
	
	timer_thread = SDL_CreateThread(timer_function, "timer_thread", NULL);
}

void timer_pause() {
	SDL_LockMutex(pause_lock);
	pause = true;
	SDL_UnlockMutex(pause_lock);
}

void timer_start() {
	SDL_LockMutex(pause_lock);
	pause = false;
	SDL_CondBroadcast(pause_cond);
	SDL_UnlockMutex(pause_lock);
}

void timer_reset() {
	timer_pause();

	SDL_LockMutex(timer_lock);
	timer_delay = 0;
	timer_sound = 0;
	SDL_UnlockMutex(timer_lock);

	timer_start();
}

void timer_terminate() {
	timer_start();

	SDL_LockMutex(running_lock);
	running = false;
	SDL_UnlockMutex(running_lock);

	SDL_WaitThread(timer_thread, NULL);

	SDL_DestroyMutex(timer_lock);
	SDL_DestroyMutex(running_lock);
	SDL_DestroyMutex(pause_lock);
	SDL_DestroyCond(pause_cond);
}
