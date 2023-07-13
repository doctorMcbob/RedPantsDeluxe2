/**
   The inputs module

   input states and key maps
   input states are an abstraction of one players controller.
   key maps are what the engine will use to manage the virtual controller

   KEY DOWN -> event.key is SDLK_UP
   for each KeyMap in our hash,
       is SDLK_UP KeyMap.A?
       is SDLK_UP KeyMap.B?
       is SDLK_UP KeyMap.X?
       is SDLK_UP KeyMap.Y?
       is SDLK_UP KeyMap.up?
          update controller with the same name InputState.UP = 1
          add UP_DOWN event to InputState.EVENTS

 */

#include "inputs.h"
#include "uthash.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define MAX_NUM_STICKS 4

InputHashNode *input_states = NULL;
KeyHashNode *key_maps = NULL;
SDL_GameController *CONTROLLERS[MAX_NUM_STICKS];
JoyMap JOYMAPS[MAX_NUM_STICKS];

void update_sticks() {
  int numControllers = SDL_NumJoysticks();
  for (int i = 0; i < numControllers && i < MAX_NUM_STICKS; i++) {
    if (!SDL_IsGameController(i)) {
      continue;
    }

    if (!SDL_GameControllerGetAttached(CONTROLLERS[i])) {
      CONTROLLERS[i] = SDL_GameControllerOpen(i);
      SDL_GameController *controller = CONTROLLERS[i];;

      // set defaults for buttons
      JOYMAPS[i].A = SDL_CONTROLLER_BUTTON_A;
      JOYMAPS[i].B = SDL_CONTROLLER_BUTTON_B;
      JOYMAPS[i].X = SDL_CONTROLLER_BUTTON_X;
      JOYMAPS[i].Y = SDL_CONTROLLER_BUTTON_Y;
      JOYMAPS[i].LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
      JOYMAPS[i].UP = SDL_CONTROLLER_BUTTON_DPAD_UP;
      JOYMAPS[i].RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
      JOYMAPS[i].DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
      JOYMAPS[i].START = SDL_CONTROLLER_BUTTON_START;
      JOYMAPS[i].HORIZ_STICK = 0;
      JOYMAPS[i].VERT_STICK = 1;

      if (controller == NULL) {
        printf("Error opening game controller %i %s\n", i, SDL_GetError());
        exit(-1);
      }
    }
  }
}

int get_num_sticks() {
  int i = 0;
  if (CONTROLLERS[0] != NULL)
    i++;
  if (CONTROLLERS[1] != NULL)
    i++;
  if (CONTROLLERS[2] != NULL)
    i++;
  if (CONTROLLERS[3] != NULL)
    i++;
  return i;
}

void add_stick_to_input_state(int name, int stick) {
  struct InputHashNode *is;
  HASH_FIND_INT(input_states, &name, is);
  if (is)
  {
    is->data->joy = stick;
  }
}

void add_input_state(int name, int stick) {
  struct InputHashNode *is;
  is = malloc(sizeof(InputHashNode));
  if (is == NULL)
  {
    printf("Error creating InputHashNode\n");
    exit(-1);
  }
  struct InputState *input_state;
  input_state = malloc(sizeof(InputState));
  input_state->A = 0;
  input_state->B = 0;
  input_state->X = 0;
  input_state->Y = 0;
  input_state->LEFT = 0;
  input_state->UP = 0;
  input_state->RIGHT = 0;
  input_state->DOWN = 0;
  input_state->START = 0;
  for (int i = 0; i < 18; i++)
  {
    input_state->EVENTS[i] = 0;
  }
  if (input_state == NULL)
  {
    printf("Error creating InputState\n");
    exit(-1);
  }
  input_state->joy = stick;
  if (stick == -1)
  {
    struct KeyHashNode *km;
    km = malloc(sizeof(KeyHashNode));
    if (km == NULL)
    {
      printf("Error creating KeyHashNode\n");
      exit(-1);
    }

    struct KeyMap *key_map;
    key_map = malloc(sizeof(KeyMap));
    if (key_map == NULL)
    {
      printf("Error creating KeyMap\n");
      exit(-1);
    }

    // set defaults
    key_map->A = SDLK_z;
    key_map->B = SDLK_x;
    key_map->X = SDLK_a;
    key_map->Y = SDLK_s;

    key_map->LEFT = SDLK_LEFT;
    key_map->UP = SDLK_UP;
    key_map->RIGHT = SDLK_RIGHT;
    key_map->DOWN = SDLK_DOWN;

    key_map->START = SDLK_RETURN;

    km->keymap = key_map;
    km->name = name;
    HASH_ADD_INT(key_maps, name, km);
  }
  is->name = name;
  is->data = input_state;

  HASH_ADD_INT(input_states, name, is);
}

