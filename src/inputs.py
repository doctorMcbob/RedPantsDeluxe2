import pygame
from pygame.locals import *

import sys

from copy import deepcopy

STATES = {}
STATE_TEMPLATE = {
    "JOY"    : None,
    "UP"     : 0,
    "DOWN"   : 0,
    "LEFT"   : 0,
    "RIGHT"  : 0,
    "A"      : 0,
    "B"      : 0,
    "X"      : 0,
    "Y"      : 0,
    "EVENTS" : [],
}

KEY_MAPS = {}
DEFAULT_MAP = {
    "UP"     : K_UP,
    "DOWN"   : K_DOWN,
    "LEFT"   : K_LEFT,
    "RIGHT"  : K_RIGHT,
    "START"  : K_RETURN,
    "A"      : K_z,
    "B"      : K_x,
    "X"      : K_a,
    "Y"      : K_s,
}
DEFAULT_CONTROLLER_MAP = {
    "A"      : 0,
    "B"      : 1,
    "X"      : 2,
    "Y"      : 3,
    "START"  : 10
}

def add_state(name, inp_map=DEFAULT_MAP, joy=None):
    STATES[name] = deepcopy(STATE_TEMPLATE)
    if joy is not None:
        STATES[name]["JOY"] = joy
    KEY_MAPS[name] = deepcopy(inp_map)

def get_state(name):
    return STATES[name] if name in STATES else None

def update(noquit=False):
    for name in STATES.keys():
        STATES[name]["EVENTS"] = []

    for e in pygame.event.get():
        for name in STATES.keys():
            state = STATES[name]
            inp_map = KEY_MAPS[name]
        
            if e.type == QUIT:
                return "QUIT" if noquit else sys.exit()

            if e.type == KEYDOWN:
                if e.key == K_PERIOD: state["EVENTS"].append("CLIP")
                if e.key == K_ESCAPE: return "QUIT" if noquit else sys.exit()
                for key in inp_map:
                    if e.key == inp_map[key]:
                        state[key] = 1
                        state["EVENTS"].append("{}_DOWN".format(key))

            if e.type == KEYUP:
                for key in inp_map:
                    if e.key == inp_map[key]:
                        state[key] = 0
                        state["EVENTS"].append("{}_UP".format(key))

            if e.type == JOYBUTTONDOWN and state["JOY"] is not None and e.instance_id == state["JOY"].get_instance_id():
                for key in inp_map:
                    if e.button == inp_map[key]:
                        state[key] = 1
                        state["EVENTS"].append("{}_DOWN".format(key))

            if e.type == JOYBUTTONUP and state["JOY"] is not None and e.instance_id == state["JOY"].get_instance_id():
                for key in inp_map:
                    if e.button == inp_map[key]:
                        state[key] = 0
                        state["EVENTS"].append("{}_UP".format(key))

            if e.type == JOYHATMOTION and state["JOY"] is not None and e.instance_id == state["JOY"].get_instance_id():
                dx, dy = state["JOY"].get_hat(e.hat)

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
                if state["DOWN"] == 1 and dy != -1:
                    state["EVENTS"].append("DOWN UP")
                    state["DOWN"] = 0

                if dx == 1:
                    if state["RIGHT"] == 0:
                        state["EVENTS"].append("RIGHT DOWN")
                    state["RIGHT"] = 1
                if dx == -1:
                    if state["LEFT"] == 0:
                        state["EVENTS"].append("LEFT DOWN")
                    state["LEFT"] = 1

                if state["RIGHT"] == 1 and dx != 1:
                    state["EVENTS"].append("RIGHT UP")
                    state["RIGHT"] = 0
                if state["LEFT"] == 1 and dx != -1:
                    state["EVENTS"].append("LEFT UP")
                    state["LEFT"] = 0
