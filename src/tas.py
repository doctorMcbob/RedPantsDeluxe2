import pygame

import sys
import os

from src import inputs
from src.utils import get_text_input, expect_input
from src import menu
from src import scripts
from src import sprites
from src import worlds
from src import frames
from src import actor
from src import printer
from src import scripts
from src import boxes

from copy import deepcopy
from pathlib import Path

ROOT_PATH = Path('.')
PATH_TO_TASES = ROOT_PATH / ("tas/" if "-o" not in sys.argv else "tas/" + sys.argv[sys.argv.index("-o") + 1])

SAVE_STATES = {}
HUD_LOCATION = (100, 100)
CLICK_LOCK = False

if not os.path.isdir(PATH_TO_TASES): os.mkdir(PATH_TO_TASES)

def set_up():
    pygame.init()
    W = 1152 if "-w" not in sys.argv else int(sys.argv[sys.argv.index("-w")+1])
    H = 640 if "-h" not in sys.argv else int(sys.argv[sys.argv.index("-h")+1])
    G = {"W":W,"H":H}
    G["SCREEN"] = pygame.display.set_mode((W, H)) if "-f" not in sys.argv else pygame.display.set_mode((W, H), pygame.FULLSCREEN)
    pygame.display.set_caption("Red Pants Deluxe 2")
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["SCREEN"].fill((255, 255, 255))
    G["SCREEN"].blit(G["HEL32"].render("Loading...", 0, (0, 0, 0)), (0, 0))
    pygame.display.update()
    G["PRINTER"] = printer
    G["WORLDS"] = worlds
    G["FRAMES"] = frames
    G["SCRIPTS"] = scripts
    G["ACTOR"] = actor
    G["ROOT"] = worlds.root if "-r" not in sys.argv else sys.argv[sys.argv.index("-r")+1]

    sprites.load()
    scripts.load()
    worlds.load()
    actor.load()
    boxes.load()

    G["INPUTS"] = inputs
    G["FRAMES"] = frames
    menu.run_controller_menu(G)

    G["CLOCK"] = pygame.time.Clock()
    G["DEBUG"] = "-d" in sys.argv

    G["TAS"] = {} if "-tas" not in sys.argv else get_tas(sys.argv[sys.argv.index("-tas")+1])
    if not G["TAS"]:
        for key in inputs.get_state_keys():
            G["TAS"][key] = {}
    G["FRAME"] = 0

    return G

def get_tas(filename):
    with open(PATH_TO_TASES / filename) as f:
        tas = f.read()

    TAS = {}
    # frame:input_name|events
    # 1:PLAYER1=LEFT_UP,B_DOWN
    for line in tas.splitlines():
        frame, data = line.split(":")
        state_name, events = data.split("=")
        if state_name not in TAS:
            TAS[state_name] = {}
        TAS[state_name][int(frame)] = events.split(",")

    return TAS

def save_tas(filename, TAS):
    s = ""
    for state_name in TAS.keys():
        for frame in TAS[state_name].keys():
            events = TAS[state_name][frame]
            s += "{}:{}={}\n".format(frame, state_name, ",".join(events))

    with open(PATH_TO_TASES / filename, "w+") as f:
        f.write(s)

def save_state(G):
    STATE = {
        "ACTORS": {},
        "WORLDS": {},
        "INPUT_STATES": {}
    }
    for key in inputs.get_state_keys():
        STATE["INPUT_STATES"][key] = deepcopy(inputs.get_state(key))
    
    for a in actor.get_actors():
        STATE["ACTORS"][a.name] = a.as_template()
    for w in worlds.get_worlds():
        STATE["WORLDS"][w.name] = w.as_template()

    SAVE_STATES[G["FRAME"]] = STATE

def load_last_state(G):
    frame = G["FRAME"]
    closest = -1
    biggest = -1
    for f in SAVE_STATES.keys():
        biggest = f if f > biggest else biggest
        if f > frame:
            continue
        if f < frame and frame > closest:
            closest = f

    if closest == -1 and biggest != -1:
        return load_save_state(G, biggest)
    return load_save_state(G, closest) if closest != -1 else None

def load_save_state(G, frame):
    save_state = SAVE_STATES[frame]
    G["FRAME"] = frame
    favorites = {}
    for f in frames.get_frames():
        if f.focus:
            favorites[f.focus.name] = f
            
    actor.swap_in(save_state["ACTORS"])
    worlds.swap_in(save_state["WORLDS"])
    for key in save_state["INPUT_STATES"].keys():
        inputs.set_state(key, save_state["INPUT_STATES"][key])
    
    for name in favorites.keys():
        f = favorites[name]
        for a in f.world.actors:
            if name == a:
                f.focus = actor.get_actor(a)

