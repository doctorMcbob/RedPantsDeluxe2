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
  SDL_Rect* rect;
  rect = malloc(sizeof(SDL_Rect));
  rect->x = x;
  rect->y = y;
  rect->w = w;
  rect->h = h;
  f->rect = rect;
  f->world = world;
  f->focus = focus;
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

SDL_Rect* scrolled(SDL_Rect* rect, Frame* frame) {
  SDL_Rect scrolled;
  scrolled.x = rect->x - frame->scroll_x;
  scrolled.y = rect->y - frame->scroll_y;
  scrolled.w = rect->w;
  scrolled.h = rect->h;
  SDL_Rect* r = &scrolled;
  return r;
}

int in_frame(Frame* frame, Actor* actor) {
  if (SDL_HasIntersection(frame->rect, scrolled(actor->ECB, frame)))
    return 1;
  else
    return 0;
}

void draw_frame(SDL_Renderer* rend, Frame* f, int debug) {
  Uint32 format;
  SDL_Texture* render_target = SDL_GetRenderTarget(rend);
  SDL_QueryTexture(render_target, &format, NULL, NULL, NULL);
  SDL_Texture* frame_buffer = SDL_CreateTexture(
    rend, format, SDL_TEXTUREACCESS_TARGET, f->rect->w, f->rect->h);

  SDL_SetRenderTarget(rend, frame_buffer);
  
  draw_world(f->world, rend, f);
  if (debug) {
    draw_debug_overlay(f->world, rend, f);
  }

  SDL_SetRenderTarget(rend, render_target);

  SDL_Rect src = {0, 0, f->rect->w, f->rect->h};
  SDL_RenderCopy(rend, frame_buffer, &src, f->rect);
  
  SDL_DestroyTexture(frame_buffer);
}

void update_frame(Frame* f) {
  if (f->focus != NULL) {
    f->scroll_x = f->focus->ECB->x + f->focus->ECB->w / 2 - f->rect->w / 2;
    f->scroll_y = f->focus->ECB->y + f->focus->ECB->h / 2 - f->rect->h / 2;
  }

  if (f->bound_left && f->rect->x < f->left_bind) {
    f->rect->x = f->left_bind;
  }

  if (f->bound_top && f->rect->y < f->top_bind) {
    f->rect->y = f->top_bind;
  }

  if (f->bound_right && f->rect->x + f->rect->w > f->right_bind) {
    f->rect->x = f->right_bind - f->rect->w;
  }

  if (f->bound_bottom && f->rect->y + f->rect->h < f->bottom_bind) {
    f->rect->y = f->bottom_bind - f->rect->h;
  }
}

int has_frame(int worldKey) {
  Frame *f, *tmp;
  HASH_ITER(hh, frames, f, tmp) {
    if (worldKey == f->world->name) return 1;
  }
  return 0;
}
