#include "uthash.h"
#include "actors.h"
#include "worlds.h"

#ifndef FRAME_LOAD
#define FRAME_LOAD 1

typedef struct Frame {
  char name[32];
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

void add_frame(const char* name, World* world, Actor* focus, int x, int y, int w, int h);
Frame* get_frame(const char* name);
int in_frame(const char* frameKey, Actor* actor);
void draw_frame(SDL_Renderer* rend, const char* name);
void update_frame(const char* frameKey);
int has_frame(char* worldKey);
#endif
