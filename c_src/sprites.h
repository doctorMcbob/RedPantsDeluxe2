# include "uthash.h"
# include "utlist.h"
# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>

#ifndef SPRITES_DEF
#define SPRITES_DEF 1

typedef struct Sprite {
  int name;
  SDL_Texture* image;
  int offx;
  int offy;
  UT_hash_handle hh;
} Sprite;

typedef struct SpriteMapEntry {
  int state;
  int frame;
  int spriteKey;
  struct SpriteMapEntry* next;
  struct SpriteMapEntry* prev;
} SpriteMapEntry;

typedef struct SpriteMap {
  int name;
  struct SpriteMapEntry* entries;
  UT_hash_handle hh;
} SpriteMap;

void add_sprite(int name, SDL_Texture* image);
Sprite* get_sprite(int name);
void add_offset(int name, int x, int y);
SDL_Surface* load_image(const char* filename);
void load_spritesheet(SDL_Renderer* rend,
		      const char* filename,
		      int names[],
		      int xs[],
		      int ys[],
		      int ws[],
		      int hs[],
		      int count);
void add_sprite_map(int name);
SpriteMap* get_sprite_map(int name);
void add_to_sprite_map(int name, int state, int frame, int spriteKey);
void sprites_taredown();
#endif
