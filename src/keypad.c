
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>


bool    keypad[16];
bool    keypad_pressed;
uint8_t keypad_pressed_key;

SDL_mutex* keypad_lock;
SDL_mutex* keypad_cond_lock;
SDL_cond*  keypad_cond;

static SDL_Keycode keycodes[] = {
	SDLK_0, SDLK_1, SDLK_2, SDLK_3,
	SDLK_4, SDLK_5, SDLK_6, SDLK_7,
	SDLK_8, SDLK_9, SDLK_a, SDLK_b,
	SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

void keypad_initialize() {
	keypad_lock      = SDL_CreateMutex();
	keypad_cond_lock = SDL_CreateMutex();
	keypad_cond      = SDL_CreateCond();
}

void keypad_get(SDL_Event* event) {

	SDL_LockMutex(keypad_lock);

	keypad_pressed = false;

	switch (event->type) {
		case SDL_KEYDOWN: {
			for (uint8_t i = 0; i < 16; i++) {
				if(event->key.keysym.sym == keycodes[i]) {
					if (!keypad[i]) {

						SDL_LockMutex(keypad_lock);
						keypad_pressed_key = i;
						SDL_UnlockMutex(keypad_lock);
						
						SDL_LockMutex(keypad_cond_lock);
						keypad_pressed = true;
						SDL_CondBroadcast(keypad_cond);
						SDL_UnlockMutex(keypad_cond_lock);
					}
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

	SDL_UnlockMutex(keypad_lock);
}

void keypad_terminate() {
	SDL_DestroyMutex(keypad_lock);
	SDL_DestroyMutex(keypad_cond_lock);
	SDL_DestroyCond(keypad_cond);
}
