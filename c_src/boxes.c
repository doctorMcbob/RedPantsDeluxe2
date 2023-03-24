# include "uthash.h"
# include "utlist.h"
# include <SDL2/SDL.h>
# include "boxes.h"

BoxMap* hitboxes = NULL;
BoxMap* hurtboxes = NULL;


void add_hitbox_map(const char* name) {
  struct BoxMap* hbm;
  hbm = malloc(sizeof(BoxMap));
  if (hbm == NULL) {
    exit(-1);
  }
  strcpy(hbm->name, name);
  hbm->entries = NULL;
  HASH_ADD_STR(hitboxes, name, hbm);
}

void add_to_hitbox_map(const char* name,
		       const char* state,
		       int frame,
		       int x[],
		       int y[],
		       int w[],
		       int h[], 
		       int count) {
  struct BoxMap* hbm;
  hbm = get_hitbox_map(name);
  if (hbm == NULL) {
    printf("No hitbox BoxMap %s\n", name);
    return;
  }

  struct BoxMapEntry* bme;
  bme = malloc(sizeof(BoxMapEntry) + count * sizeof(SDL_Rect));
  if (bme == NULL) {
    exit(-1);
  }

  strcpy(bme->state, state);
  bme->frame = frame;
  bme->next = NULL;
  bme->prev = NULL;
  
  for (int i = 0; i < count; i++) {
    SDL_Rect rect;
    rect.x = x[count];
    rect.y = y[count];
    rect.w = w[count];
    rect.h = h[count];
    bme->rect[i] = rect;
  }

  DL_APPEND(hbm->entries, bme);
}

BoxMap* get_hitbox_map(const char* name) {
  struct BoxMap *hbm;

  HASH_FIND_STR(hitboxes, name, hbm);
  if (hbm) {
    return hbm;
  }
  return NULL;
}

void add_hurtbox_map(const char* name) {
  struct BoxMap* hbm;
  hbm = malloc(sizeof(BoxMap));
  if (hbm == NULL) {
    exit(-1);
  }
  strcpy(hbm->name, name);
  hbm->entries = NULL;
  HASH_ADD_STR(hurtboxes, name, hbm);
}
void add_to_hurtbox_map(const char* name,
			const char* state,
			int frame,
			int x[],
			int y[],
			int w[],
			int h[],
			int count) {
  struct BoxMap* hbm;
  hbm = get_hurtbox_map(name);
  if (hbm == NULL) {
    printf("No hurtbox BoxMap %s\n", name);
    return;
  }

  struct BoxMapEntry* bme;
  bme = malloc(sizeof(BoxMapEntry) + count * sizeof(SDL_Rect));
  if (bme == NULL) {
    exit(-1);
  }

  strcpy(bme->state, state);
  bme->frame = frame;
  bme->next = NULL;
  bme->prev = NULL;
  
  for (int i = 0; i < count; i++) {
    SDL_Rect rect;
    rect.x = x[count];
    rect.y = y[count];
    rect.w = w[count];
    rect.h = h[count];
    bme->rect[i] = rect;
  }

  DL_APPEND(hbm->entries, bme);
}
  
BoxMap* get_hurtbox_map(const char* name) {
  struct BoxMap *hbm;

  HASH_FIND_STR(hurtboxes, name, hbm);
  if (hbm) {
    return hbm;
  }
  return NULL;
}
