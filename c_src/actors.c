#include "actors.h"
#include "actordata.h"
#include "scriptdata.h"
#include "boxes.h"
#include "frames.h"
#include "lists.h"
#include "scripts.h"
#include "stringdata.h"
#include "stringmachine.h"
#include "tree.h"
#include "uthash.h"
#include "worlddata.h"
#include "worlds.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

struct TreeNode *actors_by_name = NULL;
struct TreeNode *templates_by_name = NULL;
int DEEPEST_ACTOR = 0;

Actor *get_actor(int name) {
  int idx = value_for_key(actors_by_name, name);
  if (idx < 0)
    return NULL;
  return &ACTORS[idx];
}

void actors_reset_updated() {
  for (int i = 0; i < DEEPEST_ACTOR; i++) {
    ACTORS[i].updated = 0;
  }
}

void add_actor(int name, int x, int y, int w, int h, int x_vel, int y_vel,
               int hurtboxkey, int hitboxkey, int scriptmapkey,
               int spritemapkey, int img, int inputKey, int state, int frame,
               int direction, int rotation, int platform, int tangible,
               int physics, int updated) {
  int key = DEEPEST_ACTOR++;
  Actor *a = &ACTORS[key];
  if (!a) {
    exit(-1);
  }
  a->attributes = NULL;
  a->ECB.x = x;
  a->ECB.y = y;
  a->ECB.w = w;
  a->ECB.h = h;
  a->name = name;
  a->x_vel = 0;
  a->y_vel = 0;
  a->background = 0;
  a->hurtboxkey = hurtboxkey;

  a->hitboxkey = hitboxkey;

  a->spritemapkey = spritemapkey;

  load_script_map_into_actor(a, scriptmapkey);

  a->img = -1;

  a->_input_name = inputKey;

  if (state > 0)
    a->state = state;
  else
    a->state = _START;

  if (frame)
    a->frame = frame;
  else
    a->frame = 0;

  if (direction)
    a->direction = direction;
  else
    a->direction = -1;

  if (rotation)
    a->rotation = rotation;
  else
    a->rotation = 0;

  if (platform)
    a->platform = platform;
  else
    a->platform = 0;

  if (tangible)
    a->tangible = tangible;
  else
    a->tangible = 0;

  if (physics)
    a->physics = physics;
  else
    a->physics = 0;

  if (updated)
    a->updated = updated;
  else
    a->updated = 0;

  actors_by_name = push_to_tree(actors_by_name, name, key);
  validate_actors();
}

void copy_actor(Actor *copy, Actor *a) {
  a->ECB.x = copy->ECB.x;
  a->ECB.y = copy->ECB.y;
  a->ECB.w = copy->ECB.w;
  a->ECB.h = copy->ECB.h;
  a->name = copy->name;
  a->x_vel = copy->x_vel;
  a->y_vel = copy->y_vel;
  a->hurtboxkey = copy->hurtboxkey;
  a->hitboxkey = copy->hitboxkey;
  a->spritemapkey = copy->spritemapkey;
  for (int i = 0; i < LARGEST_SCRIPT_MAP; i++) {
    a->scriptmap[i] = copy->scriptmap[i];
  }
  a->img = -1;
  a->_input_name = copy->_input_name;
  a->state = copy->state;
  a->frame = copy->frame;
  a->direction = copy->direction;
  a->rotation = copy->rotation;
  a->platform = copy->platform;
  a->tangible = copy->tangible;
  a->physics = copy->physics;
  a->updated = copy->updated;
  a->attributes = NULL;
  a->background = copy->background;
}

Actor *add_actor_from_templatekey(int templateKey, int name) {
  int idx = value_for_key(templates_by_name, templateKey);
  if (idx == -1) {
    printf("Template %s not found\n", get_string(templateKey));
    return NULL;
  }
  if (DEEPEST_ACTOR >= NUM_ACTORS) {
    printf("Could not add actor %s from template %s because we ran out of space. Consider adding to buffer size.\n");
    return NULL;
  }
  copy_actor(&TEMPLATES[idx], &ACTORS[DEEPEST_ACTOR]);
  ACTORS[DEEPEST_ACTOR].name = name;

  actors_by_name = push_to_tree(actors_by_name, name, DEEPEST_ACTOR);

  return &ACTORS[DEEPEST_ACTOR++];
}