def draw(G):
    blitz = []
    for name in G["FRAMEMAP"]:
        frame = G["FRAMES"].get_frame(name)
        position = G["FRAMEMAP"][name]
        drawn = frame.drawn(DEBUG=G) if "DEBUG" in G and G["DEBUG"] else frame.drawn()
        blitz.append((drawn, position))
    G["SCREEN"].blits(blitz)
        
def input_callback(G):
    global HUD_LOCATION, CLICK_LOCK
    clicks = pygame.mouse.get_pressed()
    if not clicks[0]: CLICK_LOCK = False
    if clicks[-1]:
        HUD_LOCATION = pygame.mouse.get_pos()
    draw(G)
    update_input_deck(G, HUD_LOCATION, "PLAYER1")
    draw_input_deck(G, HUD_LOCATION, "PLAYER1")
    inputs.update_tas(G["TAS"], G["FRAME"])

def update_input_deck(G, pos, input_state_key):
    global CLICK_LOCK
    if CLICK_LOCK: return
    mpos = pygame.mouse.get_pos()
    clicks = pygame.mouse.get_pressed()
    input_state = inputs.get_state(input_state_key)
    tas_events = G["TAS"][input_state_key][G["FRAME"]] if G["FRAME"] in G["TAS"][input_state_key] else []
    x, y = pos
    CLICK_LOCK = clicks[0]
    
    if pygame.Rect((x+32, y+64), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("LEFT_UP" if input_state["LEFT"] else "LEFT_DOWN") not in tas_events:
            tas_events.append("LEFT_UP" if input_state["LEFT"] else "LEFT_DOWN")
        else:
            tas_events.remove("LEFT_UP" if input_state["LEFT"] else "LEFT_DOWN")
    if pygame.Rect((x+64, y+64), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("RIGHT_UP" if input_state["RIGHT"] else "RIGHT_DOWN") not in tas_events:
            tas_events.append("RIGHT_UP" if input_state["RIGHT"] else "RIGHT_DOWN")
        else:
            tas_events.remove("RIGHT_UP" if input_state["RIGHT"] else "RIGHT_DOWN")
    if pygame.Rect((x+48, y+48), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("UP_UP" if input_state["UP"] else "UP_DOWN") not in tas_events:
            tas_events.append("UP_UP" if input_state["UP"] else "UP_DOWN")
        else:
            tas_events.remove("UP_UP" if input_state["UP"] else "UP_DOWN")
    if pygame.Rect((x+48, y+80), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("DOWN_UP" if input_state["DOWN"] else "DOWN_DOWN") not in tas_events:
            tas_events.append("DOWN_UP" if input_state["DOWN"] else "DOWN_DOWN")
        else:
            tas_events.remove("DOWN_UP" if input_state["DOWN"] else "DOWN_DOWN")

    if pygame.Rect((x+32+80, y+64), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("A_UP" if input_state["A"] else "A_DOWN") not in tas_events:
            tas_events.append("A_UP" if input_state["A"] else "A_DOWN")
        else:
            tas_events.remove("A_UP" if input_state["A"] else "A_DOWN")
    if pygame.Rect((x+64+80, y+64), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("X_UP" if input_state["X"] else "X_DOWN") not in tas_events:
            tas_events.append("X_UP" if input_state["X"] else "X_DOWN")
        else:
            tas_events.append("X_UP" if input_state["X"] else "X_DOWN")
    if pygame.Rect((x+48+80, y+48), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("B_UP" if input_state["B"] else "B_DOWN") not in tas_events:
            tas_events.append("B_UP" if input_state["B"] else "B_DOWN")
        else:
            tas_events.remove("B_UP" if input_state["B"] else "B_DOWN")
    if pygame.Rect((x+48+80, y+80), (16, 16)).collidepoint(mpos) and clicks[0]:
        if ("Y_UP" if input_state["Y"] else "Y_DOWN") not in tas_events:
            tas_events.append("Y_UP" if input_state["Y"] else "Y_DOWN")
        else:
            tas_events.remove("Y_UP" if input_state["Y"] else "Y_DOWN")

    if tas_events and G["FRAME"] not in G["TAS"][input_state_key]:
        G["TAS"][input_state_key][G["FRAME"]] = tas_events
    
    
def draw_input_deck(G, pos, input_state_key):
    input_state = inputs.get_state(input_state_key)
    pygame.draw.rect(G["SCREEN"], (255, 255, 255), pygame.Rect(pos, (256, 128)))
    x, y = pos
    G["SCREEN"].blit(G["HEL16"].render("frame: {}".format(G["FRAME"]), 0, (0,0,0)), (x, y))
    G["SCREEN"].blit(G["HEL16"].render("TAS: {}".format(None if G["FRAME"] not in G["TAS"]["PLAYER1"] else G["TAS"]["PLAYER1"][G["FRAME"]]), 0, (0,0,0)), (x, y+16))
    
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["LEFT"] else (200, 200, 200), pygame.Rect((x+32, y+64), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("L", 0, (0,0,0)), (x+32, y+64))
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["RIGHT"] else (200, 200, 200), pygame.Rect((x+64, y+64), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("R", 0, (0,0,0)), (x+64, y+64))
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["UP"] else (200, 200, 200), pygame.Rect((x+48, y+48), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("U", 0, (0,0,0)), (x+48, y+48))
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["DOWN"] else (200, 200, 200), pygame.Rect((x+48, y+80), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("D", 0, (0,0,0)), (x+48, y+80))
    
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["A"] else (200, 200, 200), pygame.Rect((x+32+80, y+64), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("A", 0, (0,0,0)), (x+32+80, y+64))
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["X"] else (200, 200, 200), pygame.Rect((x+64+80, y+64), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("X", 0, (0,0,0)), (x+64+80, y+64))
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["B"] else (200, 200, 200), pygame.Rect((x+48+80, y+48), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("B", 0, (0,0,0)), (x+48+80, y+48))
    pygame.draw.rect(G["SCREEN"], (250, 50, 50) if input_state["Y"] else (200, 200, 200), pygame.Rect((x+48+80, y+80), (16, 16)))
    G["SCREEN"].blit(G["HEL16"].render("Y", 0, (0,0,0)), (x+48+80, y+80))
    
def tas_er(G, noquit=False):
    while True:
        inp = expect_input(cb=input_callback, args=G)
        mods = pygame.key.get_mods()
        if inp == pygame.QUIT or (mods & pygame.KMOD_SHIFT and inp == pygame.K_ESCAPE):
            return "QUIT" if noquit else sys.exit()

        if inp == pygame.K_BACKQUOTE:
            execute_console_command(G)
            
        if inp == pygame.K_BACKSPACE:
            load_last_state(G)

        if inp == pygame.K_RETURN:
            save_state(G)
            
        if inp == pygame.K_SPACE:
            return

        if inp == pygame.K_s and mods & pygame.KMOD_CTRL:
            filename = get_text_input(G, (0, 0))
            if filename is not None:
                save_tas(filename, G["TAS"])
        
def run(G, noquit=False):
    while True:
        exits = False
        if "-play" not in sys.argv:
            exits = tas_er(G) == "QUIT"
    
        if inputs.update_tas(G["TAS"], G["FRAME"], noquit) == "QUIT" or exits:
            return
        G["FRAME"] += 1

        worlds_for_updating = [G["FRAMES"].get_frame(name).world for name in G["FRAMEMAP"]]

        for world in G["WORLDS"].get_worlds():

            if world not in worlds_for_updating and world.flagged_for_update:
                worlds_for_updating.append(world)

        for world in worlds_for_updating:
            world.flagged_for_update = False
            world.update()

        for actor in  G["ACTOR"].get_actors():
            actor.updated = False

        blitz = []
        for name in G["FRAMEMAP"]:
            frame = G["FRAMES"].get_frame(name)
            frame.update()
            position = G["FRAMEMAP"][name]
            drawn = frame.drawn(DEBUG=G) if "DEBUG" in G and G["DEBUG"] else frame.drawn()
            blitz.append((drawn, position))

        G["SCREEN"].blits(blitz)

        pygame.display.update()
        if "PRINTER" in G:
            G["PRINTER"].save_surface(G["SCREEN"])
            if any(["CLIP" in inputs.get_state(state)["EVENTS"] for state in inputs.STATES]):
                G["PRINTER"].save_em()
                G["PRINTER"].make_gif()
                G["PRINTER"].clear_em()

        G["CLOCK"].tick(30)


def execute_console_command(G):
    cmd = get_text_input(G, (0, G["H"] - 32))
    if cmd is None: return
    if "REFERENCE" not in G:
        G["REFERENCE"] = "player10"

    if cmd.startswith("REF:"):
        G["REFERENCE"] = cmd.split(":")[-1]
    else:
        scripts.resolve(G["REFERENCE"], cmd.splitlines(), G["FRAMES"].get_frame(list(G["FRAMEMAP"].keys())[0]).world)

