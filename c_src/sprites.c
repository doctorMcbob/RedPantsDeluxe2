/**
   The sprites module

its really a dictionary of name -> sprite (SDL_Texture)

Using the implementation of a dictionary like hash `uthash.h`
https://github.com/troydhanson/uthash/blob/master/src/uthash.h
and the similar imlpementation of a doubly linked list `utlist.h`
https://github.com/troydhanson/uthash/blob/master/src/utlist.h
*/

#include "sprites.h"
#include "stringmachine.h"
#include "uthash.h"
#include "utlist.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Sprite *sprites = NULL;
SpriteMap *spritemaps = NULL;

void sprites_taredown() {
  struct Sprite *s, *tmp;
  HASH_ITER(hh, sprites, s, tmp) {
    HASH_DEL(sprites, s);
    SDL_DestroyTexture(s->image);
    free(s);
  }
  struct SpriteMap *sm, *tmp2;
  struct SpriteMapEntry *sme, *tmp3;
  HASH_ITER(hh, spritemaps, sm, tmp2) {
    DL_FOREACH_SAFE(sm->entries, sme, tmp3) {
      DL_DELETE(sm->entries, sme);
      free(sme);
    }
    HASH_DEL(spritemaps, sm);
    free(sm);
  }
}

void add_sprite_map(int name) {
  struct SpriteMap *sm;
  sm = malloc(sizeof(SpriteMap));
  if (sm == NULL) {
    exit(-1);
  }
  sm->name = name;
  sm->entries = NULL;
  HASH_ADD_INT(spritemaps, name, sm);
}

SpriteMap *get_sprite_map(int name) {
  struct SpriteMap *sm;

  HASH_FIND_INT(spritemaps, &name, sm);
  if (sm) {
    return sm;
  }
  return NULL;
}

void add_to_sprite_map(int name, int state, int frame, int spriteKey) {
  struct SpriteMap *sm;
  sm = get_sprite_map(name);
  if (sm == NULL) {
    printf("No sprite map %s\n", get_string(name));
    return;
  }

  struct SpriteMapEntry *sme;
  sme = malloc(sizeof(SpriteMapEntry));
  if (sme == NULL) {
    exit(-1);
  }
  sme->state = state;
  sme->frame = frame;
  sme->spriteKey = spriteKey;

  DL_APPEND(sm->entries, sme);
}

void add_sprite(int name, SDL_Texture *image) {
  struct Sprite *s;
  s = malloc(sizeof(Sprite));
  if (s == NULL) {
    exit(-1);
  }
  s->name = name;
  s->image = image;
  s->offx = 0;
  s->offy = 0;
  HASH_ADD_INT(sprites, name, s);
}

Sprite *get_sprite(int name) {
  struct Sprite *s;
  HASH_FIND_INT(sprites, &name, s);
  if (s) {
    return s;
  } else {
    return NULL;
  }
}

void add_offset(int name, int x, int y) {
  struct Sprite *s;
  s = get_sprite(name);
  if (s) {
    s->offx = x;
    s->offy = y;
  }
}

void load_sprite(int name, const unsigned char sprite[][4], int w, int h,
                 SDL_Renderer *rend) {
  SDL_Surface *surface =
      SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
  Uint32 *pixels = (Uint32 *)surface->pixels;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      Uint32 pixel = SDL_MapRGBA(surface->format, sprite[y * w + x][0],
                                 sprite[y * w + x][1], sprite[y * w + x][2],
                                 sprite[y * w + x][3]);
      pixels[y * w + x] = pixel;
    }
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(rend, surface);
  add_sprite(name, texture);

  SDL_FreeSurface(surface);
}