Actor *get_template(int name) {
  int idx = value_for_key(templates_by_name, name);
  if (idx < 0)
    return NULL;
  return &TEMPLATES[idx];
}

void add_template_from_actorkey(int idx, int actorKey) {
  int a_idx = value_for_key(actors_by_name, actorKey);
  if (a_idx < 0) {
    printf("Could not creat template, Actor %s not found\n",
           get_string(actorKey));
    return;
  }
  copy_actor(&ACTORS[a_idx], &TEMPLATES[idx]);

  templates_by_name = push_to_tree(templates_by_name, actorKey, idx);
}

int collision_with(Actor *a1, Actor *a2, World *world, int debug) {
  int scriptKey = find_script_from_map(a1, COLLIDE, -1);
  if (scriptKey != -1) {
    int resolution =
        resolve_script(scriptKey, a2, a1, world, debug, -1, -1, -1, -1, -1, 0);
    if (resolution < 0) 
      return resolution;
  }
  return 0;
}

void move(SDL_Rect *rect, int dx, int dy) {
  rect->x += dx;
  rect->y += dy;
}

float _floor(float x) {
  int i = x > 0 ? 1 : -1;
  x = fabs(x);
  return floor(x) * i;
}

int collision_check(Actor *actor, World *world, int debug) {
  int buffer[WORLD_BUFFER_SIZE];

  for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
    buffer[idx] = world->actors[idx];
  }

  for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
    if (buffer[idx] == -1)
      break;
    if (actor->name == buffer[idx])
      continue;
    Actor *actor2 = get_actor(buffer[idx]);
    if (actor2 == NULL)
      continue;
    if (SDL_HasIntersection(&actor->ECB, &actor2->ECB)) {
      int resolution = collision_with(actor, actor2, world, debug);
      if (resolution < 0)
        return resolution;
      int resolution2 = collision_with(actor2, actor, world, debug);
      if (resolution2 < 0)
        return resolution2;
    }
  }
  if (!world_has(world, actor->name)) {
    // we do this because the collision_with scripts may change which world the
    // actor is in. (ie, doors)
    world = world_with(actor->name);
    if (world == NULL)
      return 0;
    for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
      buffer[idx] = world->actors[idx];
    }
  }
  SDL_Rect moved;

  if (_floor(actor->x_vel) != 0) {
    int direction = actor->x_vel < 0 ? 1 : -1;
    for (int i = 0; i < (_floor(actor->x_vel) / actor->ECB.w); i++) {
      int exit = 0;
      moved.x = actor->ECB.x;
      moved.y = actor->ECB.y;
      moved.w = actor->ECB.w;
      moved.h = actor->ECB.h;
      move(&moved, actor->ECB.w * i, 0);
      for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
        if (buffer[idx] == -1)
          break;
        if (actor->name == buffer[idx])
          continue;
        Actor *actor2 = get_actor(buffer[idx]);
        if (actor2 == NULL)
          continue;
        if (!actor2->tangible)
          continue;
        if (actor->tangible == 0 && actor2->platform == 0)
          continue;
        if (SDL_HasIntersection(&moved, &actor2->ECB)) {
          actor->x_vel -= actor->ECB.w * i;
          exit = 1;
          break;
        }
      }
      if (exit)
        break;
    }

    moved.x = actor->ECB.x;
    moved.y = actor->ECB.y;
    moved.w = actor->ECB.w;
    moved.h = actor->ECB.h;
    move(&moved, actor->x_vel, 0);
    for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
      if (buffer[idx] == -1)
        break;

      if (actor->name == buffer[idx])
        continue;

      Actor *actor2 = get_actor(buffer[idx]);
      if (actor2 == NULL || !actor2->tangible)
        continue;
      if (actor->tangible == 0 && actor2->platform == 0)
        continue;
      if (SDL_HasIntersection(&moved, &actor2->ECB)) {
        int resolution3 = collision_with(actor, actor2, world, debug);
        if (resolution3 < 0)
          return resolution3;
        int resolution4 = collision_with(actor2, actor, world, debug);
        if (resolution4 < 0)
          return resolution4;
      }
    }

    int check = 0;
    while (check == 0) {
      check = 1;
      moved.x = actor->ECB.x;
      moved.y = actor->ECB.y;
      moved.w = actor->ECB.w;
      moved.h = actor->ECB.h;
      move(&moved, actor->x_vel, 0);
      Actor *actor3;
      for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
        if (buffer[idx] == -1)
          break;

        if (actor->name == buffer[idx])
          continue;

        actor3 = get_actor(buffer[idx]);
        if (actor3 == NULL || !actor3->tangible)
          continue;
        if (actor->tangible == 0 && actor3->platform == 0)
          continue;
        if (SDL_HasIntersection(&moved, &actor3->ECB)) {
          check = 0;
          break;
        }
      }
      if (check == 0) {
        int dx;
        if (direction == 1) {
          dx = actor3->ECB.x + actor3->ECB.w - moved.x;
        } else {
          dx = (moved.x + moved.w - actor3->ECB.x) * -1;
        }
        actor->x_vel += dx;
        actor->x_vel = _floor(actor->x_vel);
      }
    }
  }

  if (floor(fabs(actor->y_vel)) != 0) {
    int direction = actor->y_vel < 0 ? 1 : -1;
    for (int i = 0; i < (_floor(actor->y_vel) / actor->ECB.h); i++) {
      int exit = 0;
      moved.x = actor->ECB.x;
      moved.y = actor->ECB.y;
      moved.w = actor->ECB.w;
      moved.h = actor->ECB.h;
      move(&moved, 0, actor->ECB.h * i);
      for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
        if (buffer[idx] == -1)
          break;

        if (actor->name == buffer[idx])
          continue;

        Actor *actor2 = get_actor(buffer[idx]);
        if (actor2 == NULL)
          continue;
        if (!actor2->tangible)
          continue;
        if (actor->tangible == 0 && actor2->platform == 0)
          continue;
        if (SDL_HasIntersection(&moved, &actor2->ECB)) {
          actor->y_vel -= actor->ECB.h * i;
          exit = 1;
          break;
        }
      }
      if (exit)
        break;
    }

    moved.x = actor->ECB.x;
    moved.y = actor->ECB.y;
    moved.w = actor->ECB.w;
    moved.h = actor->ECB.h;
    move(&moved, 0, actor->y_vel);
    for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
      if (buffer[idx] == -1)
        break;

      if (actor->name == buffer[idx])
        continue;

      Actor *actor2 = get_actor(buffer[idx]);
      if (actor2 == NULL || !actor2->tangible)
        continue;
      if (actor->tangible == 0 && actor2->platform == 0)
        continue;
      if (SDL_HasIntersection(&moved, &actor2->ECB)) {
        int resolution3 = collision_with(actor, actor2, world, debug);
        if (resolution3 < 0)
          return resolution3;
        int resolution4 = collision_with(actor2, actor, world, debug);
        if (resolution4 < 0)
          return resolution4;
      }
    }

    int check = 0;
    while (check == 0) {
      check = 1;
      moved.x = actor->ECB.x;
      moved.y = actor->ECB.y;
      moved.w = actor->ECB.w;
      moved.h = actor->ECB.h;
      move(&moved, 0, actor->y_vel);
      Actor *actor3;
      for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
        if (world->actors[idx] == -1)
          break;

        if (actor->name == world->actors[idx])
          continue;

        actor3 = get_actor(world->actors[idx]);
        if (actor3 == NULL || !actor3->tangible)
          continue;
        if (actor->tangible == 0 && actor3->platform == 0)
          continue;
        if (SDL_HasIntersection(&moved, &actor3->ECB)) {
          check = 0;
          break;
        }
      }
      if (check == 0) {
        int dx;
        if (direction == 1) {
          dx = actor3->ECB.y + actor3->ECB.h - moved.y;
        } else {
          dx = (moved.y + moved.h - actor3->ECB.y) * -1;
        }
        actor->y_vel += dx;
        actor->y_vel = _floor(actor->y_vel);
      }
    }
  }

  if (_floor(actor->x_vel) != 0 && _floor(actor->y_vel) != 0) {
    int check = 0;
    while (check == 0) {
      check = 1;
      moved.x = actor->ECB.x;
      moved.y = actor->ECB.y;
      moved.w = actor->ECB.w;
      moved.h = actor->ECB.h;
      move(&moved, actor->x_vel, actor->y_vel);
      for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
        if (buffer[idx] == -1)
          break;

        if (actor->name == buffer[idx])
          continue;

        Actor *actor4 = get_actor(buffer[idx]);
        if (actor4 == NULL || !actor4->tangible)
          continue;
        if (actor->tangible == 0 && actor4->platform == 0)
          continue;
        if (SDL_HasIntersection(&moved, &actor4->ECB)) {
          check = 0;
        }
      }
      if (check == 0) {
        if (floor(actor->x_vel) == 0 || floor(actor->y_vel) == 0) {
          check = 1;
        } else {
          actor->x_vel += actor->x_vel < 0 ? 1 : -1;
          actor->y_vel += actor->y_vel < 0 ? 1 : -1;
        }
      }
    }
  }
  return 0;
}

