"""
A world represents a room in the game. 

A world will have actors
"""
import pygame

from src import sprites
from src import actor

root = "root"

worlds = {}

def load():
    from src.lib import WORLDS as W

    for name in W.WORLDS.keys():
        worlds[name] = World(W.WORLDS[name])

def get_world(world):
    return worlds[world]

class World(object):
    def __init__(self, template):
        self.background = None if template["background"] is None else sprites.get_sprite(template["background"])
        self.actors = template["actors"]
        self.scrollx, self.scrolly = (0, 0)
        
    def update(self):
        for name in self.actors:
            actor = actors.get_actor(name)
            actor.update(self)
            
    def draw(self, dest):
        if self.background is not None:
            for y in range((dest.get_height() // background.get_height())):
                for x in range((dest.get_width() // background.get_width())):
                    dest.blit(background, (x*background.get_width(), y*background.get_height()))
                
        for name in self.actors:
            actor = actors.get_actor(name)
            if _is_near(actor, dest.get_size()):
                dest.blit(actor.get_sprite(), _scroll((actor.x, actor.y)))

    def _scroll(self, pos):
        return pos[0] - self.scrollx, pos[1] - self.scrolly

    def _is_near(self, actor, dimensions):
        width, height = dimensions
        return (
            actor.x + actor.w < self.scrollx or actor.y + actor.h < self.scrolly or
            actor.x > self.scrollx + dimensions[0] or actor.y > self.scrolly + dimensions[1]
        )

