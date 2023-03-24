#include "uthash.h"
#include "worlds.h"
#include "actors.h"
#include "frames.h"
# include <SDL2/SDL.h>

Frame* frames = NULL;

void add_frame(const char* name, World* world, Actor* focus, int x, int y, int w, int h) {
  struct Frame *f;
  f = malloc(sizeof(Frame));
  if (f == NULL) {
    exit(-1);
  }
  strcpy(f->name, name);
  SDL_Rect* rect;
  rect = malloc(sizeof(SDL_Rect));
  rect->x = x;
  rect->y = y;
  rect->w = w;
  rect->h = h;
  f->rect = rect;
  f->world = world;
  f->focus = focus;
  HASH_ADD_STR(frames, name, f);
}

Frame* get_frame(const char* name){
  struct Frame* f;
  HASH_FIND_STR(frames, name, f);
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

int in_frame(const char* frameKey, Actor* actor) {
  Frame* frame = get_frame(frameKey);
  if (!frame) return 0;

  if (SDL_HasIntersection(frame->rect, scrolled(actor->ECB, frame)))
    return 1;
  else
    return 0;
}

void draw_frame(SDL_Renderer* rend, const char* name) {
  struct Frame* f;
  f = get_frame(name);
  Uint32 format;
  SDL_Texture* render_target = SDL_GetRenderTarget(rend);
  SDL_QueryTexture(render_target, &format, NULL, NULL, NULL);
  SDL_Texture* frame_buffer = SDL_CreateTexture(rend, format, SDL_TEXTUREACCESS_TARGET, f->rect->w, f->rect->h);

  SDL_SetRenderTarget(rend, frame_buffer);
  
  draw_world(f->world, rend, name);

  SDL_SetRenderTarget(rend, render_target);

  SDL_Rect src = {0, 0, f->rect->w, f->rect->h};
  SDL_RenderCopy(rend, frame_buffer, &src, f->rect);
  
  SDL_DestroyTexture(frame_buffer);
}

void update_frame(const char* frameKey) {
  Frame* f = get_frame(frameKey);
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

int has_frame(char* worldKey) {
  Frame *f, *tmp;
  HASH_ITER(hh, frames, f, tmp) {
    if (strcmp(worldKey, f->world->name) == 0) return 1;
  }
  return 0;
}