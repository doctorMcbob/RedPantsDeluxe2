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
        worlds[name] = World(w[name])

def load():
    from src.lib import WORLDS as W

    for name in list(worlds.keys()):
        worlds.pop(name)

    for name in W.WORLDS.keys():
        worlds[name] = World(W.WORLDS[name])

def get_world(world):
    return worlds[world]

def get_all_worlds():
    return list(worlds.keys())

def get_worlds():
    return list(worlds.values())

class World(object):
    def __init__(self, template):
        self.name = "" if "name" not in template else template["name"]
        self.background = None if template["background"] is None else sprites.get_sprite(template["background"])
        self.background_tag = template["background"]
        self.actors = deepcopy(template["actors"])
        self.x_lock = None if "x_lock" not in template else template["x_lock"]
        self.y_lock = None if "y_lock" not in template else template["y_lock"]
        self.flagged_for_update = True if "flagged_for_update" not in template else template["flagged_for_update"]
        self.background_xscroll = 0
        self.background_yscroll = 0
        
    def as_template(self):
        return {
            "name": self.name,
            "background": self.background_tag,
            "actors": self.actors,
            "x_lock": self.x_lock,
            "y_lock": self.y_lock,
            "backgroundscroll_x": self.background_xscroll,
            "backgroundscroll_y": self.background_yscroll,
            "flagged_for_update": self.flagged_for_update,
        }
        
    def update(self):
        for name in self.actors:
            Actor = actor.get_actor(name)
            if Actor is None:
                continue
            if not Actor.updated:
                Actor.update(self)

    def get_actors(self):
        # weird but fixes editor bug
        # old version:
        # return [actor.get_actor(a) for a in self.actors]
        ret = []
        removal = []
        for a in self.actors:
            a_ = actor.get_actor(a)
            if a_ is None:
                removal.append(a)
            else:
                ret.append(a_)
        for a in removal:
            self.actors.remove(a)
        return ret

    def draw(self, dest, frame, DEBUG=False):
        if self.background is not None:
            blitz = []
            for y in range((dest.get_height() // self.background.get_height())+2):
                y =  y*self.background.get_height()
                y -= (frame.scroll_y // 2 + self.background_yscroll) % self.background.get_height()
                for x in range((dest.get_width() // self.background.get_width())+2):
                    x = x * self.background.get_width()
                    x -= (frame.scroll_x // 2 + self.background_xscroll) % self.background.get_width()
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
                if Actor.direction == 1 and not Actor.rotation in [270, 90, -90, -270]:
                    sprite = Actor.get_sprite()
                    blitz.append((sprite, frame.scroll((
                        (Actor.x+Actor.w-dx)-sprite.get_width(), Actor.y+dy)
                    )))
                else:
                    blitz.append((Actor.get_sprite(), frame.scroll((Actor.x+dx, Actor.y+dy))))
            if DEBUG:
                for blit in blitz:
                    pygame.draw.rect(dest, (155, 0, 155), Rect(blit[1], blit[0].get_size()), 1)
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
                    ).collidepoint(mpos)
                    maketext = maketext and mpos[1] < frame.h and pygame.mouse.get_focused()

                    Actor.debug(dest,
                                frame.scroll((Actor.x+Actor.w, Actor.y)),
                                DEBUG["HEL16"],
                                frame.scroll_x, frame.scroll_y,
                                text=maketext
                    )

