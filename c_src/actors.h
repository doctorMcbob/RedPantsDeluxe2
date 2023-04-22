# include <SDL2/SDL.h>
# include "uthash.h"
# include "sprites.h"

#ifndef ACTORS_DEF
# define ACTORS_DEF 1

typedef struct Attribute {
  int name;
  int type;
  union {
    int i;
    float f;
  } value;
  UT_hash_handle hh;
} Attribute;

typedef struct Actor {
  int name;
  SDL_Rect* ECB;

  float x_vel;
  float y_vel;
  int hurtboxkey;
  int hitboxkey;
  int spritemapkey;
  int scriptmapkey;
  int img;
  int _input_name;
  int state;
  int frame;
  int direction;
  int rotation;
  // flags
  int platform;
  int tangible;
  int physics;
  int updated;
  // attribute hash
  Attribute* attributes;
  UT_hash_handle hh;
} Actor;

Actor* get_actor(int name);
void add_actor(int name,
	       int x,
	       int y,
	       int w,
	       int h,
	       int x_vel,
	       int y_vel,
	       int hurtboxkey,
	       int hitboxkey,
	       int scriptMapKey,
	       int spritemapkey,
	       int img,
	       int inputKey,
	       int state,
	       int frame,
	       int direction,
	       int rotation,
	       int platform,
	       int tangible,
	       int physics,
	       int updated);
void copy_actor(Actor* copy,  Actor *a);
void add_template(Actor* copy);
Actor* add_actor_from_templatekey(int templateKey, int name);
void add_template_from_actorkey(int actorKey);
int update_actor(int actorKey, int worldKey, int debug);
Sprite* get_sprite_for_actor(Actor* actor);
void draw_actor(SDL_Renderer* rend, Actor* actor, int frameKey);
int get_script_for_actor(Actor* actor);
void actors_reset_updated();
int find_script_from_map(Actor* actor, int scriptName, int scriptFrame);
void free_actor(Actor* actor);
#endif
