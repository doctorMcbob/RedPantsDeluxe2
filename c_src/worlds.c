#include "worlds.h"
#include "actors.h"
#include "boxes.h"
#include "debug.h"
#include "frames.h"
#include "sprites.h"
#include "stringmachine.h"
#include "utlist.h"
#include "tree.h"
#include "worlddata.h"

struct TreeNode *worlds_by_name = NULL;

void add_world(int key, int name, int background, int x_lock, int y_lock) {
  World *w = &WORLDS[key];
  for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
    w->actors[i] = -1;
  }
  w->name = name;
  w->background = background;
  if (x_lock)
    w->x_lock = x_lock;
  if (y_lock)
    w->y_lock = y_lock;
  w->background_x_scroll = 0;
  w->background_y_scroll = 0;
  w->flagged_for_update = 1;
  worlds_by_name = push_to_tree(worlds_by_name, name, key);
}

World *get_world(int name) {
  struct World *w;
  int idx = value_for_key(worlds_by_name, name);
  w = &WORLDS[idx];
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
  int i = 0;
  while (w->actors[i] != -1) {
    i++;
    if (i >= WORLD_BUFFER_SIZE) {
      printf("World %s is full when adding actor %s. If this is not an infinite memory leak issue, consider compiling with a larger world buffer.\n", get_string(worldkey),
             get_string(actorname));
      return;
    }
  }
  w->actors[i] = actorname;
}

int update_world(int worldKey, int debug) {
  struct World *w;
  w = get_world(worldKey);
  if (w == NULL)
    return 0;

  int update_list[WORLD_BUFFER_SIZE];
  for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
    update_list[i] = w->actors[i];
  }
  for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
    if (update_list[i] == -1) return 0;
    if (update_actor(update_list[i], worldKey, debug) == -2) {
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
    dest.y = y * dest.h;
    dest.y -=
        alt_modulo((frame->scroll_y / 2) + world->background_y_scroll, dest.h);
    for (int x = 0; x < w / dest.w + 2; x++) {
      dest.x = x * dest.w;
      dest.x -= alt_modulo((frame->scroll_x / 2) + world->background_x_scroll,
                           dest.w);
      SDL_RenderCopy(rend, background->image, &src, &dest);
    }
  }
}

void draw_world(World *world, SDL_Renderer *rend, Frame *frame) {
  _draw_background(world, rend, frame);

  for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
    if (world->actors[i] == -1) {
      break;
    }
    Actor *a;
    a = get_actor(world->actors[i]);

    if (!a)
      continue;
    if (in_frame(frame, a))
      draw_actor(rend, a, frame);
  }
};

int world_has(World *world, int actorKey) {
  for (int i = 0; i < WORLD_BUFFER_SIZE; i++) {
    if (world->actors[i] == actorKey)
      return 1;
    if (world->actors[i] == -1)
      return 0;
  }
  return 0;
}

int exists(int actorKey) {
  struct World *w, *tmp;
  for (int i = 0; i < NUM_WORLDS; i++) {
    World *w = &WORLDS[i];
    if (world_has(w, actorKey))
      return 1;
  }
  return 0;
}

int remove_actor_from_world(World *world, int actorKey) {
  int i = 0;
  int shiftIndex = -1; // Index to store the position to shift actors to
  int found = 0;
  while (world->actors[i] != -1) {
    if (world->actors[i] == actorKey) {
      found = 1;
      shiftIndex = i; // Store the position to shift actors to
      world->actors[i] = -1; // Set the actor to be removed as -1
    }
    i++;
    if (i >= WORLD_BUFFER_SIZE) {
      // actor not in world and world is full
      return 0;
    }
  }

  if (shiftIndex != -1) {
    // Shift the remaining actors forward
    for (i = shiftIndex + 1; world->actors[i] != -1 && i < WORLD_BUFFER_SIZE; i++) {
      world->actors[i - 1] = world->actors[i];
      world->actors[i] = -1; // Clear the previous position
    }
  }

  return found;
}


void remove_actor_from_worlds(int actorKey) {
  struct World *w;
  for (int i = 0; i < NUM_WORLDS; i++) {
    World *w = &WORLDS[i];
    remove_actor_from_world(w, actorKey);
  }
}

World* world_with(int actorKey) {
  struct World *w;
  for (int i = 0; i < NUM_WORLDS; i++) {
    World *w = &WORLDS[i];
    if (world_has(w, actorKey))
      return w;
  }
  return NULL;
}