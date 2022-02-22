"""
A world represents a room in the game. 

A world will have actors
"""
import pygame
from pygame import Rect

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
            Actor = actor.get_actor(name)
            Actor.update(self)

    def get_actors(self):
        return [actor.get_actor(a) for a in self.actors]

    def draw(self, dest, DEBUG=False):
        if self.background is not None:
            for y in range((dest.get_height() // background.get_height())):
                for x in range((dest.get_width() // background.get_width())):
                    dest.blit(background, (x*background.get_width(), y*background.get_height()))
        else:
            dest.fill((185, 185, 185))
        
        for name in self.actors:
            Actor = actor.get_actor(name)
            dx, dy = Actor.spriteoffset
            if self._is_near(Actor, dest.get_size()):
                dest.blit(Actor.get_sprite(), self._scroll((Actor.x+dx, Actor.y+dy)))

            if DEBUG: Actor.debug(DEBUG)
    def _scroll(self, pos):
        return pos[0] - self.scrollx, pos[1] - self.scrolly

    def _is_near(self, Actor, dimensions):
        return Rect((self.scrollx, self.scrolly), dimensions).colliderect(Actor)

