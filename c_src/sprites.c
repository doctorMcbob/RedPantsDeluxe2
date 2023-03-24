/**
   The sprites module

its really a dictionary of name -> sprite (SDL_Texture)

Using the implementation of a dictionary like hash `uthash.h`
https://github.com/troydhanson/uthash/blob/master/src/uthash.h
and the similar imlpementation of a doubly linked list `utlist.h`
https://github.com/troydhanson/uthash/blob/master/src/utlist.h
*/

# include "uthash.h"
# include "utlist.h"
# include "sprites.h"

# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>

Sprite* sprites = NULL;
SpriteMap* spritemaps = NULL;


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

void add_sprite_map(const char* name) {
  struct SpriteMap *sm;
  sm = malloc(sizeof(SpriteMap));
  if (sm == NULL) {
    exit(-1);
  }
  strcpy(sm->name, name);
  sm->entries = NULL;
  HASH_ADD_STR(spritemaps, name, sm);
}

SpriteMap* get_sprite_map(const char* name) {
  struct SpriteMap *sm;

  HASH_FIND_STR(spritemaps, name, sm);
  if (sm) {
    return sm;
  }
  return NULL;
}

void add_to_sprite_map(const char* name, const char* state, int frame, const char* spriteKey) {
  struct SpriteMap *sm;
  sm = get_sprite_map(name);
  if (sm == NULL) {
    printf("No sprite map %s\n", name);
    return;
  }
  
  struct SpriteMapEntry *sme;
  sme = malloc(sizeof(SpriteMapEntry));
  if (sme == NULL) {
    exit(-1);
  }
  strcpy(sme->state, state);
  sme->frame = frame;
  strcpy(sme->spriteKey, spriteKey);

  DL_APPEND(sm->entries, sme);
}

void add_sprite(const char* name, SDL_Texture* image) {
  struct Sprite *s;
  s = malloc(sizeof(Sprite));
  if (s == NULL) {
    exit(-1);
  }
  strcpy(s->name, name);
  s->image = image;
  HASH_ADD_STR(sprites, name, s);
}

Sprite* get_sprite(const char* name) {
  struct Sprite *s;

  HASH_FIND_STR(sprites, name, s);
  if (s) {
    return s;
  } else {
    return NULL;
  }
}

void add_offset(const char* name, int x, int y) {
  struct Sprite *s;
  s = get_sprite(name);
  if (s) {
    s->offx = x;
    s->offy = y;
  }
}

SDL_Surface* load_image(const char* filename) {
  SDL_Surface* image = IMG_Load(filename);
  if (!image) {
    return NULL;
  }
  return image;
}

void load_spritesheet(SDL_Renderer* rend,
		      const char* filename,
		      const char* names[],
		      int xs[],
		      int ys[],
		      int ws[],
		      int hs[],
		      int count) {

  SDL_Surface* spritesheet = load_image(filename);
  if (!spritesheet)
    {
      printf("Could not load %s\n", filename);
      return;
    }

  SDL_SetColorKey(spritesheet, SDL_TRUE, SDL_MapRGB(spritesheet->format, 1, 255, 1));
  for (int i = 0; i < count; i++) {
    if (ws[i] == 0) {
      ws[i] = 1;
    }
    if (hs[i] == 0) {
      hs[i] = 1;
    }
    
    SDL_Surface* sprite = SDL_CreateRGBSurfaceWithFormat(0, ws[i], hs[i], 32, spritesheet->format->format);
    SDL_Rect src;
    SDL_Rect dst;
    src.x = xs[i];
    src.y = ys[i];
    src.w = ws[i];
    src.h = hs[i];
    dst.x = 0;
    dst.y = 0;
    dst.w = sprite->w;
    dst.h = sprite->h;
    
    SDL_BlitSurface(spritesheet, &src, sprite, &dst);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, sprite);

    add_sprite(names[i], texture);

    SDL_FreeSurface(sprite);
  }

  SDL_FreeSurface(spritesheet);
}

