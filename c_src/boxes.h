# include "uthash.h"
# include "utlist.h"
# include <SDL2/SDL.h>

#ifndef BOX_DEF
#define BOX_DEF 1

typedef struct BoxMapEntry {
  char state[32];
  int frame;
  struct BoxMapEntry* next;
  struct BoxMapEntry* prev;
  SDL_Rect rect[];
} BoxMapEntry;

typedef struct BoxMap {
  char name[32];
  struct BoxMapEntry* entries;
  UT_hash_handle hh;
} BoxMap;

void add_hitbox_map(const char* name);
void add_to_hitbox_map(const char* name,
		       const char* state,
		       int frame,
		       int x[],
		       int y[],
		       int w[],
		       int h[],
		       int count);

BoxMap* get_hitbox_map(const char* name);

void add_hurtbox_map(const char* name);
void add_to_hurtbox_map(const char* name,
			const char* state,
			int frame,
			int x[],
			int y[],
			int w[],
			int h[],
			int count);

BoxMap* get_hurtbox_map(const char* name);
#endif
