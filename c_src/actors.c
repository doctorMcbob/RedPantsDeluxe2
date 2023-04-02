# include <SDL2/SDL.h>
# include <stdio.h>
# include <string.h>
# include <math.h>
# include "uthash.h"
# include "actors.h"
# include "worlds.h"
# include "frames.h"
# include "scripts.h"
# include "stringmachine.h"
# include <math.h>

Actor* actors = NULL;
Actor* templates = NULL;

Actor* get_actor(int name) {
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
    HASH_ITER(hh, actors, a, tmp) {
      a->updated = 0;
    }
}

void add_actor(int name,
	       int x,
	       int y,
	       int w,
	       int h,
	       int x_vel,
	       int y_vel,
	       int hurtboxkey,
	       int hitboxkey,
	       int scriptmapkey,
	       int spritemapkey,
	       int img,
	       int inputKey,
	       int state,
	       int frame,
	       int direction,
	       int rotation,
	       int platform,
	       int tangible,
	       int physics,
	       int updated) {
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
  if (x_vel)
    a->x_vel = x_vel;
  else
    a->x_vel = 0;
  if (y_vel)
    a->y_vel = y_vel;
  else
    a->y_vel = 0;
  
  a->hurtboxkey= hurtboxkey;
  
  a->hitboxkey= hitboxkey;
    
  a->spritemapkey = spritemapkey;
  
  a->scriptmapkey = scriptmapkey;
  a->img = -1;

  a->_input_name = inputKey;

  if (state > 0)
    a->state =  state;
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

void copy_actor(Actor* copy,  Actor *a) {
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
}

void add_template(Actor* copy) {
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

void add_actor_from_templatekey(int templateKey) {
  struct Actor *copy;
  HASH_FIND_INT(templates, &templateKey, copy);
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
  
  HASH_ADD_INT(actors, name, a);
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

int collision_with(Actor *a1, Actor *a2, World* world) {
  int scriptName = index_string("COLLIDE");
  int scriptKey = find_script_from_map(a1, scriptName);
  if (scriptKey > 0) {
    int resolution = resolve_script(scriptKey,  a1, a2, world);
    if (resolution < 0) return resolution;
  }
  return 0;
}

SDL_Rect* move(SDL_Rect* rect, int dx, int dy) {
  SDL_Rect *new = malloc(sizeof(SDL_Rect));
  new->x = rect->x + dx;
  new->y = rect->y + dy;
  new->w = rect->w;
  new->h = rect->h;
  return new;
}

int collision_check(Actor *actor, World* world) {
  ActorEntry *ae;
  DL_FOREACH(world->actors, ae) {
    if (actor->name == ae->actorKey) continue;
    Actor *actor2 = get_actor(ae->actorKey);
    if (SDL_HasIntersection(actor->ECB, actor2->ECB)) {
      int resolution = collision_with(actor, actor2, world);
      if (resolution < 0) return resolution;
      int resolution2 = collision_with(actor2, actor, world);
      if (resolution2 < 0) return resolution2;
    }
  }

  if (floor(actor->x_vel) != 0) {
    int direction = actor->x_vel < 0 ? 1 : -1;
    for (int i = 0; i < (floor(actor->x_vel) / actor->ECB->w); i++) {
      ActorEntry *ae2;
      int exit = 0;
      DL_FOREACH(world->actors, ae2) {
        if (actor->name == ae2->actorKey) continue;
        Actor *actor2 = get_actor(ae2->actorKey);
        if (!actor2->tangible) continue;
        if (SDL_HasIntersection(move(actor->ECB, actor->ECB->w * i, 0), actor2->ECB)) {
          actor->x_vel -= actor->ECB->w * i;
          exit = 1;
          break;
        }
      }
      if (exit) break;
    }

    ActorEntry *ae2;
    DL_FOREACH(world->actors, ae2) {
      if (actor->name == ae2->actorKey) continue;
      Actor *actor2 = get_actor(ae2->actorKey);
      if (!actor2->tangible) continue;
      if (SDL_HasIntersection(move(actor->ECB, actor->x_vel, 0), actor2->ECB)) {
        int resolution3 = collision_with(actor, actor2, world);
        if (resolution3 < 0) return resolution3;
        int resolution4 = collision_with(actor2, actor, world);
        if (resolution4 < 0) return resolution4;
      }
    }

    int check = 0;
    while (check == 0) {
      check = 1;
      ActorEntry *ae3;
      DL_FOREACH(world->actors, ae3) {
        if (actor->name == ae3->actorKey) continue;
        Actor *actor3 = get_actor(ae3->actorKey);
        if (!actor3->tangible) continue;
        if (SDL_HasIntersection(move(actor->ECB, actor->x_vel, 0), actor3->ECB)) {
          check = 0;
        } 
      }
      if (check == 0) {
        actor->x_vel += direction;
      }
    }
  }

  if (floor(actor->y_vel) != 0) {
    int direction = actor->y_vel < 0 ? 1 : -1;
    for (int i = 0; i < (floor(actor->y_vel) / actor->ECB->h); i++) {
      ActorEntry *ae2;
      int exit = 0;
      DL_FOREACH(world->actors, ae2) {
        if (actor->name == ae2->actorKey) continue;
        Actor *actor2 = get_actor(ae2->actorKey);
        if (!actor2->tangible) continue;
        if (SDL_HasIntersection(move(actor->ECB, 0, actor->ECB->h * i), actor2->ECB)) {
          actor->y_vel -= actor->ECB->h * i;
          exit = 1;
          break;
        }
      }
      if (exit) break;
    }

    ActorEntry *ae2;
    DL_FOREACH(world->actors, ae2) {
      if (actor->name == ae2->actorKey) continue;
      Actor *actor2 = get_actor(ae2->actorKey);
      if (!actor2->tangible) continue;
      if (SDL_HasIntersection(move(actor->ECB, 0, actor->y_vel), actor2->ECB)) {
        int resolution3 = collision_with(actor, actor2, world);
        if (resolution3 < 0) return resolution3;
        int resolution4 = collision_with(actor2, actor, world);
        if (resolution4 < 0) return resolution4;
      }
    }

    int check = 0;
    while (check == 0) {
      check = 1;
      ActorEntry *ae3;
      DL_FOREACH(world->actors, ae3) {
        if (actor->name == ae3->actorKey) continue;
        Actor *actor3 = get_actor(ae3->actorKey);
        if (!actor3->tangible) continue;
        if (SDL_HasIntersection(move(actor->ECB, 0, actor->y_vel), actor3->ECB)) {
          check = 0;
        } 
      }
      if (check == 0) {
        actor->y_vel += direction;
      }
    }
  }

  if (floor(actor->x_vel) != 0 && floor(actor->y_vel) != 0) {
    ActorEntry *ae4;

    int check = 0;
    while (check == 0) {
      check = 1;
      DL_FOREACH(world->actors, ae4) {
        if (actor->name == ae4->actorKey) continue;
        Actor *actor4 = get_actor(ae4->actorKey);
        if (SDL_HasIntersection(move(actor->ECB, actor->x_vel, actor->y_vel), actor4->ECB)) {
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

int find_script_from_map(Actor* actor, int scriptName) {
  ScriptMap *sm = get_script_map(actor->scriptmapkey);
  ScriptMapEntry *sme;

  DL_FOREACH(sm->entries, sme) {
    if (sme->state == scriptName) {
      return sme->scriptIdx;
    }
  };
  return -1;
}

int update_actor(int actorKey, int worldKey) {
  Actor *actor = get_actor(actorKey);
  if (!actor) return 0;
  
  World *world = get_world(worldKey);
  if (!world) return 0;
  if (actor->updated) {
    if ((actor->physics || actor->tangible) && world_has(world, actorKey)) {
      collision_check(actor, world);
    }
    return 0;
  }
  actor->updated = 1;
  actor->img = -1;
  
  int scriptKey = get_script_for_actor(actor);
  if (scriptKey != -1) {
    int resolution = resolve_script(scriptKey, actor, NULL, world);
    if (resolution < 0) return resolution;
  }
  float x_flag = actor->x_vel, y_flag = actor->y_vel;
  if (actor->physics || actor->tangible) {
    collision_check(actor, world);
    actor->ECB->x += floor(actor->x_vel);
    actor->ECB->y += floor(actor->y_vel);
  }

  if (x_flag != actor->x_vel && floor(actor->x_vel) == 0) {
    actor->x_vel = 0;

    int scriptName = index_string("XCOLLISION");

    int scriptKey = find_script_from_map(actor, scriptName);
    if (scriptKey != -1) {
    int resolution = resolve_script(scriptKey, actor, NULL, world);
      if (resolution < 0) return resolution;
    }
  }
  if (y_flag != actor->y_vel && floor(actor->y_vel) == 0) {
    actor->y_vel = 0;

    int scriptName = index_string("YCOLLISION");

    int scriptKey = find_script_from_map(actor, scriptName);
    if (scriptKey != -1) {
    int resolution = resolve_script(scriptKey, actor, NULL, world);
    if (resolution < 0) return resolution;
    }
  }
  
  actor->frame += 1;

  return 0;
}

int get_script_for_actor(Actor* actor) {
  ScriptMap *sm = get_script_map(actor->scriptmapkey);
  if (!sm) return -1;
  ScriptMapEntry *best, *sme;
  best = NULL;
    DL_FOREACH(sm->entries, sme) {
    if (actor->state != sme->state) continue;
    if (actor->frame < sme->frame) continue;
    if (best) {
      if (sme->frame < best->frame) continue;
    }
    best = sme;
  }
  if (!best) {
    return -1;
  }

  return best->scriptIdx;
}

Sprite* get_sprite_for_actor(Actor* actor) {
  if (actor->platform) {
    return NULL;
  }
  if (actor->img >= 0) {
    return get_sprite(actor->img);
  }

  struct SpriteMap *sm;
  sm = get_sprite_map(actor->spritemapkey);
  if (!sm) return NULL;
  SpriteMapEntry *best, *sme;
  best = NULL;
  DL_FOREACH(sm->entries, sme) {
    if (actor->state != sme->state) continue;
    if (actor->frame < sme->frame) continue;
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

void _draw_platform(SDL_Renderer* rend, Actor* actor) {
  // currently the grossest
  struct SpriteMap *sm;
  sm = get_sprite_map(actor->spritemapkey);
  if (!sm) return;

  char key00[32],key01[32],key02[32],
       key10[32],key11[32],key12[32],
       key20[32],key21[32],key22[32];
  
  sprintf(key00, "%s%i%i", get_string(actor->state), 0, 0);
  sprintf(key01, "%s%i%i", get_string(actor->state), 0, 1);
  sprintf(key02, "%s%i%i", get_string(actor->state), 0, 2);
  sprintf(key10, "%s%i%i", get_string(actor->state), 1, 0);
  sprintf(key11, "%s%i%i", get_string(actor->state), 1, 1);
  sprintf(key12, "%s%i%i", get_string(actor->state), 1, 2);
  sprintf(key20, "%s%i%i", get_string(actor->state), 2, 0);
  sprintf(key21, "%s%i%i", get_string(actor->state), 2, 1);
  sprintf(key22, "%s%i%i", get_string(actor->state), 2, 2);
  
  Sprite *s00, *s01, *s02,
         *s10, *s11, *s12,
         *s20, *s21, *s22;
  
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

  for (int y=0; y < actor->ECB->h / 32; y++) {
    for (int x=0; x < actor->ECB->w / 32; x++) {
      Sprite* img = NULL;
      if      (x == 0                  && y == 0                 ) img = s00;
      else if (x == actor->ECB->w/32-1 && y == 0                 ) img = s02;
      else if (x == 0                  && y == actor->ECB->h/32-1) img = s20;
      else if (x == actor->ECB->w/32-1 && y == actor->ECB->h/32-1) img = s22;
      else if (x == 0                                            ) img = s10;
      else if (x == actor->ECB->w/32-1                           ) img = s12;
      else if (                           y == 0                 ) img = s01;
      else if (                           y == actor->ECB->h/32-1) img = s21;
      else img = s11;

      if (img == NULL) {
	continue;
      }
      SDL_Rect dest, src;
      dest.x = actor->ECB->x + x*32;
      dest.y = actor->ECB->y + y*32;
      src.x = 0;
      src.y = 0;
      
      SDL_QueryTexture(img->image, NULL, NULL, &dest.w, &dest.h);
      SDL_QueryTexture(img->image, NULL, NULL, &src.w, &src.h);
      
      SDL_RenderCopy(rend, img->image, &src, &dest);
    }
  }
}

void draw_actor(SDL_Renderer* rend, Actor* actor, const char* frameKey) {
  if (actor->platform) {
    return _draw_platform(rend, actor);
  }

  Sprite *s;
  s = get_sprite_for_actor(actor);
  if (s == NULL) return;
  Frame *f;
  f = get_frame(frameKey);
  if (f == NULL) return;
  
  SDL_Rect dest, src;
  dest.x = actor->ECB->x + s->offx - f->scroll_x;
  dest.y = actor->ECB->y + s->offy - f->scroll_y;
  src.x = 0;
  src.y = 0;
  
  SDL_QueryTexture(s->image, NULL, NULL, &dest.w, &dest.h);
  src.w = dest.w;
  src.h = dest.h;
  
  SDL_RenderCopy(rend, s->image, &src, &dest);
}
