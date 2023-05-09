# include "uthash.h"
# include "utlist.h"
# include <SDL2/SDL.h>

#ifndef BOX_DEF
#define BOX_DEF 1

typedef struct BoxMapEntry {
  int state;
  int frame;
  struct BoxMapEntry* next;
  struct BoxMapEntry* prev;
  int count;
  SDL_Rect rect[];
} BoxMapEntry;

typedef struct BoxMap {
  int name;
  struct BoxMapEntry* entries;
  UT_hash_handle hh;
} BoxMap;

void add_hitbox_map(int name);
void add_to_hitbox_map(int name,
		       int state,
		       int frame,
		       int x[],
		       int y[],
		       int w[],
		       int h[],
		       int count);

BoxMap* get_hitbox_map(int name);

void add_hurtbox_map(int name);
void add_to_hurtbox_map(int name,
			int state,
			int frame,
			int x[],
			int y[],
			int w[],
			int h[],
			int count);

BoxMap* get_hurtbox_map(int name);
#endif
