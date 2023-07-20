#include "uthash.h"
#include "actors.h"
#include "worlds.h"

#ifndef FRAME_LOAD
#define FRAME_LOAD 1

typedef struct Frame {
  int name;
  int scroll_x;
  int scroll_y;
  SDL_Rect rect;
  World* world;
  int focus;
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
int in_frame(Frame* frame, Actor* actor);
void draw_frame(SDL_Renderer* rend, Frame* frame, int debug);
void update_frame(Frame* frame);
int has_frame(int worldKey);
int delete_frame(Frame* frame);
void scrolled(SDL_Rect* rect, Frame* frame);
void remove_actor_from_frames(int name);
#endif
