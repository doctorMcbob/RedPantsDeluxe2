import pygame
from pygame.locals import *

import sys

from copy import deepcopy

STICKS = []
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
    "A"           : 0,
    "B"           : 3,
    "X"           : 2,
    "Y"           : 1,
    "START"       : 10,
    "HAT"         : 0,
    "HORIZ_STICK" : 0,
    "VERT_STICK"  : 1,
}

MAPPED_CONTROLLERS = {
    "Sony Interactive Entertainment Wireless Controller": deepcopy(DEFAULT_CONTROLLER_MAP),
    "Xbox Series X Controller": {
        "A"      : 0,
        "B"      : 2,
        "X"      : 1,
        "Y"      : 3,
            "START"  : 7,

        "HAT"         : 0,
        "HORIZ_STICK" : 0,
        "VERT_STICK"  : 1,
    },
    "Xbox One S Controller": {
        "A"      : 0,
        "B"      : 3,
        "X"      : 1,
        "Y"      : 4,
        "START"  : 7,

        "HAT"         : 0,
        "HORIZ_STICK" : 0,
        "VERT_STICK"  : 1,
    },
    "Controller (Xbox for windows)": {
        "A"      : 0,
        "B"      : 2,
        "X"      : 1,
        "Y"      : 3,
        "START"  : 7,

        "HAT"         : 0,
        "HORIZ_STICK" : 0,
        "VERT_STICK"  : 1,
    },
    "Controller (Xbox for Windows)": {
        "A"      : 0,
        "B"      : 2,
        "X"      : 1,
        "Y"      : 3,
        "START"  : 7,

        "HAT"         : 0,
        "HORIZ_STICK" : 0,
        "VERT_STICK"  : 1,
    },
    "Nintendo Co., Ltd. Pro Controller": {
        "A"      : 0,
        "B"      : 2,
        "X"      : 1,
        "Y"      : 3,
        "START"  : 7,

        "HAT"         : 0,
        "HORIZ_STICK" : 0,
        "VERT_STICK"  : 1,
    }
}

def add_state(name, inp_map=DEFAULT_MAP, joy=None):
    STATES[name] = deepcopy(STATE_TEMPLATE)
    if joy is not None:
        STATES[name]["JOY"] = joy
    KEY_MAPS[name] = deepcopy(inp_map)

def get_state(name):
    return STATES[name] if name in STATES else None

def get_state_keys():
    return list(STATES.keys())

def set_state(name, state):
    STATES[name] = state

def set_defaults():
    if "PLAYER2" in STATES:
        STATES.pop("PLAYER2")
    add_state("PLAYER1")
    
def update_sticks():
    global STICKS
    STICKS = [pygame.joystick.Joystick(x) for x in range(min(pygame.joystick.get_count(), 4))]
    pygame.event.pump()
    for joy in STICKS:
        if not joy.get_init():
            joy.init()

def get_num_sticks():
    return len(STICKS)

def add_joystick_to_input_state(name, stick):
    if (stick > len(STICKS)):
        return
    joy = STICKS[stick]
    if joy.get_name() in MAPPED_CONTROLLERS:
        controller_map = deepcopy(MAPPED_CONTROLLERS[joy.get_name()])
    else:
        controller_map = deepcopy(DEFAULT_CONTROLLER_MAP)

    add_state(
        name,
        inp_map=controller_map,
        joy=joy)

