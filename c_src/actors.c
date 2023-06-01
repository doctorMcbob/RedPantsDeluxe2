#include "actors.h"
#include "boxes.h"
#include "frames.h"
#include "lists.h"
#include "scripts.h"
#include "stringdata.h"
#include "stringmachine.h"
#include "uthash.h"
#include "worlds.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

Actor *actors = NULL;
Actor *templates = NULL;

Actor *get_actor(int name) {
  struct Actor *a;
  HASH_FIND_INT(actors, &name, a);
  if (a) {
    return a;
  } else {
    return NULL;
  }
}

void actors_reset_updated() {
  Actor *a, *tmp;
  HASH_ITER(hh, actors, a, tmp) { a->updated = 0; }
}

void add_actor(int name, int x, int y, int w, int h, int x_vel, int y_vel,
               int hurtboxkey, int hitboxkey, int scriptmapkey,
               int spritemapkey, int img, int inputKey, int state, int frame,
               int direction, int rotation, int platform, int tangible,
               int physics, int updated) {
  struct Actor *a;
  a = malloc(sizeof(Actor));
  if (!a) {
    exit(-1);
  }
  a->attributes = NULL;
  SDL_Rect *ecb;
  ecb = malloc(sizeof(SDL_Rect));
  if (!ecb) {
    exit(-1);
  }
  ecb->x = x;
  ecb->y = y;
  ecb->w = w;
  ecb->h = h;
  a->ECB = ecb;
  a->name = name;
  a->x_vel = 0;
  a->y_vel = 0;
  a->background = 0;
  a->hurtboxkey = hurtboxkey;

  a->hitboxkey = hitboxkey;

  a->spritemapkey = spritemapkey;

  a->scriptmapkey = scriptmapkey;
  a->img = -1;

  a->_input_name = inputKey;

  if (state > 0)
    a->state = state;
  else
    a->state = index_string("START");

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

  HASH_ADD_INT(actors, name, a);
}

