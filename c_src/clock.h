#include <SDL2/SDL.h>
#ifndef _CLOCK_H_
#define _CLOCK_H_

typedef struct Clock {
    Uint32 startTicks;
    Uint32 lastTicks;
    Uint32 frameTicks;
    Uint32 fps;
} Clock;

void Clock_tick(Clock *clock, Uint32 fps);
Uint32 Clock_get_fps(Clock *clock);
Clock* new_clock();
#endif