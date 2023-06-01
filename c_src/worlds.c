#include "worlds.h"
#include "actors.h"
#include "boxes.h"
#include "debug.h"
#include "frames.h"
#include "sprites.h"
#include "stringmachine.h"
#include "uthash.h"
#include "utlist.h"

World *worlds = NULL;

void add_world(int name, int background, int x_lock, int y_lock) {
  struct World *w;
  w = malloc(sizeof(World));
  if (!w) {
    exit(-1);
  }
  w->actors = NULL;
  w->name = name;
  w->background = background;
  if (x_lock)
    w->x_lock = x_lock;
  if (y_lock)
    w->y_lock = y_lock;
  w->background_x_scroll = 0;
  w->background_y_scroll = 0;
  w->flagged_for_update = 1;

  HASH_ADD_INT(worlds, name, w);
}

World *get_world(int name) {
  struct World *w;
  HASH_FIND_INT(worlds, &name, w);
  if (w) {
    return w;
  } else {
    return NULL;
  }
}

void add_actor_to_world(int worldkey, int actorname) {
  struct World *w;
  w = get_world(worldkey);
  if (!w) {
    printf("World %s not found when adding actor %s\n", get_string(worldkey),
           get_string(actorname));
    return;
  }
  struct ActorEntry *ae;
  ae = malloc(sizeof(ActorEntry));
  ae->actorKey = actorname;

  DL_APPEND(w->actors, ae);
}

int update_world(int worldKey, int debug) {
  struct World *w;
  w = get_world(worldKey);
  if (w == NULL)
    return 0;
  struct ActorEntry *ae, *tmp;
  DL_FOREACH_SAFE(w->actors, ae, tmp) {
    if (update_actor(ae->actorKey, worldKey, debug) == -2) {
      return -2;
    }
  }
  return 0;
}

int alt_modulo(int x, int y) {
  int mod = x % y;
  if ((mod < 0 && y > 0) || (mod > 0 && y < 0)) {
    mod += y;
  }
  return mod;
}

void _draw_background(World *world, SDL_Renderer *rend, Frame *frame) {
  Sprite *background;
  background = get_sprite(world->background);
  if (!background) {
    return;
  }
  SDL_Rect dest;
  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  dest.x = 0;
  dest.y = 0;
  SDL_QueryTexture(background->image, NULL, NULL, &dest.w, &dest.h);
  SDL_QueryTexture(background->image, NULL, NULL, &src.w, &src.h);
  int w, h;
  SDL_GetRendererOutputSize(rend, &w, &h);

  for (int y = 0; y < h / dest.h + 2; y++) {
    dest.y = y * dest.h + world->background_y_scroll;
    dest.y -= alt_modulo((frame->scroll_y / 2), dest.h);
    for (int x = 0; x < w / dest.w + 2; x++) {
      dest.x = x * dest.w + world->background_x_scroll;
      dest.x -= alt_modulo(frame->scroll_x / 2, dest.w);
      SDL_RenderCopy(rend, background->image, &src, &dest);
    }
  }
}

void draw_world(World *world, SDL_Renderer *rend, Frame *frame) {
  _draw_background(world, rend, frame);

  struct ActorEntry *ae;
  DL_FOREACH(world->actors, ae) {
    Actor *a;
    a = get_actor(ae->actorKey);

    if (!a)
      continue;
    if (in_frame(frame, a))
      draw_actor(rend, a, frame);
  }
};

int world_has(World *world, int actorKey) {
  ActorEntry *ae;
  DL_FOREACH(world->actors, ae) {
    if (ae->actorKey == actorKey) {
      return 1;
    }
  }
  return 0;
}

int exists(int actorKey) {
  struct World *w, *tmp;
  HASH_ITER(hh, worlds, w, tmp) {
    if (world_has(w, actorKey))
      return 1;
  }
  return 0;
}

int remove_actor_from_world(World *world, int actorKey) {
  ActorEntry *ae, *tmp;
  DL_FOREACH_SAFE(world->actors, ae, tmp) {
    if (ae->actorKey == actorKey) {
      DL_DELETE(world->actors, ae);
      free(ae);
      return 1;
    }
  }
  return 0;
}

void remove_actor_from_worlds(int actorKey) {
  struct World *w, *tmp;
  HASH_ITER(hh, worlds, w, tmp) { remove_actor_from_world(w, actorKey); }
}
