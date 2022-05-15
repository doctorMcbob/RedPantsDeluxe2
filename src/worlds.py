"""
A world represents a room in the game. 

A world will have actors
"""
import pygame
from pygame import Rect

from src import sprites
from src import actor

root = "demostart"

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
            if frame.in_frame(Actor):
                dx, dy = Actor.get_offset()
                if Actor.direction == 1 and not Actor.rotation in [270, 90]:
                    sprite = Actor.get_sprite()
                    dest.blit(sprite, frame.scroll((
                        (Actor.x+Actor.w-dx)-sprite.get_width(), Actor.y+dy)))
                else:
                    dest.blit(Actor.get_sprite(), frame.scroll((Actor.x+dx, Actor.y+dy)))
        if DEBUG:
            for name in self.actors:
                Actor = actor.get_actor(name)
                if frame.in_frame(Actor):
                    maketext = Rect(frame.scroll((Actor.x, Actor.y)), Actor.size).collidepoint(pygame.mouse.get_pos())

                    Actor.debug(dest,
                                frame.scroll((Actor.x+Actor.w, Actor.y)),
                                DEBUG["HEL16"],
                                frame.scroll_x, frame.scroll_y,
                                text=maketext
                    )