int find_script_from_map(Actor *actor, int scriptName, int scriptFrame) {
  int i = 0;
  while (i < LARGEST_SCRIPT_MAP) {
    if (actor->scriptmap[i] == -1) {
      return -1;
    }
    int state = actor->scriptmap[i++];
    int frame = actor->scriptmap[i++];
    int idx = actor->scriptmap[i++];
    if (state == scriptName && frame == scriptFrame) {
      return idx;
    }
  }
  return -1;
}

void pop_from_script_map(Actor *actor, int scriptName, int scriptFrame) {
  int i = 0;
  int j = -1;
  while (i < LARGEST_SCRIPT_MAP) {
    if (actor->scriptmap[i] == -1) {
      break;
    }
    int state = actor->scriptmap[i++];
    int frame = actor->scriptmap[i++];
    int idx = actor->scriptmap[i++];
    if (state == scriptName && frame == scriptFrame) {
      j = i;
      continue;
    }
    if (j != -1) {
      actor->scriptmap[j++] = state;
      actor->scriptmap[j++] = frame;
      actor->scriptmap[j++] = idx;
      actor->scriptmap[i - 3] = -1;
      actor->scriptmap[i - 2] = -1;
      actor->scriptmap[i - 1] = -1;
    }
  }
}

int update_actor(int actorKey, int worldKey, int debug) {
  Actor *actor = get_actor(actorKey);
  if (actor == NULL)
    return 0;

  World *world = get_world(worldKey);
  if (!world)
    return 0;
  if (actor->updated) {
    if ((actor->physics || actor->tangible) && world_has(world, actorKey)) {
      int resolution = collision_check(actor, world, debug);
      if (resolution < 0)
        return resolution;
    }
    return 0;
  }
  actor->updated = 1;
  actor->img = -1;
  int scriptKey = get_script_for_actor(actor);
  if (scriptKey != -1) {
    int resolution = resolve_script(scriptKey, actor, NULL, world, debug, -1,
                                    -1, -1, -1, -1, 0);
    if (resolution < 0) {
      return resolution;
    }
  }
  float x_flag = actor->x_vel, y_flag = actor->y_vel;
  if (actor->physics || actor->tangible) {
    int resolution = collision_check(actor, world, debug);
    if (resolution < 0)
      return resolution;
    actor->ECB.x += _floor(actor->x_vel);
    actor->ECB.y += _floor(actor->y_vel);
  }

  if (x_flag != actor->x_vel && _floor(actor->x_vel) == 0) {
    int scriptKey = find_script_from_map(actor, XCOLLISION, -1);
    if (scriptKey != -1) {
      int resolution = resolve_script(scriptKey, actor, NULL, world, debug, -1,
                                      -1, -1, -1, -1, 0);

      if (resolution < 0)
        return resolution;
    }
  }
  if (y_flag != actor->y_vel && _floor(actor->y_vel) == 0) {
    actor->y_vel = 0;
    int scriptKey = find_script_from_map(actor, YCOLLISION, -1);
    if (scriptKey != -1) {
      int resolution = resolve_script(scriptKey, actor, NULL, world, debug, -1,
                                      -1, -1, -1, -1, 0);
      if (resolution < 0)
        return resolution;
    }
  }

  int buffer[WORLD_BUFFER_SIZE];
  for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
    buffer[idx] = world->actors[idx];
  }
  for (int idx = 0; idx < WORLD_BUFFER_SIZE; idx++) {
    if (buffer[idx] == -1)
      break;
    if (actor->name == buffer[idx])
      continue;
    Actor *actor2 = get_actor(buffer[idx]);
    if (actor2 == NULL)
      continue;
    int resolution = hit_check(actor, actor2, world, debug);
    if (resolution < 0)
      return resolution;
  }

  actor->frame += 1;

  return 0;
}

