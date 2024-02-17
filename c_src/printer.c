#include "printer.h"
#include <SDL2/SDL.h>
#include "gif_lib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
   The printer

   The printer has the following components
   - a frame counter
   - a counter of how many surfaces are stored
   - a max size (should FPS * gif length in seconds)
   - an array of Surfaces the length of max size
     - the Surfaces should be the size of the screen
     ? maybe this will include an init function that allocates them?
   - a save function that writes a surfece to the next surface to the index frame
     - when the index frame has reached the max size, it will reset to 0
     - when writing a gif (thats the purpose here by the way) the frames will start at
       the frame counter and then "wrap around" so thats everything after frame and then 
       everything before frame.
   - a clear function that

   ---- Notes ----
   This could be extended to include a dynamic replay size.
   This could also be extended to hold n replays
   This could be extended to write the output of a replay to the screen in a frame
        or even to an actor

   We could write the TAS system, and save the full replay in memory
   and have a show TAS available at the end of the game
   your "Reward for winning" could be credits -> watch the TAS button on the main menu

   I could TAS and re-record cutscenes o-o 0-0 @-@ <3
*/

int FRAME = 0;
int TOTAL = 0;
SDL_Surface* replay[MAX_SIZE];

void init() {
  for (int i = 0; i < MAX_SIZE; i++) {
    replay[i] = SDL_CreateRGBSurface(0, WID, HIGH, 32, 0, 0, 0, 0);
    if (replay[i] == NULL) {
      fprintf(stderr, "Could not allocate surface: %s\n", SDL_GetError());
      exit(1);
    }
  }  
}
// .. I will write the code to populate the Surfaces with game footage

// write a function that writes the array to "replay.gif" using gif_lib.h
