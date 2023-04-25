#include "uthash.h"
#include "actors.h"
#include "worlds.h"

#ifndef FRAME_LOAD
#define FRAME_LOAD 1

typedef struct Frame {
  int name;
  int scroll_x;
  int scroll_y;
  SDL_Rect* rect;
  World* world;
  Actor* focus;
  int bound_left;
  int left_bind;
  int bound_top;
  int top_bind;
  int bound_right;
  int right_bind;
  int bound_bottom;
  int bottom_bind;
  int active;
  UT_hash_handle hh;
} Frame;

void add_frame(int name, World* world, Actor* focus, int x, int y, int w, int h);
Frame* get_frame(int name);
int in_frame(int frameKey, Actor* actor);
void draw_frame(SDL_Renderer* rend, int name, int debug);
void update_frame(int frameKey);
int has_frame(int worldKey);
#endif
