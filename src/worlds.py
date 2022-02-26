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
        self.exits = {} if "exits" not in template else template["exits"]
        self.locks = {} if "locks" not in template else template["locks"]
        
    def update(self):
        for name in self.actors:
            Actor = actor.get_actor(name)
            Actor.update(self)

    def get_actors(self):
        return [actor.get_actor(a) for a in self.actors]

    def get_exits(self):
        return list(self.exits.keys())

    def get_locks(self):
        return list(self.exits.keys())

    def draw(self, dest, frame, DEBUG=False):
        if self.background is not None:
            for y in range((dest.get_height() // self.background.get_height())+2):
                y =  y*self.background.get_height()
                y -= (frame.scroll_y // 2) % self.background.get_height()
                for x in range((dest.get_width() // self.background.get_width())+2):
                    x = x * self.background.get_width()
                    x -= (frame.scroll_x // 2) % self.background.get_width()

                    dest.blit(self.background, (x, y))
        else:
            dest.fill((185, 185, 185))
        
        for name in self.actors:
            Actor = actor.get_actor(name)
            dx, dy = Actor.spriteoffset
            if frame.in_frame(Actor):
                dest.blit(Actor.get_sprite(), frame.scroll((Actor.x+dx, Actor.y+dy)))

            if DEBUG: Actor.debug(DEBUG)


