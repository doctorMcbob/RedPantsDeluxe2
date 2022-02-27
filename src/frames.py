import pygame
from pygame import Surface, Rect

FRAMES = {}

def clear():
    global FRAMES
    FRAMES = {}

def add_frame(name, world, size, position=(0, 0), focus=None):
    FRAMES[name] = Frame(world, size, position, focus)

def get_frame(name):
    return FRAMES[name]

class Frame(object):
    # focus should be an actor if not None
    def __init__(self, world, size, position=(0, 0), focus=None):
        self.scroll_x = position[0]
        self.scroll_y = position[1]

        self.w, self.h = size

        self.world = world

        self.focus = focus # can changed toggled with scripts
        
    def in_frame(self, actor):
        return Rect((self.scroll_x, self.scroll_y), (self.w, self.h)).colliderect(actor)

    def scroll(self, position):
        return (position[0] - self.scroll_x, position[1] - self.scroll_y)

    def drawn(self, DEBUG=False):
        surf = Surface((self.w, self.h))
        self.world.draw(surf, self, DEBUG=DEBUG)
        return surf

    def update(self):
        if self.focus is not None:
            self.scroll_x = self.focus.x - self.w // 2
            self.scroll_y = self.focus.y - self.h // 2

