#include "frames.h"
#include "actors.h"
#include "stringmachine.h"
#include "uthash.h"
#include "worlds.h"
#include <SDL2/SDL.h>

Frame *frames = NULL;

void add_frame(int name, World *world, Actor *focus, int x, int y, int w,
               int h) {
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
  f->focus = focus == NULL ? -1 : focus->name;
  f->bound_bottom = 0;
  f->bound_left = 0;
  f->bound_right = 0;
  f->bound_top = 0;

  f->scroll_x = 0;
  f->scroll_y = 0;

  f->active = 1;
  HASH_ADD_INT(frames, name, f);
}

Frame *get_frame(int name) {
  struct Frame *f;
  HASH_FIND_INT(frames, &name, f);
  if (f) {
    return f;
  } else {
    return NULL;
  }
}
void scrolled(SDL_Rect *rect, Frame *frame) {
  rect->x -= frame->scroll_x;
  rect->y -= frame->scroll_y;
}

int in_frame(Frame *frame, Actor *actor) {
  SDL_Rect *r = malloc(sizeof(SDL_Rect));
  r->x = actor->ECB.x;
  r->y = actor->ECB.y;
  r->w = actor->ECB.w;
  r->h = actor->ECB.h;
  scrolled(r, frame);
  SDL_Rect frameRect = {0, 0, frame->rect.w, frame->rect.h};

  int i = SDL_HasIntersection(&frameRect, r);
  free(r);

  if (i)
    return 1;
  else
    return 0;
}

void draw_frame(SDL_Renderer *rend, Frame *f, int debug) {
  SDL_Texture *render_target = SDL_GetRenderTarget(rend);
  SDL_Texture *frame_buffer =
      SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, f->rect.w, f->rect.h);

  if (frame_buffer == NULL) {
    printf("Error creating frame buffer: %s\n", SDL_GetError());
  }

  if (SDL_SetRenderTarget(rend, frame_buffer) != 0) {
    printf("Error setting render target: %s\n", SDL_GetError());
  }

  draw_world(f->world, rend, f);
  if (debug) {
    draw_debug_overlay(f->world, rend, f);
  }

  if (SDL_SetRenderTarget(rend, render_target) != 0) {
    printf("Error resetting render target: %s\n", SDL_GetError());
  }

  SDL_Rect dest = {f->rect.x, f->rect.y, f->rect.w, f->rect.h};
  SDL_Rect src = {0, 0, f->rect.w, f->rect.h};

  if (SDL_RenderCopy(rend, frame_buffer, &src, &dest) != 0) {
    printf("Error copying frame buffer: %s\n", SDL_GetError());
  }

  SDL_DestroyTexture(frame_buffer);
}

void update_frame(Frame *f) {
  if (f == NULL)
    return;
  if (f->focus != -1) {
    Actor *a = get_actor(f->focus);
    f->scroll_x = a->ECB.x + a->ECB.w / 2 - f->rect.w / 2;
    f->scroll_y = a->ECB.y + a->ECB.h / 2 - f->rect.h / 2;
  }

  if (f->bound_left && f->scroll_x < f->left_bind) {
    f->scroll_x = f->left_bind;
  }

  if (f->bound_top && f->scroll_y < f->top_bind) {
    f->scroll_y = f->top_bind;
  }

  if (f->bound_right && f->scroll_x + f->rect.w > f->right_bind) {
    f->scroll_x = f->right_bind - f->rect.w;
  }

  if (f->bound_bottom && f->scroll_y + f->rect.h > f->bottom_bind) {
    f->scroll_y = f->bottom_bind - f->rect.h;
  }
}

int has_frame(int worldKey) {
  Frame *f, *tmp;
  HASH_ITER(hh, frames, f, tmp) {
    if (worldKey == f->world->name)
      return 1;
  }
  return 0;
}

int delete_frame(Frame *frame) {
  HASH_DEL(frames, frame);
  free(frame);
  return 0;
}

void remove_actor_from_frames(int name) {
  Frame *f, *tmpf;
  HASH_ITER(hh, frames, f, tmpf) {
    if (f->focus != -1 && f->focus == name) {
      f->focus = -1;
    }
  }
}
