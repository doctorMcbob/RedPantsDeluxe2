#include "sounds.h"
#include "audiodata.h"
#include "stringmachine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

soundKey *SOUND_MAP = NULL;
soundKey *SONG_MAP = NULL;
int CURRENT_SONG = -1;
int CHANNEL = 0;
int SONGS_ON = 1;
int SFX_ON = 1;

int get_song() { return CURRENT_SONG; }

void add_song_data(int key, const void *wavData, int size) {
  SDL_RWops *rw = SDL_RWFromConstMem(wavData, size);
  SONGS[key] = Mix_LoadMUS_RW(rw, 1);
}

void add_song(int name, int key) {
  soundKey *s = malloc(sizeof(soundKey));
  s->name = name;
  s->key = key;
  HASH_ADD_INT(SONG_MAP, name, s);
}

void add_sound(int name, int key) {
  soundKey *s = malloc(sizeof(soundKey));
  s->name = name;
  s->key = key;
  HASH_ADD_INT(SOUND_MAP, name, s);
}

void teardown_sound_map() {
  soundKey *current_sound, *tmp;
  HASH_ITER(hh, SOUND_MAP, current_sound, tmp) {
    HASH_DEL(SOUND_MAP, current_sound);
    free(current_sound);
  }
}

void teardown_song_map() {
  soundKey *current_sound, *tmp;
  HASH_ITER(hh, SONG_MAP, current_sound, tmp) {
    HASH_DEL(SONG_MAP, current_sound);
    free(current_sound);
  }
}

void tear_down_sounds() {
  for (int i = 0; i < NUM_SOUNDS; i++) {
    if (SOUNDS[i] != NULL) {
      Mix_FreeChunk(SOUNDS[i]);
    }
  }
}

void play_sound(int name) {
  if (!SFX_ON)
    return;
  soundKey *s;
  HASH_FIND_INT(SOUND_MAP, &name, s);
  if (s == NULL) {
    printf("Sound not found: %s\n", get_string(name));
    return;
  }
  if (SOUNDS[s->key] == NULL) {
    printf("Sound not loaded: %s : %s\n", get_string(name), Mix_GetError());
    return;
  }
  Mix_HaltChannel(CHANNEL);
  if (Mix_PlayChannel(CHANNEL, SOUNDS[s->key], 0) == -1) {
    printf("Error playing sound %s at %i : %s\n", get_string(name), s->key,
           Mix_GetError());
  };
  if (CHANNEL++ == 16) {
    CHANNEL = 0;
  }
}

void play_song(int name) {
  if (!SONGS_ON)
    return;
  if (name == CURRENT_SONG)
    return;
  soundKey *s;
  HASH_FIND_INT(SONG_MAP, &name, s);
  if (s == NULL) {
    printf("Song not found: %s\n", get_string(name));
    return;
  }
  if (SONGS[s->key] == NULL) {
    printf("Song not loaded: %s : %s\n", get_string(name), Mix_GetError());
    return;
  }
  if (Mix_PlayMusic(SONGS[s->key], -1) == -1) {
    printf("Error playing song %s at %i : %s\n", get_string(name), s->key,
           Mix_GetError());
  };
  CURRENT_SONG = name;
}

void disable_music() {
  SONGS_ON = 0;
  Mix_HaltMusic();
}

void disable_sfx() { SFX_ON = 0; }
