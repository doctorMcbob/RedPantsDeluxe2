# include <SDL2/SDL.h>
# include "uthash.h"
# include "sprites.h"
# include "boxes.h"
# include "scriptdata.h"

#ifndef ACTORS_DEF
# define ACTORS_DEF 1

extern int DEEPEST_ACTOR;
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
  SDL_Rect ECB;

  float x_vel;
  float y_vel;
  int hurtboxkey;
  int hitboxkey;
  int spritemapkey;
  int scriptmap[LARGEST_SCRIPT_MAP];
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
  int background;
  // attribute hash
  Attribute* attributes;
} Actor;

void translate_rect_by_actor(Actor *actor, SDL_Rect *rect);
void validate_actors();
Actor* get_actor(int name);
void add_actor(
         int name,
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
void copy_actor(Actor *copy,  Actor *a);
Actor* add_actor_from_templatekey(int templateKey, int name);
Actor* get_template(int name);
void add_template_from_actorkey(int idx, int actorKey);
int update_actor(int actorKey, int worldKey, int debug);
Sprite* get_sprite_for_actor(Actor* actor);

# include "frames.h"
void draw_actor(SDL_Renderer* rend, Actor* actor, Frame* frame);
int get_script_for_actor(Actor* actor);
void actors_reset_updated();
int find_script_from_map(Actor* actor, int scriptName, int scriptFrame);
void pop_from_script_map(Actor* actor, int scriptName, int scriptFrame);
void free_actor(Actor* actor);
BoxMapEntry* get_hurtboxes_for_actor(Actor* actor);
BoxMapEntry* get_hitboxes_for_actor(Actor* actor);
int hit_check(Actor *self, Actor* related, World *world, int debug);
#endif
