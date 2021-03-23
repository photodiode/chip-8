
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>
//#include <SDL2/SDL.h>


uint8_t         timer_delay = 0;
uint8_t         timer_sound = 0;
pthread_mutex_t timer_lock  = PTHREAD_MUTEX_INITIALIZER;

static bool            running      = true;
static pthread_mutex_t running_lock = PTHREAD_MUTEX_INITIALIZER;

static bool            pause      = false;
static pthread_mutex_t pause_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  pause_cond = PTHREAD_COND_INITIALIZER;

static pthread_t timer_thread;

void* timer_function() {

	long delay = 1000000000L / 60L; // 60 Hz
	struct timespec time;
	long elapsed;

	while(1) {

		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec;


		pthread_mutex_lock(&timer_lock);
		if (timer_delay > 0) timer_delay--;
		if (timer_sound > 0) timer_sound--;
		pthread_mutex_unlock(&timer_lock);
		
		pthread_mutex_lock(&running_lock);
		if (!running) break;
		pthread_mutex_unlock(&running_lock);

		pthread_mutex_lock(&pause_lock);
		while(pause) pthread_cond_wait(&pause_cond, &pause_lock);
		pthread_mutex_unlock(&pause_lock);


		clock_gettime(CLOCK_REALTIME, &time);
		elapsed = time.tv_nsec - elapsed;

		nanosleep(&(struct timespec){
			.tv_sec  = 0,
			.tv_nsec = delay - elapsed
		}, NULL);
	}

	return NULL;
}

void timer_initialize() {
	pthread_create(&timer_thread, NULL, timer_function, NULL);
}

void timer_pause() {
	pthread_mutex_lock(&pause_lock);
	pause = true;
	pthread_mutex_unlock(&pause_lock);
}

void timer_start() {
	pthread_mutex_lock(&pause_lock);
	pause = false;
	pthread_cond_broadcast(&pause_cond);
	pthread_mutex_unlock(&pause_lock);
}

void timer_reset() {
	timer_pause();

	pthread_mutex_lock(&timer_lock);
	timer_delay = 0;
	timer_sound = 0;
	pthread_mutex_unlock(&timer_lock);

	timer_start();
}

void timer_terminate() {
	pthread_mutex_lock(&running_lock);
	running = false;
	pthread_mutex_unlock(&running_lock);

	pthread_join(timer_thread, NULL);
}
