#include "clock.h"
#include <SDL2/SDL.h>

void Clock_tick(Clock *clock, Uint32 fps) {
  clock->lastTicks = clock->startTicks;
  clock->startTicks = SDL_GetTicks();
  clock->frameTicks = clock->startTicks - clock->lastTicks;

  int targetFrameTicks = 1000 / fps;

  if (targetFrameTicks > clock->frameTicks) {
    int delay = targetFrameTicks - clock->frameTicks;
    SDL_Delay(delay);
    clock->frameTicks = targetFrameTicks;
  }

  clock->fps = 1000 / clock->frameTicks;
}

Uint32 Clock_get_fps(Clock *clock) { return clock->fps; }

Clock *new_clock() {
  Clock *c = malloc(sizeof(Clock));
  c->startTicks = 0;
  c->lastTicks = 0;
  c->frameTicks = 0;
  c->fps = 0;
  return c;
}