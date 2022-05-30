import pygame
from pygame import Surface, Rect

from src import worlds

FRAMES = {}

def clear():
    global FRAMES
    FRAMES = {}

def add_frame(name, world, size, position=(0, 0), focus=None):
    FRAMES[name] = Frame(world, size, position, focus)

def get_frame(name):
    return FRAMES[name] if name in FRAMES else None

class Frame(object):
    # focus should be an actor if not None
    def __init__(self, world, size, position=(0, 0), focus=None):
        self.scroll_x = position[0]
        self.scroll_y = position[1]

        self.w, self.h = size

        self.world = worlds.get_world(world)

        self.focus = focus # can changed toggled with scripts
        
    def in_frame(self, actor):
        return Rect((self.scroll_x, self.scroll_y), (self.w, self.h)).colliderect(actor)

    def scroll(self, position):
        return (position[0] - self.scroll_x, position[1] - self.scroll_y)

    def drawn(self, DEBUG=False):
        resize = self.w < 800 and self.h < 500
        if resize:
            self.w *= 2
            self.h *= 2
            self.update()
        
        surf = Surface((self.w, self.h))
        self.world.draw(surf, self, DEBUG=DEBUG)
        if resize:
            self.w /= 2
            self.h /= 2
            return pygame.transform.smoothscale(surf, (int(self.w), int(self.h)))
        return surf

    def update(self):
        if self.focus is not None:
            self.scroll_x = self.focus.x - self.w // 2
            self.scroll_y = self.focus.y - self.h // 2
        # world x_lock and y_lock are values that override the frames focus
        # ie, no scroll worlds, scroll x only or scroll y only worlds
        if self.world.x_lock is not None:
            self.scroll_x = self.world.x_lock
        if self.world.y_lock is not None:
            self.scroll_y = self.world.y_lock

