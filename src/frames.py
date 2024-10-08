import sys

import pygame
from pygame import Surface, Rect

from src import worlds

FRAMES = {}

def clear():
    global FRAMES
    FRAMES = {}

def add_frame(name, world, size, pos=(0, 0), position=(0, 0), focus=None):
    FRAMES[name] = Frame(world, size, pos, position, focus)
    return FRAMES[name]

def get_frame(name):
    return FRAMES[name] if name in FRAMES else None

def get_frames():
    return list(FRAMES.values())

def delete_frame(name):
    return FRAMES.pop(name)

class Frame(object):
    # focus should be an actor if not None
    def __init__(self, world, size, pos=(0, 0), scrollpos=(0, 0), focus=None):
        self.scroll_x = scrollpos[0]
        self.scroll_y = scrollpos[1]
        self.pos = pos
        self.w, self.h = size

        self.world = worlds.get_world(world)

        self.should_zoom = 0
        self.zoom_scale = 1.0
        self.focus = focus # can changed toggled with scripts
        self.scrollbound = {
            "left": None,
            "top": None, 
            "right": None, 
            "bottom": None,
        }
        self.active = True
        
    def in_frame(self, actor):
        return Rect((self.scroll_x, self.scroll_y), (self.w, self.h)).colliderect(actor)

    def scroll(self, position):
        return (position[0] - self.scroll_x, position[1] - self.scroll_y)

    def drawn(self, DEBUG=False):
        if self.should_zoom:
            orig_w = self.w
            orig_h = self.h
            self.w *= self.zoom_scale
            self.h *= self.zoom_scale
            self.update()
        
        surf = Surface((self.w, self.h))
        self.world.draw_bg(surf, self)
        if "-e" not in sys.argv:
            for world in worlds.get_worlds():
                if world == self.world: continue
                if world.flagged_for_update:
                    world.draw(surf, self, DEBUG=False)
        self.world.draw(surf, self, DEBUG=DEBUG)
        if self.should_zoom:
            self.w = orig_w
            self.h = orig_h
            return pygame.transform.smoothscale(surf, (int(self.w), int(self.h)))
        return surf

    def update(self):
        if self.focus is not None:
            self.scroll_x = self.focus.x + (self.focus.w // 2) - self.w // 2
            self.scroll_y = self.focus.y + (self.focus.h // 2) - self.h // 2

        if self.scrollbound["left"] is not None:
            if self.scroll_x < self.scrollbound["left"]:
                self.scroll_x = self.scrollbound["left"]

        if self.scrollbound["right"] is not None:
            if self.scroll_x + self.w > self.scrollbound["right"]:
                self.scroll_x = self.scrollbound["right"] - self.w

        if self.scrollbound["top"] is not None:
            if self.scroll_y < self.scrollbound["top"]:
                self.scroll_y = self.scrollbound["top"]

        if self.scrollbound["bottom"] is not None:
            if self.scroll_y + self.h > self.scrollbound["bottom"]:
                self.scroll_y = self.scrollbound["bottom"] - self.h