InputState *get_input_state(int name) {
  struct InputHashNode *is;

  HASH_FIND_INT(input_states, &name, is);
  if (is)
  {
    return is->data;
  }
  else
  {
    return NULL;
  }
}

void _clear_events() {
  struct InputHashNode *is, *tmp;
  HASH_ITER(hh, input_states, is, tmp)
  {
    memset(is->data->EVENTS, 0, sizeof is->data->EVENTS);
  }
}

int input_update() {
  _clear_events();
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT: {
        return -1;
      }
      case SDL_KEYDOWN: {
        if (event.key.repeat != 0)
          continue;
        // remove this eventually
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
          return -1;
        }
        KeyHashNode *km, *tmp;
        HASH_ITER(hh, key_maps, km, tmp)
        {
          InputHashNode *inputNode;
          HASH_FIND_INT(input_states, &(km->name), inputNode);
          if (inputNode == NULL || inputNode->data == NULL)
          {
            printf("Unfortunately there was no input state with the name %i",
                  km->name);
            continue;
          }

          if (inputNode->data->joy != -1)
          {
            continue;
          }

          if (event.key.keysym.sym == km->keymap->A)
          {
            inputNode->data->A = 1;
            inputNode->data->EVENTS[A_DOWN] = 1;
          }

          if (event.key.keysym.sym == km->keymap->B)
          {
            inputNode->data->B = 1;
            inputNode->data->EVENTS[B_DOWN] = 1;
          }

          if (event.key.keysym.sym == km->keymap->X)
          {
            inputNode->data->X = 1;
            inputNode->data->EVENTS[X_DOWN] = 1;
          }

          if (event.key.keysym.sym == km->keymap->Y)
          {
            inputNode->data->Y = 1;
            inputNode->data->EVENTS[Y_DOWN] = 1;
          }

          if (event.key.keysym.sym == km->keymap->LEFT)
          {
            inputNode->data->LEFT = 1;
            inputNode->data->EVENTS[LEFT_DOWN] = 1;
          }
          if (event.key.keysym.sym == km->keymap->UP)
          {
            inputNode->data->UP = 1;
            inputNode->data->EVENTS[UP_DOWN] = 1;
          }
          if (event.key.keysym.sym == km->keymap->RIGHT)
          {
            inputNode->data->RIGHT = 1;
            inputNode->data->EVENTS[RIGHT_DOWN] = 1;
          }
          if (event.key.keysym.sym == km->keymap->DOWN)
          {
            inputNode->data->DOWN = 1;
            inputNode->data->EVENTS[DOWN_DOWN] = 1;
          }

          if (event.key.keysym.sym == km->keymap->START)
          {
            inputNode->data->START = 1;
            inputNode->data->EVENTS[START_DOWN] = 1;
          }
        }
        break;
      }
      case SDL_KEYUP: {
        KeyHashNode *km, *tmp;
        HASH_ITER(hh, key_maps, km, tmp)
        {
          InputHashNode *inputNode;
          HASH_FIND_INT(input_states, &km->name, inputNode);
          if (!inputNode)
          {
            printf("Unfortunately there was no input state with the name %i",
                  km->name);
            continue;
          }

          if (inputNode->data->joy != -1)
          {
            continue;
          }

          if (event.key.keysym.sym == km->keymap->A)
          {
            inputNode->data->A = 0;
            inputNode->data->EVENTS[A_UP] = 1;
          }

          if (event.key.keysym.sym == km->keymap->B)
          {
            inputNode->data->B = 0;
            inputNode->data->EVENTS[B_UP] = 1;
          }

          if (event.key.keysym.sym == km->keymap->X)
          {
            inputNode->data->X = 0;
            inputNode->data->EVENTS[X_UP] = 1;
          }

          if (event.key.keysym.sym == km->keymap->Y)
          {
            inputNode->data->Y = 0;
            inputNode->data->EVENTS[Y_UP] = 1;
          }

          if (event.key.keysym.sym == km->keymap->LEFT)
          {
            inputNode->data->LEFT = 0;
            inputNode->data->EVENTS[LEFT_UP] = 1;
          }
          if (event.key.keysym.sym == km->keymap->UP)
          {
            inputNode->data->UP = 0;
            inputNode->data->EVENTS[UP_UP] = 1;
          }
          if (event.key.keysym.sym == km->keymap->RIGHT)
          {
            inputNode->data->RIGHT = 0;
            inputNode->data->EVENTS[RIGHT_UP] = 1;
          }
          if (event.key.keysym.sym == km->keymap->DOWN)
          {
            inputNode->data->DOWN = 0;
            inputNode->data->EVENTS[DOWN_UP] = 1;
          }

          if (event.key.keysym.sym == km->keymap->START)
          {
            inputNode->data->START = 0;
            inputNode->data->EVENTS[START_UP] = 1;
          }
        }
      }
      case SDL_CONTROLLERBUTTONDOWN: {
        InputHashNode *inputNode, *tmp;
        HASH_ITER(hh, input_states, inputNode, tmp) {
          if (inputNode->data->joy == event.cbutton.which) {
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].A) {
              inputNode->data->A = 1;
              inputNode->data->EVENTS[A_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].B) {
              inputNode->data->B = 1;
              inputNode->data->EVENTS[B_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].X) {
              inputNode->data->X = 1;
              inputNode->data->EVENTS[X_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].Y) {
              inputNode->data->Y = 1;
              inputNode->data->EVENTS[Y_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].START) {
              inputNode->data->START = 1;
              inputNode->data->EVENTS[START_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].LEFT) {
              inputNode->data->LEFT = 1;
              inputNode->data->EVENTS[LEFT_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].UP) {
              inputNode->data->UP = 1;
              inputNode->data->EVENTS[UP_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].RIGHT) {
              inputNode->data->RIGHT = 1;
              inputNode->data->EVENTS[RIGHT_DOWN] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].DOWN) {
              inputNode->data->DOWN = 1;
              inputNode->data->EVENTS[DOWN_DOWN] = 1;
            }
            break;
          }
        }
        break;
      }
      case SDL_CONTROLLERBUTTONUP: {
        InputHashNode *inputNode, *tmp;
        HASH_ITER(hh, input_states, inputNode, tmp) {
          if (inputNode->data->joy == event.cbutton.which) {
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].A) {
              inputNode->data->A = 0;
              inputNode->data->EVENTS[A_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].B) {
              inputNode->data->B = 0;
              inputNode->data->EVENTS[B_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].X) {
              inputNode->data->X = 0;
              inputNode->data->EVENTS[X_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].Y) {
              inputNode->data->Y = 0;
              inputNode->data->EVENTS[Y_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].START) {
              inputNode->data->START = 0;
              inputNode->data->EVENTS[START_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].LEFT) {
              inputNode->data->LEFT = 0;
              inputNode->data->EVENTS[LEFT_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].UP) {
              inputNode->data->UP = 0;
              inputNode->data->EVENTS[UP_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].RIGHT) {
              inputNode->data->RIGHT = 0;
              inputNode->data->EVENTS[RIGHT_UP] = 1;
            }
            if (event.cbutton.button == JOYMAPS[inputNode->data->joy].DOWN) {
              inputNode->data->DOWN = 0;
              inputNode->data->EVENTS[DOWN_UP] = 1;
            }
            break;
          }
        }
        break;
      }
      case SDL_CONTROLLERAXISMOTION: {
        InputHashNode *inputNode, *tmp;
        HASH_ITER(hh, input_states, inputNode, tmp) {
          if (inputNode->data->joy == event.caxis.which) {
            if (event.caxis.axis == JOYMAPS[inputNode->data->joy].HORIZ_STICK) { 
              if (event.caxis.value > 6400) {
                if (inputNode->data->RIGHT == 0) {
                  inputNode->data->EVENTS[RIGHT_DOWN] = 1;
                }
                inputNode->data->RIGHT = 1;
              } else {
                if (inputNode->data->RIGHT == 1) {
                  inputNode->data->EVENTS[RIGHT_UP] = 1;
                }
                inputNode->data->RIGHT = 0;
              }
              if (event.caxis.value < -6400) {
                if (inputNode->data->LEFT == 0) {
                  inputNode->data->EVENTS[LEFT_DOWN] = 1;
                }
                inputNode->data->LEFT = 1;
              } else {
                if (inputNode->data->LEFT == 1) {
                  inputNode->data->EVENTS[LEFT_UP] = 1;
                }
                inputNode->data->LEFT = 0;
              }
              break;
            }
            if (event.caxis.axis == JOYMAPS[inputNode->data->joy].VERT_STICK) { 
              if (event.caxis.value > 6400) {
                if (inputNode->data->DOWN == 0) {
                  inputNode->data->EVENTS[DOWN_DOWN] = 1;
                }
                inputNode->data->DOWN = 1;
              } else {
                if (inputNode->data->DOWN == 1) {
                  inputNode->data->EVENTS[DOWN_UP] = 1;
                }
                inputNode->data->DOWN = 0;
              }
              if (event.caxis.value < -6400) {
                if (inputNode->data->UP == 0) {
                  inputNode->data->EVENTS[UP_DOWN] = 1;
                }
                inputNode->data->UP = 1;
              } else {
                if (inputNode->data->UP == 1) {
                  inputNode->data->EVENTS[UP_UP] = 1;
                }
                inputNode->data->UP = 0;
              }
              break;
            }
          }
        }
        break;
      }
    }
  }
  return 1;
}
