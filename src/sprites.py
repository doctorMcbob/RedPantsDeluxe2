import pygame
from pygame import Surface

IMG_LOCATION = "img/"

SPRITES = {}

def load():
    from src.lib import SPRITESHEETS
    for filename in SPRITESHEETS.SPRITESHEETS:
        data = SPRITESHEETS.SPRITESHEETS[filename]
        _load_spritesheet(filename, data)
    
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

def get_sprite(name):
    return None if name not in SPRITES else SPRITES[name]
