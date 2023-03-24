# include <SDL2/SDL.h>
# include "uthash.h"
# include "sprites.h"
# include "scripts.h"

#ifndef ACTORS_DEF
# define ACTORS_DEF 1

typedef struct Attribute {
  char name[32];
  SyntaxNode* value;
  UT_hash_handle hh;
} Attribute;

typedef struct Actor {
  char name[32];
  SDL_Rect* ECB;

  float x_vel;
  float y_vel;
  char hurtboxkey[32];
  char hitboxkey[32];
  char spritemapkey[32];
  char scriptmapkey[32];
  char *img;
  char _input_name[32];
  char state[32];
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

Actor* get_actor(const char* name);
void add_actor(const char* name,
	       int x,
	       int y,
	       int w,
	       int h,
	       int x_vel,
	       int y_vel,
	       char* hurtboxkey,
	       char* hitboxkey,
	       char* scriptMapKey,
	       char* spriteMapKey,
	       char* img,
	       char* inputKey,
	       char* state,
	       int frame,
	       int direction,
	       int rotation,
	       int platform,
	       int tangible,
	       int physics,
	       int updated);
void copy_actor(Actor* copy,  Actor *a);
void add_template(Actor* copy);
void add_actor_from_templatekey(char* templateKey);
void add_template_from_actorkey(char* actorKey);
int update_actor(char* actorKey, char* worldKey, int debug);
Sprite* get_sprite_for_actor(Actor* actor);
void draw_actor(SDL_Renderer* rend, Actor* actor, const char* frameKey);
int get_script_for_actor(Actor* actor);
void actors_reset_updated();
int find_script_from_map(Actor* actor, char* scriptName);
#endif