def update_tas(TAS, frame, noquit=False):
    for state_name in TAS.keys():
        if state_name in STATES and frame in TAS[state_name]:
            STATES[state_name]["EVENTS"] = TAS[state_name][frame]
        else:
            STATES[state_name]["EVENTS"] = []

        for event in STATES[state_name]["EVENTS"]:
            if event == "A_UP": STATES[state_name]["A"] = 0
            elif event == "A_DOWN": STATES[state_name]["A"] = 1
            elif event == "B_UP": STATES[state_name]["B"] = 0
            elif event == "B_DOWN": STATES[state_name]["B"] = 1
            elif event == "X_UP": STATES[state_name]["X"] = 0
            elif event == "X_DOWN": STATES[state_name]["X"] = 1
            elif event == "Y_UP": STATES[state_name]["Y"] = 0
            elif event == "Y_DOWN": STATES[state_name]["Y"] = 1
            elif event == "UP_UP": STATES[state_name]["UP"] = 0
            elif event == "UP_DOWN": STATES[state_name]["UP"] = 1
            elif event == "DOWN_UP": STATES[state_name]["DOWN"] = 0
            elif event == "DOWN_DOWN": STATES[state_name]["DOWN"] = 1
            elif event == "LEFT_UP": STATES[state_name]["LEFT"] = 0
            elif event == "LEFT_DOWN": STATES[state_name]["LEFT"] = 1
            elif event == "RIGHT_UP": STATES[state_name]["RIGHT"] = 0
            elif event == "RIGHT_DOWN": STATES[state_name]["RIGHT"] = 1
            if event == "QUIT":
                return "QUIT" if noquit else sys.exit()
            
def update(noquit=False):
    for name in STATES.keys():
        STATES[name]["EVENTS"] = []

    for e in pygame.event.get():
        for name in STATES.keys():
            state = STATES[name]
            inp_map = KEY_MAPS[name]
        
            if e.type == QUIT:
                return "QUIT" if noquit else sys.exit()

            elif e.type == KEYDOWN:
                if e.key == K_PERIOD: state["EVENTS"].append("CLIP")
                if e.key == K_BACKQUOTE: state["EVENTS"].append("CONSOLEDEBUG")
                if e.key == K_ESCAPE: return "QUIT" if noquit else sys.exit()
                for key in inp_map:
                    if e.key == inp_map[key]:
                        state[key] = 1
                        state["EVENTS"].append("{}_DOWN".format(key))

            elif e.type == KEYUP:
                for key in inp_map:
                    if e.key == inp_map[key]:
                        state[key] = 0
                        state["EVENTS"].append("{}_UP".format(key))

            elif e.type == JOYBUTTONDOWN and state["JOY"] is not None and e.instance_id == state["JOY"].get_instance_id():
                for key in inp_map:
                    if e.button == inp_map[key]:
                        state[key] = 1
                        state["EVENTS"].append("{}_DOWN".format(key))

            elif e.type == JOYBUTTONUP and state["JOY"] is not None and e.instance_id == state["JOY"].get_instance_id():
                for key in inp_map:
                    if e.button == inp_map[key]:
                        state[key] = 0
                        state["EVENTS"].append("{}_UP".format(key))

            elif e.type == JOYHATMOTION and state["JOY"] is not None and e.instance_id == state["JOY"].get_instance_id():
                dx, dy = state["JOY"].get_hat(e.hat)

                if dy == 1:
                    if state["UP"] == 0:
                        state["EVENTS"].append("UP_DOWN")
                    state["UP"] = 1
                if dy == -1:
                    if state["DOWN"] == 0:
                        state["EVENTS"].append("DOWN_DOWN")
                    state["DOWN"] = 1

                if state["UP"] == 1 and dy != 1:
                    state["EVENTS"].append("UP_UP")
                    state["UP"] = 0
                if state["DOWN"] == 1 and dy != -1:
                    state["EVENTS"].append("DOWN_UP")
                    state["DOWN"] = 0

                if dx == 1:
                    if state["RIGHT"] == 0:
                        state["EVENTS"].append("RIGHT_DOWN")
                    state["RIGHT"] = 1
                if dx == -1:
                    if state["LEFT"] == 0:
                        state["EVENTS"].append("LEFT_DOWN")
                    state["LEFT"] = 1

                if state["RIGHT"] == 1 and dx != 1:
                    state["EVENTS"].append("RIGHT_UP")
                    state["RIGHT"] = 0
                if state["LEFT"] == 1 and dx != -1:
                    state["EVENTS"].append("LEFT_UP")
                    state["LEFT"] = 0