void copy_actor(Actor *copy, Actor *a) {
  a->ECB->x = copy->ECB->x;
  a->ECB->y = copy->ECB->y;
  a->ECB->w = copy->ECB->w;
  a->ECB->h = copy->ECB->h;
  a->name = copy->name;
  a->x_vel = copy->x_vel;
  a->y_vel = copy->y_vel;
  a->hurtboxkey = copy->hurtboxkey;
  a->hitboxkey = copy->hitboxkey;
  a->spritemapkey = copy->spritemapkey;
  a->scriptmapkey = copy->scriptmapkey;
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

void add_template(Actor *copy) {
  struct Actor *a;
  a = malloc(sizeof(Actor));
  if (!a) {
    exit(-1);
  }
  SDL_Rect *ecb;
  ecb = malloc(sizeof(SDL_Rect));
  if (!ecb) {
    exit(-1);
  }
  a->ECB = ecb;
  copy_actor(copy, a);

  HASH_ADD_INT(templates, name, a);
}

Actor *add_actor_from_templatekey(int templateKey, int name) {
  struct Actor *copy;
  HASH_FIND_INT(templates, &templateKey, copy);
  if (copy == NULL) {
    printf("Template %s not found\n", get_string(templateKey));
    return NULL;
  }
  struct Actor *a;
  a = malloc(sizeof(Actor));
  if (!a) {
    exit(-1);
  }
  SDL_Rect *ecb;
  ecb = malloc(sizeof(SDL_Rect));
  if (!ecb) {
    exit(-1);
  }
  a->ECB = ecb;
  copy_actor(copy, a);
  a->name = name;

  HASH_ADD_INT(actors, name, a);
  return a;
}

Actor *get_template(int name) {
  struct Actor *copy;
  HASH_FIND_INT(templates, &name, copy);
  if (copy == NULL) {
    return NULL;
  }
  return copy;
}

void add_template_from_actorkey(int actorKey) {
  struct Actor *copy;
  HASH_FIND_INT(actors, &actorKey, copy);
  struct Actor *a;
  a = malloc(sizeof(Actor));
  if (!a) {
    exit(-1);
  }
  SDL_Rect *ecb;
  ecb = malloc(sizeof(SDL_Rect));
  if (!ecb) {
    exit(-1);
  }
  a->ECB = ecb;
  copy_actor(copy, a);

  HASH_ADD_INT(templates, name, a);
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

SDL_Rect *move(SDL_Rect *rect, int dx, int dy) {
  SDL_Rect *new = malloc(sizeof(SDL_Rect));
  new->x = rect->x + dx;
  new->y = rect->y + dy;
  new->w = rect->w;
  new->h = rect->h;
  return new;
}

float _floor(float x) {
  int i = x > 0 ? 1 : -1;
  x = fabs(x);
  return floor(x) * i;
}

int collision_check(Actor *actor, World *world, int debug) {
  ActorEntry *ae;
  DL_FOREACH(world->actors, ae) {
    if (actor->name == ae->actorKey)
      continue;
    Actor *actor2 = get_actor(ae->actorKey);
    if (SDL_HasIntersection(actor->ECB, actor2->ECB)) {
      int resolution = collision_with(actor, actor2, world, debug);
      if (resolution < 0)
        return resolution;
      int resolution2 = collision_with(actor2, actor, world, debug);
      if (resolution2 < 0)
        return resolution2;
    }
  }

  if (_floor(actor->x_vel) != 0) {
    int direction = actor->x_vel < 0 ? 1 : -1;
    for (int i = 0; i < (_floor(actor->x_vel) / actor->ECB->w); i++) {
      ActorEntry *ae2;
      int exit = 0;
      DL_FOREACH(world->actors, ae2) {
        if (actor->name == ae2->actorKey)
          continue;
        Actor *actor2 = get_actor(ae2->actorKey);
        if (!actor2->tangible)
          continue;
        if (actor->tangible == 0 && actor2->platform == 0)
          continue;
        if (SDL_HasIntersection(move(actor->ECB, actor->ECB->w * i, 0),
                                actor2->ECB)) {
          actor->x_vel -= actor->ECB->w * i;
          exit = 1;
          break;
        }
      }
      if (exit)
        break;
    }

    ActorEntry *ae2;
    DL_FOREACH(world->actors, ae2) {
      if (actor->name == ae2->actorKey)
        continue;
      Actor *actor2 = get_actor(ae2->actorKey);
      if (!actor2->tangible)
        continue;
      if (actor->tangible == 0 && actor2->platform == 0)
        continue;
      if (SDL_HasIntersection(move(actor->ECB, actor->x_vel, 0), actor2->ECB)) {
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
      ActorEntry *ae3;
      DL_FOREACH(world->actors, ae3) {
        if (actor->name == ae3->actorKey)
          continue;
        Actor *actor3 = get_actor(ae3->actorKey);
        if (!actor3->tangible)
          continue;
        if (actor->tangible == 0 && actor3->platform == 0)
          continue;
        if (SDL_HasIntersection(move(actor->ECB, actor->x_vel, 0),
                                actor3->ECB)) {
          check = 0;
        }
      }
      if (check == 0) {
        actor->x_vel += direction;
        actor->x_vel = _floor(actor->x_vel);
      }
    }
  }

  if (floor(fabs(actor->y_vel)) != 0) {
    int direction = actor->y_vel < 0 ? 1 : -1;
    for (int i = 0; i < (_floor(actor->y_vel) / actor->ECB->h); i++) {
      ActorEntry *ae2;
      int exit = 0;
      DL_FOREACH(world->actors, ae2) {
        if (actor->name == ae2->actorKey)
          continue;
        Actor *actor2 = get_actor(ae2->actorKey);
        if (!actor2->tangible)
          continue;
        if (actor->tangible == 0 && actor2->platform == 0)
          continue;
        if (SDL_HasIntersection(move(actor->ECB, 0, actor->ECB->h * i),
                                actor2->ECB)) {
          actor->y_vel -= actor->ECB->h * i;
          exit = 1;
          break;
        }
      }
      if (exit)
        break;
    }

    ActorEntry *ae2;
    DL_FOREACH(world->actors, ae2) {
      if (actor->name == ae2->actorKey)
        continue;
      Actor *actor2 = get_actor(ae2->actorKey);
      if (!actor2->tangible)
        continue;
      if (actor->tangible == 0 && actor2->platform == 0)
        continue;
      if (SDL_HasIntersection(move(actor->ECB, 0, actor->y_vel), actor2->ECB)) {
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
      ActorEntry *ae3;
      DL_FOREACH(world->actors, ae3) {
        if (actor->name == ae3->actorKey)
          continue;
        Actor *actor3 = get_actor(ae3->actorKey);
        if (!actor3->tangible)
          continue;
        if (actor->tangible == 0 && actor3->platform == 0)
          continue;
        if (SDL_HasIntersection(move(actor->ECB, 0, actor->y_vel),
                                actor3->ECB)) {
          check = 0;
        }
      }
      if (check == 0) {
        actor->y_vel += direction;
        actor->y_vel = _floor(actor->y_vel);
      }
    }
  }

  if (_floor(actor->x_vel) != 0 && _floor(actor->y_vel) != 0) {
    ActorEntry *ae4;

    int check = 0;
    while (check == 0) {
      check = 1;
      DL_FOREACH(world->actors, ae4) {
        if (actor->name == ae4->actorKey)
          continue;
        Actor *actor4 = get_actor(ae4->actorKey);
        if (!actor4->tangible)
          continue;
        if (actor->tangible == 0 && actor4->platform == 0)
          continue;
        if (SDL_HasIntersection(move(actor->ECB, actor->x_vel, actor->y_vel),
                                actor4->ECB)) {
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
  ScriptMap *sm = get_script_map(actor->scriptmapkey);
  ScriptMapEntry *sme;

  DL_FOREACH(sm->entries, sme) {
    if (sme->state == scriptName && sme->frame == scriptFrame) {
      return sme->scriptIdx;
    }
  };
  return -1;
}

int update_actor(int actorKey, int worldKey, int debug) {
  Actor *actor = get_actor(actorKey);
  if (!actor)
    return 0;

  World *world = get_world(worldKey);
  if (!world)
    return 0;
  if (actor->updated) {
    if ((actor->physics || actor->tangible) && world_has(world, actorKey)) {
      collision_check(actor, world, debug);
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
    collision_check(actor, world, debug);
    actor->ECB->x += _floor(actor->x_vel);
    actor->ECB->y += _floor(actor->y_vel);
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

  ActorEntry *ae;
  Actor *actor2;

  DL_FOREACH(world->actors, ae) {
    actor2 = get_actor(ae->actorKey);
    if (actor2->name == actor->name)
      continue;
    int resolution = hit_check(actor, actor2, world, debug);
    if (resolution < 0)
      return resolution;
  }

  actor->frame += 1;

  return 0;
}

int get_script_for_actor(Actor *actor) {
  ScriptMap *sm = get_script_map(actor->scriptmapkey);
  if (!sm)
    return -1;
  ScriptMapEntry *best, *sme;
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
    return -1;
  }

  return best->scriptIdx;
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
  int x1 = actor->ECB->x;
  int y1 = actor->ECB->y;
  int w1 = actor->ECB->w;
  int h1 = actor->ECB->h;
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
    rect->x = (actor->ECB->x + actor->ECB->w) - (rect->x + rect->w);
    rect->y = actor->ECB->y + rect->y;
  } else {
    rect->x = actor->ECB->x + rect->x;
    rect->y = actor->ECB->y + rect->y;
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
        break;
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

  for (int y = 0; y < actor->ECB->h / 32; y++) {
    for (int x = 0; x < actor->ECB->w / 32; x++) {
      Sprite *img = NULL;
      if (x == 0 && y == 0)
        img = s00;
      else if (x == actor->ECB->w / 32 - 1 && y == 0)
        img = s02;
      else if (x == 0 && y == actor->ECB->h / 32 - 1)
        img = s20;
      else if (x == actor->ECB->w / 32 - 1 && y == actor->ECB->h / 32 - 1)
        img = s22;
      else if (x == 0)
        img = s10;
      else if (x == actor->ECB->w / 32 - 1)
        img = s12;
      else if (y == 0)
        img = s01;
      else if (y == actor->ECB->h / 32 - 1)
        img = s21;
      else
        img = s11;

      if (img == NULL) {
        continue;
      }
      SDL_Rect dest, src;
      dest.x = actor->ECB->x + x * 32 - frame->scroll_x;
      dest.y = actor->ECB->y + y * 32 - frame->scroll_y;
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

  switch (actor->rotation) {
  case -90:
  case 90:
  case 0:
    dest.x = actor->ECB->x + s->offx;
    dest.y = actor->ECB->y + s->offy;
    break;
  case -270:
  case 270:
  case -180:
  case 180:
    dest.x = actor->ECB->x + actor->ECB->w - s->offx - dest.w;
    dest.y = actor->ECB->y + actor->ECB->h - s->offy - dest.h;
    break;
  }

  src.w = dest.w;
  src.h = dest.h;

  scrolled(&dest, f);

  SDL_RendererFlip flip =
      actor->direction == 1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  double angle = 0 - actor->rotation;
  SDL_Point pivot = {dest.w / 2, dest.h / 2};

  // // draw dest rect
  // SDL_SetRenderDrawColor(rend, 155, 0, 155, 255);
  // SDL_RenderDrawRect(rend, &dest);

  SDL_RenderCopyEx(rend, s->image, &src, &dest, angle, &pivot, flip);
}

void free_actor(Actor *actor) {
  HASH_DEL(actors, actor);

  Attribute *attr, *tmp;

  HASH_ITER(hh, actor->attributes, attr, tmp) {
    if (attr->type == LIST) {
      remove_owner(attr->value.i);
    }
    HASH_DEL(actor->attributes, attr);
    free(attr);
  }

  if (actor->ECB) {
    free(actor->ECB);
  }
  free(actor);
}
