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

# include "uthash.h"
# include "inputs.h"
# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>

InputHashNode* input_states = NULL;
KeyHashNode* key_maps = NULL;

void add_input_state(const char* name, SDL_Joystick* joy) {
  struct InputHashNode *is;
  is = malloc(sizeof(InputHashNode));
  if (is == NULL) {
    printf("Error creating InputHashNode\n");
    exit(-1);
  }
  struct InputState *input_state;
  input_state = malloc(sizeof(InputState));
  if (input_state == NULL) {
    printf("Error creating InputState\n");
    exit(-1);
  }
  input_state->joy = joy;
  if (joy == NULL) {
    struct KeyHashNode *km;
    km = malloc(sizeof(KeyHashNode));
    if (km == NULL) {
      printf("Error creating KeyHashNode\n");
      exit(-1);
    }
    
    struct KeyMap *key_map;
    key_map = malloc(sizeof(KeyMap));
    if (key_map == NULL) {
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
    strcpy(km->name, name);
    HASH_ADD_STR(key_maps, name, km);
  }
  strcpy(is->name, name);
  is->data = input_state;
  
  HASH_ADD_STR(input_states, name, is);
}

InputState* get_input_state(const char* name) {
  struct InputHashNode *is;

  HASH_FIND_STR(input_states, name, is);
  if (is) {
    return is->data;
  } else {
    return NULL;
  }
}

void _clear_events() {
  struct InputHashNode *is, *tmp;
  HASH_ITER(hh, input_states, is, tmp) {
    memset(is->data->EVENTS, 0, sizeof is->data->EVENTS);
  }
}

int input_update() {
  _clear_events();
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      return -1;
   case SDL_KEYDOWN:
      {
	if (event.key.keysym.sym == SDLK_ESCAPE) {
	  return -1;
	}
	KeyHashNode *km, *tmp;
	HASH_ITER(hh, key_maps, km, tmp) {
	  InputHashNode* inputNode;
	  HASH_FIND_STR(input_states, km->name, inputNode);
	  if (!inputNode) {
	    printf("Unfortunately there was no input state with the name %s", km->name);
	    continue;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->A) {
	    inputNode->data->A = 1;
	    inputNode->data->EVENTS[A_DOWN] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->B) {
	    inputNode->data->B = 1;
	    inputNode->data->EVENTS[B_DOWN] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->X) {
	    inputNode->data->X = 1;
	    inputNode->data->EVENTS[X_DOWN] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->Y) {
	    inputNode->data->Y = 1;
	    inputNode->data->EVENTS[Y_DOWN] = 1;
	  }

	  
	  if (event.key.keysym.sym == km->keymap->LEFT) {
	    inputNode->data->LEFT = 1;
	    inputNode->data->EVENTS[LEFT_DOWN] = 1;
	  }
	  if (event.key.keysym.sym == km->keymap->UP) {
	    inputNode->data->UP = 1;
	    inputNode->data->EVENTS[UP_DOWN] = 1;
	  }
	  if (event.key.keysym.sym == km->keymap->RIGHT) {
	    inputNode->data->RIGHT = 1;
	    inputNode->data->EVENTS[RIGHT_DOWN] = 1;
	  }
	  if (event.key.keysym.sym == km->keymap->DOWN) {
	    inputNode->data->DOWN = 1;
	    inputNode->data->EVENTS[DOWN_DOWN] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->START) {
	    inputNode->data->START = 1;
	    inputNode->data->EVENTS[START_DOWN] = 1;
	  }
	}
	break;
      }
    case SDL_KEYUP:
      {
	KeyHashNode *km, *tmp;
	HASH_ITER(hh, key_maps, km, tmp) {
	  InputHashNode* inputNode;
	  HASH_FIND_STR(input_states, km->name, inputNode);
	  if (!inputNode) {
	    printf("Unfortunately there was no input state with the name %s", km->name);
	    continue;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->A) {
	    inputNode->data->A = 0;
	    inputNode->data->EVENTS[A_UP] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->B) {
	    inputNode->data->B = 0;
	    inputNode->data->EVENTS[B_UP] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->X) {
	    inputNode->data->X = 0;
	    inputNode->data->EVENTS[X_UP] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->Y) {
	    inputNode->data->Y = 0;
	    inputNode->data->EVENTS[Y_UP] = 1;
	  }

	  
	  if (event.key.keysym.sym == km->keymap->LEFT) {
	    inputNode->data->LEFT = 0;
	    inputNode->data->EVENTS[LEFT_UP] = 1;
	  }
	  if (event.key.keysym.sym == km->keymap->UP) {
	    inputNode->data->UP = 0;
	    inputNode->data->EVENTS[UP_UP] = 1;
	  }
	  if (event.key.keysym.sym == km->keymap->RIGHT) {
	    inputNode->data->RIGHT = 0;
	    inputNode->data->EVENTS[RIGHT_UP] = 1;
	  }
	  if (event.key.keysym.sym == km->keymap->DOWN) {
	    inputNode->data->DOWN = 0;
	    inputNode->data->EVENTS[DOWN_UP] = 1;
	  }
	  
	  if (event.key.keysym.sym == km->keymap->START) {
	    inputNode->data->START = 0;
	    inputNode->data->EVENTS[START_UP] = 1;
	  }
	}
      }
    }
  }
  return 1;
}

