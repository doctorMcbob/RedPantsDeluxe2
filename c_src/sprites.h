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

typedef struct TileMap {
  int sprites[9];
} TileMap;

void add_sprite(int name, SDL_Texture* image);
Sprite* get_sprite(int name);
void add_offset(int name, int x, int y);
SDL_Surface* load_image(const char* filename);
void add_sprite_map(int name);
SpriteMap* get_sprite_map(int name);
void add_to_sprite_map(int name, int state, int frame, int spriteKey);
void sprites_taredown();
void load_sprite(int name, const unsigned char sprite[][4], int w, int h, SDL_Renderer* rend);
void add_tile_map(int name,
		  int sprite00, int sprite01, int sprite02,
		  int sprite10, int sprite11, int sprite12,
		  int sprite20, int sprite21, int sprite22);
TileMap *get_tile_map(int name);
#endif
