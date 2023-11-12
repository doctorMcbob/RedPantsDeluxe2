#include <SDL2/SDL.h>
#include "benchmarks.h"

int STRING_MACHINE_TIME = 0;
int DRAWING_TIME = 0;
int COLLISION_TIME = 0;
int INTERPERETER_TIME = 0;
int MODE = -1;

int last_time = 0;

void switch_benchmark_mode(int mode) {
  int current_time = SDL_GetTicks();
  int delta = current_time - last_time;
  switch (MODE) {
  case STRING_MACHINE_MODE:
    STRING_MACHINE_TIME += delta;
    break;
  case DRAWING_MODE:
    DRAWING_TIME += delta;
    break;
  case COLLISION_MODE:
    COLLISION_TIME += delta;
    break;
  case INTERPERETER_MODE:
    INTERPERETER_TIME += delta;
    break;
  }
  
  MODE = mode;
  last_time = current_time;
}

int get_benchmark_mode() {
  return MODE;
}

void print_benchmark_data() {
  int total_time = STRING_MACHINE_TIME + DRAWING_TIME + COLLISION_TIME + INTERPERETER_TIME;

  printf("Time elapsed: %i\n", total_time);
  printf("String Machine Time:\n  %f percent (%i)\n",
         ((float)STRING_MACHINE_TIME / total_time) * 100, STRING_MACHINE_TIME);
  printf("Drawing Time:\n  %f percent (%i)\n",
         ((float)DRAWING_TIME / total_time) * 100, DRAWING_TIME);
  printf("Collision Time:\n  %f percent (%i)\n",
         ((float)COLLISION_TIME / total_time) * 100, COLLISION_TIME);
  printf("Interpreter Time:\n  %f percent (%i)\n",
         ((float)INTERPERETER_TIME / total_time) * 100, INTERPERETER_TIME);
}
