import pygame
from pygame import Surface

from copy import deepcopy

IMG_LOCATION = "img/"

SPRITES = {}
OFFSETS = {}
SPRITEMAPS = {}
TILEMAPS = {}

def swap_in(offsets=None, sprite_maps=None, tile_maps=None):
    global OFFSETS, SPRITEMAPS, TILEMAPS
    load()
    OFFSET = {}
    SPRITEMAPS = {}
    
    if offsets is not None:
        OFFSETS = deepcopy(offsets)
    if sprite_maps is not None:
        SPRITEMAPS = deepcopy(sprite_maps)
    if tile_maps is not None:
        TILEMAPS = deepcopy(tile_maps)
        
def load():
    global SPRITEMAPS, OFFSETS, TILEMAPS
    from src.lib import SPRITESHEETS
    for filename in SPRITESHEETS.SPRITESHEETS:
        data = SPRITESHEETS.SPRITESHEETS[filename]
        _load_spritesheet(filename, data)
    SPRITEMAPS = SPRITESHEETS.SPRITEMAPS
    OFFSETS = SPRITESHEETS.OFFSETS
    TILEMAPS = SPRITESHEETS.TILEMAPS

def _load_spritesheet(filename, data, colorkey=(1, 255, 1)):
    """data should be dict with key: ((x, y), (w, h)), assumes w, h are 32, 32"""
    surf = pygame.image.load(IMG_LOCATION+filename).convert()
    for name in data:
        sprite = Surface(data[name][1])
        x, y = 0 - data[name][0][0], 0 - data[name][0][1]
        sprite.blit(surf, (x, y))
        sprite.set_colorkey(colorkey)
        SPRITES[name] = sprite
    return SPRITES

def set_sprite(name, surf):
    SPRITES[name] = surf
    
def get_sprites():
    return list(SPRITES.keys())

def get_sprite(name):
    return None if name not in SPRITES else SPRITES[name]

def get_sprite_map(name):
    return None if name not in SPRITEMAPS else SPRITEMAPS[name]

def get_offset(key, name):
    if key not in OFFSETS:
        return (0, 0)
    return (0, 0) if name not in OFFSETS[key] else OFFSETS[key][name]

def get_tile_map(key):
    return None if key not in TILEMAPS else TILEMAPS[key]