int get_script_for_actor(Actor *actor) {
  int bestIdx = -1;
  int bestFrame = -1;
  int i = 0;
  while (i < LARGEST_SCRIPT_MAP) {
    int state = actor->scriptmap[i++];
    int frame = actor->scriptmap[i++];
    int idx = actor->scriptmap[i++];
    if (state != actor->state)
      continue;
    if (actor->frame < frame)
      continue;
    if (bestFrame > frame)
      continue;
    bestFrame = frame;
    bestIdx = idx;
  }
  return bestIdx;
}

Sprite *get_sprite_for_actor(Actor *actor) {
  if (actor->platform) {
    return NULL;
  }
  if (actor->img >= 0) {
    Sprite *img = get_sprite(actor->img);
    return img;
  }

  struct SpriteMap *sm;
  sm = get_sprite_map(actor->spritemapkey);
  if (!sm)
    return NULL;
  SpriteMapEntry *best, *sme;
  best = NULL;
  DL_FOREACH(sm->entries, sme) {
    if (actor->state != sme->state)
      continue;
    if (actor->frame < sme->frame)
      continue;
    if (best) {
      if (sme->frame < best->frame)
        continue;
    }
    best = sme;
  }
  if (!best) {
    return NULL;
  }
  return get_sprite(best->spriteKey);
}

