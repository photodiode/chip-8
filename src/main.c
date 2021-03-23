
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>
#include <SDL2/SDL.h>

#include "mem.h"
#include "cpu.h"
#include "timer.h"
#include "sound.h"
#include "keypad.h"


int main(int argc, char *argv[]) {

	if (argc == 1) {
		printf("CHIP-8: Please specify a program to load (chip-8 <program>)\n");
		return EXIT_FAILURE;
	}

	mem_initialize();
	mem_load_program(argv[1]);

	// initialize SDL
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window*   window   = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*12, HEIGHT*12, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Surface*  surface  = SDL_CreateRGBSurfaceFrom(GFX, WIDTH, HEIGHT, 1, WIDTH/8, 0x0, 0x0, 0x0, 0x0);

	SDL_Color colors[2] = {{0, 0, 0, 255}, {255, 255, 255, 255}};
	SDL_SetPaletteColors(surface->format->palette, colors, 0, 2);
	// ----

	cpu_initialize();
	timer_initialize();
	sound_initialize();

	bool quit = false;
	SDL_Event event;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					quit = true;
					break;
				}
				case SDL_KEYDOWN: {
					if(event.key.keysym.sym == SDLK_r) {
						cpu_reset();
						timer_reset();
					}
					break;
				}
				default: break;
			}
			keypad_get(&event);
		}
		
		sound_update();

		SDL_RenderClear(renderer);

		pthread_mutex_lock(&mem_lock);
		SDL_Texture* framebuffer = SDL_CreateTextureFromSurface(renderer, surface);
		pthread_mutex_unlock(&mem_lock);

		SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
		SDL_DestroyTexture(framebuffer);

		SDL_RenderPresent(renderer);
	}

	cpu_terminate();
	timer_terminate();
	sound_terminate();

	// terminate SDL
	SDL_FreeSurface(surface);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	// ----

	return EXIT_SUCCESS;
}