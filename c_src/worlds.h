# include "uthash.h"
# include <SDL2/SDL.h>

# ifndef WORLDS_DEF
#define WORLDS_DEF 1

typedef struct ActorEntry {
  int actorKey;
  struct ActorEntry* next;
  struct ActorEntry* prev;
} ActorEntry;

typedef struct World {
  int name;
  int background;
  ActorEntry* actors;
  int x_lock;
  int y_lock;
  int background_x_scroll;
  int background_y_scroll;
  int flagged_for_update;
  UT_hash_handle hh;
} World;

void add_world(int name,
	       int background,
	       int x_lock,
	       int y_lock);
World* get_world(int name);
void add_actor_to_world(int worldkey, int actorname);
int update_world(int worldKey, int debug);

# include "frames.h"
void draw_world(World* world, SDL_Renderer* rend, Frame* frame);
int exists(int actorKey);
int world_has(World *world, int actorKey);
void remove_actor_from_worlds(int actorKey);
void draw_debug_overlay(World* world, SDL_Renderer* rend, Frame* frame);

# endif

