import pygame
from pygame.mixer import Sound

import os

SOUND_LOCATION = "audio/"

SOUNDS = {}
SONGS = {}
SONG = None

def load():
    filenames = []
    for _,_, files in os.walk(SOUND_LOCATION):
        for f in files:
            if f[-4:] == ".ogg":
                SOUNDS[f[:-4]] = Sound(SOUND_LOCATION + "/" + f)
            elif f[-4:] == ".mp3":
                SONGS[f[:-4]] = SOUND_LOCATION + "/" + f

def get_songs():
    return SONGS

def get_sounds():
    return SOUNDS

def get_song():
    return SONG

def play_sound(sound):
    if sound in SOUNDS:
        SOUNDS[sound].stop()
        SOUNDS[sound].play()

def play_song(song):
    global SONG

    if song in SONGS:
        if song == SONG:
            return
        SONG = song
        pygame.mixer.music.load(SONGS[song])
        pygame.mixer.music.play(-1)

def stop_sounds():
    for key in SOUNDS:
        stop_sound(key)

def stop_sound(sound):
    SOUNDS[sound].stop()

def stop_song():
    pygame.mixer.music.stop()
