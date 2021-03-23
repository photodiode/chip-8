
#include <stdint.h>
#include <stdbool.h>

#include <pthread.h>
#include <SDL2/SDL.h>


bool            keypad[16];
uint8_t         keypad_pressed;
pthread_mutex_t keypad_lock = PTHREAD_MUTEX_INITIALIZER;

static SDL_Keycode keycodes[] = {
	SDLK_0, SDLK_1, SDLK_2, SDLK_3,
	SDLK_4, SDLK_5, SDLK_6, SDLK_7,
	SDLK_8, SDLK_9, SDLK_a, SDLK_b,
	SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

void keypad_get(SDL_Event* event) {

	pthread_mutex_lock(&keypad_lock);

	keypad_pressed = 0;

	switch (event->type) {
		case SDL_KEYDOWN: {
			for (uint8_t i = 0; i < 16; i++) {
				if(event->key.keysym.sym == keycodes[i]) {
					if (!keypad[i]) keypad_pressed = i;
					keypad[i] = true;
				}
			}
			break;
		}
		case SDL_KEYUP: {
			for (uint8_t i = 0; i < 16; i++) {
				if(event->key.keysym.sym == keycodes[i]) {
					keypad[i] = false;
				}
			}
			break;
		}
		default: break;
	}

	pthread_mutex_unlock(&keypad_lock);
}