BoxMapEntry *get_hitboxes_for_actor(Actor *actor) {
  if (actor->hitboxkey == NULL)
    return NULL;
  BoxMap *bm = get_hitbox_map(actor->hitboxkey);
  if (!bm)
    return NULL;
  BoxMapEntry *best, *bme;
  best = NULL;
  DL_FOREACH(bm->entries, bme) {
    if (actor->state != bme->state)
      continue;
    if (actor->frame < bme->frame)
      continue;
    if (best) {
      if (bme->frame < best->frame)
        continue;
    }
    best = bme;
  }
  if (best != NULL)
    return best;
  return NULL;
}

BoxMapEntry *get_hurtboxes_for_actor(Actor *actor) {
  if (actor->hurtboxkey == NULL)
    return NULL;
  BoxMap *bm = get_hurtbox_map(actor->hurtboxkey);
  if (!bm)
    return NULL;
  BoxMapEntry *best, *bme;
  best = NULL;
  DL_FOREACH(bm->entries, bme) {
    if (actor->state != bme->state)
      continue;
    if (actor->frame < bme->frame)
      continue;
    if (best) {
      if (bme->frame < best->frame)
        continue;
    }
    best = bme;
  }
  if (best != NULL)
    return best;
  return NULL;
}

void rotate_box_by_actor(Actor *actor, SDL_Rect *rect, int deg) {
  int x1 = actor->ECB.x;
  int y1 = actor->ECB.y;
  int w1 = actor->ECB.w;
  int h1 = actor->ECB.h;
  int x2 = rect->x - x1;
  int y2 = rect->y - y1;
  int w2 = rect->w;
  int h2 = rect->h;
  switch (deg) {
  case 0:
    rect->x = x1 + x2;
    rect->y = y1 + y2;
    rect->w = w2;
    rect->h = h2;
    break;
  case 90:
    if (actor->direction == 1) {
      rect->x = x1 + y2;
      rect->y = y1 + h1 + h1 - x2 - w2;
      rect->w = h2;
      rect->h = w2;
    } else {
      rect->x = x1 + y2;
      rect->y = y1 + h1 - x2 - w2;
      rect->w = h2;
      rect->h = w2;
    }
    break;
  case 180:
    rect->x = x1 + w1 - x2 - w2;
    rect->y = y1 + h1 - y2 - h2;
    rect->w = w2;
    rect->h = h2;
    break;
  case 270:
    if (actor->direction == 1) {
      rect->x = x1 + w1 - y2 - h2;
      rect->y = y1 + x2 - h1;
      rect->w = h2;
      rect->h = w2;
    } else {
      rect->x = x1 + w1 - y2 - h2;
      rect->y = y1 + x2;
      rect->w = h2;
      rect->h = w2;
    }
    break;
  }
}

