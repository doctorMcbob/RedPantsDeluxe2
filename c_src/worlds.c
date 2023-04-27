# include "actors.h"
# include "worlds.h"
# include "frames.h"
# include "sprites.h"
# include "utlist.h"
# include "uthash.h"
# include "stringmachine.h"
#include <SDL2/SDL_ttf.h> // for debug purposes only and can be removed from final build

TTF_Font* font;
World* worlds = NULL;

void add_world(int name,
	       int background,
	       int x_lock,
	       int y_lock) {
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

World* get_world(int name) {
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
    printf("World %s not found when adding actor %s\n", get_string(worldkey), get_string(actorname));
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
  struct ActorEntry *ae, *tmp;
  DL_FOREACH_SAFE(w->actors, ae, tmp) {
    if (update_actor(ae->actorKey, worldKey, debug) == -2) return -2;
  }
  return 0;
}

void _draw_background(World* world, SDL_Renderer* rend) {
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

  for (int y = 0; y < h/dest.h+2; y++) {
    dest.y = y*dest.h;
    for (int x = 0; x < w/dest.w+2; x++) {
      dest.x = x*dest.w;
      // TODO: offset by frame background offset value
      SDL_RenderCopy(rend, background->image, &src, &dest);
    }
  }
}

void draw_world(World* world, SDL_Renderer* rend, Frame* frame) {
  _draw_background(world, rend);

  struct ActorEntry *ae;
  DL_FOREACH(world->actors, ae) {
    Actor* a;
    a = get_actor(ae->actorKey);
    
    if (!a) continue;
    if (in_frame(frame, a))
      draw_actor(rend, a, frame);
  }
};

void draw_debug_overlay(World* world, SDL_Renderer* rend, Frame* frame) {
  struct ActorEntry *ae;
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  int hasTextDrawn = 0;
  DL_FOREACH(world->actors, ae) {
    Actor* a;
    a = get_actor(ae->actorKey);
    SDL_Rect *ECB = a->ECB;
    ECB->x += frame->scroll_x;
    ECB->y += frame->scroll_y;
    SDL_SetRenderDrawColor(rend, 0, 0, 255, 255);
    SDL_RenderDrawRect(rend, ECB);

    if (hasTextDrawn) continue;
    if (mouseX >= ECB->x && mouseX < ECB->x + ECB->w && mouseY >= ECB->y && mouseY < ECB->y + ECB->h) {
      char data[100]; // buffer to hold the formatted string
      sprintf(data, "%s %s:%iT?%i",
              get_string(ae->actorKey), get_string(a->state), a->frame, a->tangible);

      SDL_Color textColor = { 0, 0, 0 };
      SDL_Surface* surface = TTF_RenderText_Solid(font, data, textColor);

      SDL_Texture* Message = SDL_CreateTextureFromSurface(rend, surface);
      SDL_Rect Message_rect = { mouseX, mouseY, surface->w, surface->h };
      SDL_RenderCopy(rend, Message, NULL, &Message_rect);

      SDL_FreeSurface(surface);
      SDL_DestroyTexture(Message);
      hasTextDrawn = 1;
    }
  }
}

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
    if (world_has(w, actorKey)) return 1;
  }
  return 0;
}

void remove_actor_from_worlds(int actorKey) {
  struct World *w, *tmp;
  HASH_ITER(hh, worlds, w, tmp) {
    ActorEntry *ae, *tmp2;
    DL_FOREACH_SAFE(w->actors, ae, tmp2) {
      if (ae->actorKey == actorKey) {
        DL_DELETE(w->actors, ae);
        free(ae);
      }
    }
  }
}
