# include "uthash.h"
# include "utlist.h"
# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>

#ifndef SPRITES_DEF
#define SPRITES_DEF 1

typedef struct Sprite {
  char name[32];
  SDL_Texture* image;
  int offx;
  int offy;
  UT_hash_handle hh;
} Sprite;

typedef struct SpriteMapEntry {
  char state[32];
  int frame;
  char spriteKey[32];
  struct SpriteMapEntry* next;
  struct SpriteMapEntry* prev;
} SpriteMapEntry;

typedef struct SpriteMap {
  char name[32];
  struct SpriteMapEntry* entries;
  UT_hash_handle hh;
} SpriteMap;

void add_sprite(const char* name, SDL_Texture* image);
Sprite* get_sprite(const char* name);
void add_offset(const char* name, int x, int y);
SDL_Surface* load_image(const char* filename);
void load_spritesheet(SDL_Renderer* rend,
		      const char* filename,
		      const char* names[],
		      int xs[],
		      int ys[],
		      int ws[],
		      int hs[],
		      int count);
void add_sprite_map(const char* name);
SpriteMap* get_sprite_map(const char* name);
void add_to_sprite_map(const char* name, const char* state, int frame, const char* spriteKey);
void sprites_taredown();
#endif