void translate_rect_by_actor(Actor *actor, SDL_Rect *rect) {
  if (actor == NULL || rect == NULL)
    return;

  if (actor->direction == 1) {
    rect->x = (actor->ECB.x + actor->ECB.w) - (rect->x + rect->w);
    rect->y = actor->ECB.y + rect->y;
  } else {
    rect->x = actor->ECB.x + rect->x;
    rect->y = actor->ECB.y + rect->y;
  }
  rotate_box_by_actor(actor, rect, actor->rotation);
}

int hit_check(Actor *self, Actor *related, World *world, int debug) {
  BoxMapEntry *hurtboxes, *hitboxes;

  hurtboxes = get_hurtboxes_for_actor(self);
  if (hurtboxes == NULL)
    return 0;
  hitboxes = get_hitboxes_for_actor(related);
  if (hitboxes == NULL)
    return 0;

  for (int i = 0; i < hurtboxes->count; i++) {
    SDL_Rect hurtbox = {hurtboxes->rect[i].x, hurtboxes->rect[i].y,
                        hurtboxes->rect[i].w, hurtboxes->rect[i].h};
    translate_rect_by_actor(self, &hurtbox);

    for (int j = 0; j < hitboxes->count; j++) {
      SDL_Rect hitbox = {hitboxes->rect[j].x, hitboxes->rect[j].y,
                         hitboxes->rect[j].w, hitboxes->rect[j].h};
      translate_rect_by_actor(related, &hitbox);
      if (SDL_HasIntersection(&hurtbox, &hitbox)) {
 
        int scriptKey = find_script_from_map(related, HIT, -1);
        if (scriptKey != -1) {
          int resolution = resolve_script(scriptKey, self, related, world,
                                          debug, -1, -1, -1, -1, -1, 0);
          if (resolution < 0) {
            return resolution;
          }
        }
        return 0;
      }
    }
  }
  return 0;
}

void _draw_platform(SDL_Renderer *rend, Actor *actor, Frame *frame) {
  struct SpriteMap *sm;
  sm = get_sprite_map(actor->spritemapkey);
  if (!sm)
    return;

  char key00[32], key01[32], key02[32], key10[32], key11[32], key12[32],
      key20[32], key21[32], key22[32];

  sprintf(key00, "%s%i%i", get_string(actor->state), 0, 0);
  sprintf(key01, "%s%i%i", get_string(actor->state), 0, 1);
  sprintf(key02, "%s%i%i", get_string(actor->state), 0, 2);
  sprintf(key10, "%s%i%i", get_string(actor->state), 1, 0);
  sprintf(key11, "%s%i%i", get_string(actor->state), 1, 1);
  sprintf(key12, "%s%i%i", get_string(actor->state), 1, 2);
  sprintf(key20, "%s%i%i", get_string(actor->state), 2, 0);
  sprintf(key21, "%s%i%i", get_string(actor->state), 2, 1);
  sprintf(key22, "%s%i%i", get_string(actor->state), 2, 2);

  Sprite *s00, *s01, *s02, *s10, *s11, *s12, *s20, *s21, *s22;

  SpriteMapEntry *sme;
  DL_FOREACH(sm->entries, sme) {
    if (strcmp(get_string(sme->state), key00) == 0) {
      s00 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key01) == 0) {
      s01 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key02) == 0) {
      s02 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key10) == 0) {
      s10 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key11) == 0) {
      s11 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key12) == 0) {
      s12 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key20) == 0) {
      s20 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key21) == 0) {
      s21 = get_sprite(sme->spriteKey);
      continue;
    }
    if (strcmp(get_string(sme->state), key22) == 0) {
      s22 = get_sprite(sme->spriteKey);
    }
  }

  for (int y = 0; y < actor->ECB.h / 32; y++) {
    for (int x = 0; x < actor->ECB.w / 32; x++) {
      Sprite *img = NULL;
      if (x == 0 && y == 0)
        img = s00;
      else if (x == actor->ECB.w / 32 - 1 && y == 0)
        img = s02;
      else if (x == 0 && y == actor->ECB.h / 32 - 1)
        img = s20;
      else if (x == actor->ECB.w / 32 - 1 && y == actor->ECB.h / 32 - 1)
        img = s22;
      else if (x == 0)
        img = s10;
      else if (x == actor->ECB.w / 32 - 1)
        img = s12;
      else if (y == 0)
        img = s01;
      else if (y == actor->ECB.h / 32 - 1)
        img = s21;
      else
        img = s11;

      if (img == NULL) {
        continue;
      }
      SDL_Rect dest, src;
      dest.x = actor->ECB.x + x * 32 - frame->scroll_x;
      dest.y = actor->ECB.y + y * 32 - frame->scroll_y;
      src.x = 0;
      src.y = 0;

      SDL_QueryTexture(img->image, NULL, NULL, &dest.w, &dest.h);
      SDL_QueryTexture(img->image, NULL, NULL, &src.w, &src.h);

      SDL_RenderCopy(rend, img->image, &src, &dest);
    }
  }
}

