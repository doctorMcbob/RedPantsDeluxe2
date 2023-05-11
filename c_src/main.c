/**
   The Red Pants Game Engine

Okay so I'm gonna level with you, as I write this I am very new to C.
I have a lot of the core functionality of SDL understood through the lense of Pygame.
I am leaning heavily on this video: https://www.youtube.com/watch?v=yFLa3ln16w0

I am starting off with the basics, trying to implement this method i wrote and have been using for a long time in pygame.

this is going to need a lot,
I will be using uthash.h as my dictionary implementation

 */
# include "inputs.h"
# include "sprites.h"
# include "scripts.h"
# include "worlds.h"
# include "frames.h"
# include "stringmachine.h"
# include "stringdata.h"
# include "clock.h"
# include <string.h>
# include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_ttf.h>

# include <SDL2/SDL.h>
# include <SDL2/SDL_timer.h>
# include <SDL2/SDL_image.h>

# define WID 1152
# define HIGH 640


TTF_Font* font;
void spritesheet_load(SDL_Renderer* rend);
void actor_load();
void world_load();
void boxes_load();
void scripts_load();
void load_string_indexers();
extern Frame* frames;
extern World* worlds;

int main (int argc, char *argv[]) {
  int debug = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      debug++;
    }
  }

  if (TTF_Init() == -1) {
    return 1;
  }
  font = TTF_OpenFont("/usr/share/fonts/truetype/tlwg/Waree-Bold.ttf", 16);


  srand(time(NULL));

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Error initializing SDL2: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window* screen = SDL_CreateWindow("Long way to the top, if you wanna make a game engine",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					WID,
					HIGH,
					0);
  if (!screen) {
    printf("error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  SDL_Renderer* rend = SDL_CreateRenderer(screen, -1, render_flags);
  
  if (!rend) {
    printf("error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(screen);
    SDL_Quit();
    return 1;
  }
  load_string_indexers();
  spritesheet_load(rend);
  actor_load();
  world_load();
  boxes_load();
  scripts_load();
  add_input_state(index_string("PLAYER1"), NULL);
  add_frame(ROOT, get_world(_ROOT), NULL, 0, 0, W, H);
  Clock* c = new_clock();

  while (input_update() != -1) {
    SDL_RenderClear(rend);

    Frame *f, *tmpf;
    HASH_ITER(hh, frames, f, tmpf) {
      if (f->active && f->world != NULL) {
        f->world->flagged_for_update = 1;
      }
    }

    World *w, *tmpw;
    HASH_ITER(hh, worlds, w, tmpw) {
      if (w->flagged_for_update) {
        if (update_world(w->name, debug) == -2) return 0;
      }
      w->flagged_for_update = 0;
    }
    actors_reset_updated();

    HASH_ITER(hh, frames, f, tmpf) {
      if (f->active) {
        update_frame(f);
        draw_frame(rend, f, debug);
      }
    }

    SDL_RenderPresent(rend);
    Clock_tick(c, 20);
  }

  sprites_taredown();
  
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(screen);
  SDL_Quit();
}

