
#include <stdint.h>

#include <pthread.h>
#include <SDL2/SDL.h>

#include "timer.h"

void sound_callback(void* userdata, uint8_t* stream, int len) {

	static uint16_t t = 0;
	
	(void)userdata;
	
	for(int64_t i = 0; i < len; i++) {

 		// make a triangle wave of roughly a C5
		int16_t  tri  = ((int16_t)(t%16) - 8) * 32;
		uint16_t tmp  = tri >> 15;
		         tri ^= tmp;
		         tri += tmp & 1;
		t++;
		// ----

		stream[i] = 127 | tri;
	}
}

void sound_initialize() {

	SDL_OpenAudio(&(SDL_AudioSpec){
		.freq     = 8192,
		.format   = AUDIO_U8,
	        .channels = 1,
	        .samples  = 256,
		.callback = sound_callback
	}, NULL);
}

void sound_update() {

	pthread_mutex_lock(&timer_lock);
	if (timer_sound > 0) {
		SDL_PauseAudio(0);
	} else {
		SDL_PauseAudio(1);
	}
	pthread_mutex_unlock(&timer_lock);
}

void sound_terminate() {
	SDL_CloseAudio();
}
