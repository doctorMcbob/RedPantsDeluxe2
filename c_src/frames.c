#include "uthash.h"
#include "worlds.h"
#include "actors.h"
#include "frames.h"
#include "stringmachine.h"
# include <SDL2/SDL.h>

Frame* frames = NULL;

void add_frame(int name, World* world, Actor* focus, int x, int y, int w, int h) {
  struct Frame *f;
  f = malloc(sizeof(Frame));
  if (f == NULL) {
    exit(-1);
  }
  f->name = name;

  f->rect.x = x;
  f->rect.y = y;
  f->rect.w = w;
  f->rect.h = h;

  f->world = world;
  f->focus = focus;

  f->bound_bottom = 0;
  f->bound_left = 0;
  f->bound_right = 0;
  f->bound_top = 0;

  f->scroll_x = 0;
  f->scroll_y = 0;

  f->active = 1;
  HASH_ADD_INT(frames, name, f);
}

Frame* get_frame(int name){
  struct Frame* f;
  HASH_FIND_INT(frames, &name, f);
  if (f) {
    return f;
  } else {
    return NULL;
  }
}
void scrolled(SDL_Rect* rect, Frame* frame) {
  rect->x -= frame->scroll_x;
  rect->y -= frame->scroll_y;
}

int in_frame(Frame* frame, Actor* actor) {
  SDL_Rect *r = malloc(sizeof(SDL_Rect));
  r->x = actor->ECB->x;
  r->y = actor->ECB->y;
  r->w = actor->ECB->w;
  r->h = actor->ECB->h;
  scrolled(r, frame);

  int i = SDL_HasIntersection(&frame->rect, r);
  free(r);
  if (i)
    return 1;
  else
    return 0;
}

void draw_frame(SDL_Renderer* rend, Frame* f, int debug) {
  Uint32 format;
  SDL_Texture* render_target = SDL_GetRenderTarget(rend);
  SDL_QueryTexture(render_target, &format, NULL, NULL, NULL);
  SDL_Texture* frame_buffer = SDL_CreateTexture(
    rend, format, SDL_TEXTUREACCESS_TARGET, f->rect.w, f->rect.h);

  SDL_SetRenderTarget(rend, frame_buffer);

  draw_world(f->world, rend, f);
  if (debug) {
    draw_debug_overlay(f->world, rend, f);
  }

  SDL_SetRenderTarget(rend, render_target);

  SDL_Rect src = {0, 0, f->rect.w, f->rect.h};
  SDL_RenderCopy(rend, frame_buffer, &src, &f->rect);
  
  SDL_DestroyTexture(frame_buffer);
}

void update_frame(Frame* f) {
  if (f == NULL) return;
  if (f->focus != NULL) {
    f->scroll_x = f->focus->ECB->x + f->focus->ECB->w / 2 - f->rect.w / 2;
    f->scroll_y = f->focus->ECB->y + f->focus->ECB->h / 2 - f->rect.h / 2;
  }

  if (f->bound_left && f->rect.x < f->left_bind) {
    f->rect.x = f->left_bind;
  }

  if (f->bound_top && f->rect.y < f->top_bind) {
    f->rect.y = f->top_bind;
  }

  if (f->bound_right && f->rect.x + f->rect.w > f->right_bind) {
    f->rect.x = f->right_bind - f->rect.w;
  }

  if (f->bound_bottom && f->rect.y + f->rect.h < f->bottom_bind) {
    f->rect.y = f->bottom_bind - f->rect.h;
  }
}

int has_frame(int worldKey) {
  Frame *f, *tmp;
  HASH_ITER(hh, frames, f, tmp) {
    if (worldKey == f->world->name) return 1;
  }
  return 0;
}

int delete_frame(Frame* frame) {
  HASH_DEL(frames, frame);
  free(frame);
  return 0;
}
