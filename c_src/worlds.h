# include "uthash.h"
# include <SDL2/SDL.h>

# ifndef WORLDS_DEF
#define WORLDS_DEF 1

typedef struct ActorEntry {
  char actorKey[32];
  struct ActorEntry* next;
  struct ActorEntry* prev;
} ActorEntry;

typedef struct World {
  char name[32];
  char background[32];
  ActorEntry* actors;
  int x_lock;
  int y_lock;
  int background_x_scroll;
  int background_y_scroll;
  int flagged_for_update;
  UT_hash_handle hh;
} World;

void add_world(const char* name,
	       char* background,
	       int x_lock,
	       int y_lock);
World* get_world(const char* name);
void add_actor_to_world(const char* worldkey, const char* actorname);
int update_world(char* worldKey, int debug);
void draw_world(World* world, SDL_Renderer* rend, const char* frameKey);
int exists(char* actorKey);
int world_has(World *world, char *actorKey);
# endif

