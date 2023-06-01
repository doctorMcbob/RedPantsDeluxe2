#include "boxes.h"
#include "stringmachine.h"
#include "uthash.h"
#include "utlist.h"
#include <SDL2/SDL.h>

BoxMap *hitboxes = NULL;
BoxMap *hurtboxes = NULL;

void add_hitbox_map(int name) {
  struct BoxMap *hbm;
  hbm = malloc(sizeof(BoxMap));
  if (hbm == NULL) {
    exit(-1);
  }
  hbm->name = name;
  hbm->entries = NULL;
  HASH_ADD_INT(hitboxes, name, hbm);
}

void add_to_hitbox_map(int name, int state, int frame, int x[], int y[],
                       int w[], int h[], int count) {
  struct BoxMap *hbm;
  hbm = get_hitbox_map(name);
  if (hbm == NULL) {
    printf("No hitbox BoxMap %s\n", get_string(name));
    return;
  }

  struct BoxMapEntry *bme;
  bme = malloc(sizeof(BoxMapEntry) + count * sizeof(SDL_Rect));
  if (bme == NULL) {
    exit(-1);
  }

  bme->state = state;
  bme->frame = frame;
  bme->next = NULL;
  bme->prev = NULL;

  for (int i = 0; i < count; i++) {
    SDL_Rect rect;
    rect.x = x[i];
    rect.y = y[i];
    rect.w = w[i];
    rect.h = h[i];
    bme->rect[i] = rect;
  }
  bme->count = count;
  DL_APPEND(hbm->entries, bme);
}

BoxMap *get_hitbox_map(int name) {
  struct BoxMap *hbm;

  HASH_FIND_INT(hitboxes, &name, hbm);
  if (hbm) {
    return hbm;
  }
  return NULL;
}

void add_hurtbox_map(int name) {
  struct BoxMap *hbm;
  hbm = malloc(sizeof(BoxMap));
  if (hbm == NULL) {
    exit(-1);
  }
  hbm->name = name;
  hbm->entries = NULL;
  HASH_ADD_INT(hurtboxes, name, hbm);
}

void add_to_hurtbox_map(int name, int state, int frame, int x[], int y[],
                        int w[], int h[], int count) {
  struct BoxMap *hbm;
  hbm = get_hurtbox_map(name);
  if (hbm == NULL) {
    printf("No hurtbox BoxMap %s\n", get_string(name));
    return;
  }

  struct BoxMapEntry *bme;
  bme = malloc(sizeof(BoxMapEntry) + count * sizeof(SDL_Rect));
  if (bme == NULL) {
    exit(-1);
  }

  bme->state = state;
  bme->frame = frame;
  bme->next = NULL;
  bme->prev = NULL;
  for (int i = 0; i < count; i++) {
    SDL_Rect rect;
    rect.x = x[i];
    rect.y = y[i];
    rect.w = w[i];
    rect.h = h[i];
    bme->rect[i] = rect;
  }
  bme->count = count;

  DL_APPEND(hbm->entries, bme);
}

BoxMap *get_hurtbox_map(int name) {
  struct BoxMap *hbm;

  HASH_FIND_INT(hurtboxes, &name, hbm);
  if (hbm) {
    return hbm;
  }
  return NULL;
}
