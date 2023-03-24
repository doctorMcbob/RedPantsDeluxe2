
# include "uthash.h"

# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>

#ifndef INPUTS_DEF
#define INPUTS_DEF 1


// Key events. index values for EVENTS array.
# define A_DOWN 0
# define A_UP 1
# define B_DOWN 2
# define B_UP 3
# define X_DOWN 4
# define X_UP 5
# define Y_DOWN 6
# define Y_UP 7
# define LEFT_DOWN 8
# define LEFT_UP 9
# define UP_DOWN 10
# define UP_UP 11
# define RIGHT_DOWN 12
# define RIGHT_UP 13
# define DOWN_DOWN 14
# define DOWN_UP 15
# define START_DOWN 16
# define START_UP 17


typedef struct InputState {
  SDL_Joystick* joy;
  unsigned char A;
  unsigned char B;
  unsigned char X;
  unsigned char Y;
  unsigned char LEFT;
  unsigned char UP;
  unsigned char RIGHT;
  unsigned char DOWN;
  unsigned char START;
  unsigned char EVENTS[18];
} InputState; 

typedef struct InputHashNode {
  char name[32];
  InputState* data;
  UT_hash_handle hh;
} InputHashNode;

typedef struct KeyMap {
  SDL_Keycode A;
  SDL_Keycode B;
  SDL_Keycode X;
  SDL_Keycode Y;
  SDL_Keycode LEFT;
  SDL_Keycode UP;
  SDL_Keycode RIGHT;
  SDL_Keycode DOWN;
  SDL_Keycode START;
} KeyMap;

typedef struct KeyHashNode {
  char name[32];
  KeyMap* keymap;
  UT_hash_handle hh;
} KeyHashNode;

void add_input_state(const char* name, SDL_Joystick* joy);
InputState* get_input_state(const char* name);
int input_update();
#endif
