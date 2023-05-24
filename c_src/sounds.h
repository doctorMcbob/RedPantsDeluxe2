#include "stringmachine.h"
#include "uthash.h"

#ifndef SOUNDS_H
#define SOUNDS_H

typedef struct soundKey {
    int name;
    int key;
    UT_hash_handle hh;
} soundKey;

void add_sound(int name, int key);
void teardown_sound_map();
void tear_down_sounds();
void play_sound(int name);
void add_song_data(int key, const void *wavData, int size);
void add_song(int name, int key);
void teardown_song_map();
int get_song();
void play_song();
void disable_music();
void disable_sfx();
#endif
