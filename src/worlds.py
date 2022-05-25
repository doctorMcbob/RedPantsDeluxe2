"""
A world represents a room in the game. 

A world will have actors
"""
import pygame
from pygame import Rect

from copy import deepcopy

from src import sprites
from src import actor

root = "root"

worlds = {}

def swap_in(w):
    global worlds
    worlds = {}

    for name in w.keys():
        worlds[name] = World(deepcopy(w[name]))

def load():
    from src.lib import WORLDS as W

    for name in W.WORLDS.keys():
        worlds[name] = World(W.WORLDS[name])

def get_world(world):
    return worlds[world]

def get_all_worlds():
    return list(worlds.keys())

class World(object):
    def __init__(self, template):
        
        self.background = None if template["background"] is None else sprites.get_sprite(template["background"])
        self.actors = template["actors"]
        self.x_lock = None if "x_lock" not in template else template["x_lock"]
        self.y_lock = None if "y_lock" not in template else template["y_lock"]
        
    def update(self):
        for name in self.actors:
            Actor = actor.get_actor(name)
            Actor.update(self)

    def get_actors(self):
        return [actor.get_actor(a) for a in self.actors]

    def draw(self, dest, frame, DEBUG=False):
        if self.background is not None:
            blitz = []
            for y in range((dest.get_height() // self.background.get_height())+2):
                y =  y*self.background.get_height()
                y -= (frame.scroll_y // 2) % self.background.get_height()
                for x in range((dest.get_width() // self.background.get_width())+2):
                    x = x * self.background.get_width()
                    x -= (frame.scroll_x // 2) % self.background.get_width()

                    blitz.append((self.background, (x, y)))
            dest.blits(blitz)
        else:
            dest.fill((185, 185, 185))
        
        for name in self.actors:
            Actor = actor.get_actor(name)
            if Actor is None: continue

            blitz = []
            if frame.in_frame(Actor):
                dx, dy = Actor.get_offset()
                if Actor.direction == 1 and not Actor.rotation in [270, 90]:
                    sprite = Actor.get_sprite()
                    blitz.append((sprite, frame.scroll((
                        (Actor.x+Actor.w-dx)-sprite.get_width(), Actor.y+dy)
                    )))
                else:
                    blitz.append((Actor.get_sprite(), frame.scroll((Actor.x+dx, Actor.y+dy))))
            dest.blits(blitz)
        if DEBUG:
            for name in self.actors:
                Actor = actor.get_actor(name)
                if Actor is None: continue
                if frame.in_frame(Actor):
                    mpos = pygame.mouse.get_pos()
                    maketext = Rect(
                        frame.scroll((Actor.x, Actor.y)),
                        Actor.size
                    ).collidepoint(mpos) and mpos[1] < frame.h

                    Actor.debug(dest,
                                frame.scroll((Actor.x+Actor.w, Actor.y)),
                                DEBUG["HEL16"],
                                frame.scroll_x, frame.scroll_y,
                                text=maketext
                    )