void draw_actor(SDL_Renderer *rend, Actor *actor, Frame *f) {
  if (actor->platform) {
    return _draw_platform(rend, actor, f);
  }

  Sprite *s;
  s = get_sprite_for_actor(actor);
  if (s == NULL)
    return;

  SDL_Rect dest, src;

  SDL_QueryTexture(s->image, NULL, NULL, &dest.w, &dest.h);

  src.x = 0;
  src.y = 0;

  src.w = dest.w;
  src.h = dest.h;

  SDL_RendererFlip flip =
      actor->direction == 1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  double angle = 0 - actor->rotation;

  actor->rotation = (actor->rotation + 360) % 360;
  
  SDL_Point pivot;
  switch (actor->rotation) {
	case 0: {
  		dest.x = actor->direction == 1 ? actor->ECB.x + actor->ECB.w - s->offx - dest.w : actor->ECB.x + s->offx;
  		dest.y = actor->ECB.y + s->offy;
 		
		pivot.x = dest.w / 2;
		pivot.y = dest.h / 2;
		break;
	}
	case 180: {
  		dest.x = actor->direction == 1 ? actor->ECB.x + actor->ECB.w - s->offx - dest.w : actor->ECB.x + s->offx;
		dest.y = actor->ECB.y + actor->ECB.h - s->offy - dest.h;
 		
		pivot.x = dest.w / 2;
		pivot.y = dest.h / 2;
		break;
	}
	case 90: {
		dest.x = actor->ECB.x + s->offy;
		dest.y = actor->ECB.y + actor->ECB.h - s->offx - dest.w;

		pivot.x = 0;
		pivot.y = 0;
		dest.w = src.w;
		dest.h = src.h;
		dest.y += dest.w;
		break;
	}
	case 270: {
		dest.x = actor->ECB.x + actor->ECB.w - s->offy - dest.h;
		dest.y = actor->ECB.y + actor->ECB.h - s->offx - dest.w ;

		pivot.x = 0;
		pivot.y = 0;
		dest.w = src.w;
		dest.h = src.h;
		dest.x += dest.h;
		break;
	}
  }
  
  scrolled(&dest, f);
  // // draw dest rect ;P my favorite debug tool <3
  // SDL_SetRenderDrawColor(rend, 155, 0, 155, 255);
  // SDL_RenderDrawRect(rend, &dest);

  SDL_RenderCopyEx(rend, s->image, &src, &dest, angle, &pivot, flip);
}

void free_actor(Actor *actor) {
  Attribute *attr, *tmp;

  HASH_ITER(hh, actor->attributes, attr, tmp) {
    if (attr->type == LIST) {
      remove_owner(attr->value.i);
    }
    HASH_DEL(actor->attributes, attr);
    free(attr);
  }

  int idx = value_for_key(actors_by_name, actor->name);
  actors_by_name = remove_from_tree(actors_by_name, actor->name);
  if (idx == --DEEPEST_ACTOR) {
    return;
  }
  copy_actor(&ACTORS[DEEPEST_ACTOR], &ACTORS[idx]);
  ACTORS[idx].attributes = ACTORS[DEEPEST_ACTOR].attributes;
  actors_by_name = remove_from_tree(actors_by_name, ACTORS[idx].name);
  actors_by_name = push_to_tree(actors_by_name, ACTORS[idx].name, idx);

  validate_actors();
}

void validate_actors() {
  for (int i = 0; i < DEEPEST_ACTOR; i++) {
    int idx = value_for_key(actors_by_name, ACTORS[i].name);
    if (idx != i)
      printf("Actor %s is at %i in memory but %i in map (DEEPEST ACTOR %i)\n",
             get_string(ACTORS[i].name), i, idx, DEEPEST_ACTOR);
  }
}
