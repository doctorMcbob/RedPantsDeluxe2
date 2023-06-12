

# include <SDL2/SDL.h>

# ifndef WORLDS_DEF
#define WORLDS_DEF 1
# include "worlddata.h"

typedef struct World {
  int name;
  int background;
  int actors[WORLD_BUFFER_SIZE];
  int x_lock;
  int y_lock;
  int background_x_scroll;
  int background_y_scroll;
  int flagged_for_update;
} World;
extern World WORLDS[];
void add_world(int key,
         int name,
	       int background,
	       int x_lock,
	       int y_lock);
World* get_world(int name);
void add_actor_to_world(int worldkey, int actorname);
int update_world(int worldKey, int debug);

# include "frames.h"
World* world_with(int actorKey);
void draw_world(World* world, SDL_Renderer* rend, Frame* frame);
int exists(int actorKey);
int world_has(World *world, int actorKey);
void remove_actor_from_worlds(int actorKey);
void draw_debug_overlay(World* world, SDL_Renderer* rend, Frame* frame);
int remove_actor_from_world(World* world, int actorKey);
# endif

