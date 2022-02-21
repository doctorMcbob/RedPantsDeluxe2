import pygame
from pygame.locals import *

import sys

STATE = {
    "JOY"    : None,
    "UP"     : 0,
    "DOWN"   : 0,
    "LEFT"   : 0,
    "RIGHT"  : 0,
    "A"      : 0,
    "B"      : 0,
    "EVENTS" : [],
}

DEFAULT_KEY_MAP = {
    "UP"     : K_UP,
    "DOWN"   : K_DOWN,
    "LEFT"   : K_LEFT,
    "RIGHT"  : K_RIGHT,
    "A"      : K_z,
    "B"      : K_x,
}

def update(inp_map=DEFAULT_KEY_MAP, state=STATE):
    for e in pygame.event.get():
        if e.type == QUIT: sys.exit()
        if e.type == KEYDOWN:
            if e.key == K_ESCAPE: sys.exit()
            for key in inp_map:
                if e.key == inp_map[key]:
                    state[key] = 1
                    state["EVENTS"].append("{} DOWN".format(key))
        if e.type == KEYUP:
            for key in inp_map:
                if e.key == inp_map[key]:
                    state[key] = 0
                    state["EVENTS"].append("{} UP".format(key))

        if e.type == JOYBUTTONDOWN and state["JOY"] is not None:
            for key in inp_map:
                if e.button == inp_map[key]:
                    state[key] = 1
                    state["EVENTS"].append("{} DOWN".format(key))
        if e.type == JOYBUTTONUP and state["JOY"] is not None:
            for key in inp_map:
                if e.button == inp_map[key]:
                    state[key] = 1
                    state["EVENTS"].append("{} UP".format(key))

    if state["JOY"] is not None:
        joy = state["JOY"]
        for idx in range(joy.get_num_axes()):
            dx, dy = joy.get_axis(idx)
            if dy == 1:
                if state["UP"] == 0:
                    state["EVENTS"].append("UP DOWN")
                state["UP"] = 1
            if dy == -1:
                if state["DOWN"] == 0:
                    state["EVENTS"].append("DOWN DOWN")
                state["DOWN"] = 1
            if state["UP"] == 1 and dy != 1:
                state["EVENTS"].append("UP UP")
                state["UP"] = 0
            if state["DOWN"] == 1 and dy != 1:
                state["EVENTS"].append("DOWN UP")
                state["DOWN"] = 0

            if dx == 1:
                if state["RIGHT"] == 0:
                    state["EVENTS"].append("RIGHT DOWN")
                state["RIGHT"] = 1
            if dy == -1:
                if state["LEFT"] == 0:
                    state["EVENTS"].append("LEFT DOWN")
                state["DOWN"] = 1
            if state["RIGHT"] == 1 and dy != 1:
                state["EVENTS"].append("RIGHT UP")
                state["RIGHT"] = 0
            if state["LEFT"] == 1 and dy != 1:
                state["EVENTS"].append("LEFT UP")
                state["LEFT"] = 0
