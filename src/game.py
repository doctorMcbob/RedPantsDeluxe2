import pygame

import sys
import os
import random

from src import inputs
from src.utils import get_text_input
from src import scripts
from src import sprites
from src import worlds
from src import frames
from src import actor
from src import printer
from src import scripts
from src import boxes
from src import sounds

def load():
    actor.load()
    worlds.load()

def set_up():
    random.seed()
    pygame.init()
    pygame.mixer.init()
    W = 1152 if "-w" not in sys.argv else int(sys.argv[sys.argv.index("-w")+1])
    H = 640 if "-h" not in sys.argv else int(sys.argv[sys.argv.index("-h")+1])
    G = {"W":W,"H":H}
    G["SCREEN"] = pygame.display.set_mode((W, H)) if "-f" not in sys.argv else pygame.display.set_mode((W, H), pygame.FULLSCREEN)
    pygame.display.set_caption("Red Pants")
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["SCREEN"].fill((255, 255, 255))
    G["SCREEN"].blit(G["HEL32"].render("Loading...", 0, (0, 0, 0)), (0, 0))
    pygame.display.update()
    G["ROOT"] = worlds.root if "-r" not in sys.argv else sys.argv[sys.argv.index("-r")+1]
    sprites.load()
    scripts.load()
    worlds.load()
    actor.load()
    boxes.load()
    sounds.load()
    if sprites.get_sprite("icon"):
        pygame.display.set_icon(
            sprites.get_sprite("icon"))

    inputs.set_defaults()    
    frames.add_frame("ROOT", G["ROOT"], (G["W"], G["H"]), pos=(0, 0))
    pygame.mixer.music.set_volume(0.30)
    G["CLOCK"] = pygame.time.Clock()
    G["DEBUG"] = "-d" in sys.argv
    return G

def run(G, noquit=False):
    while True:
        if inputs.update(noquit) == "QUIT":
            return

        worlds_for_updating = [
            frame.world for frame in filter(
                lambda f: f.active, frames.get_frames()
            )
        ]

        for world in worlds.get_worlds():

            if world not in worlds_for_updating and world.flagged_for_update:
                worlds_for_updating.append(world)

        for world in worlds_for_updating:
            world.flagged_for_update = False
            world.update()

        for a in  actor.get_actors():
            a.updated = False

        blitz = []
        for frame in frames.get_frames():
            if not frame.active: 
                continue
            frame.update()
            position = frame.pos
            drawn = frame.drawn(DEBUG=G) if "DEBUG" in G and G["DEBUG"] else frame.drawn()
            blitz.append((drawn, position))

        G["SCREEN"].blits(blitz)

         # maybe remove FPS counter before release, dont worry about it for a while though
        fps = G["CLOCK"].get_fps()
        msg = ""
        if fps < 15:
            msg = 'very significant lag'
        elif fps < 20:
            msg = 'legit lag'
        elif fps < 25:
            msg = 'lag'
        elif fps < 29:
            msg = 'minor lag'
        G["SCREEN"].blit(
            G["HEL16"].render("FPS:{}".format(fps), 0, (0, 0, 0)),
            (0, 0))
        if msg:
            G["SCREEN"].blit(
                G["HEL16"].render(msg, 0, (0, 0, 0)),
                (0, 16))

        pygame.display.update()
        printer.save_surface(G["SCREEN"])
        if any(["CLIP" in inputs.get_state(state)["EVENTS"] for state in inputs.STATES]):
            printer.save_em()
            printer.make_gif()
            printer.clear_em()

        if "DEBUG" in G and G["DEBUG"]:
            if any(["CONSOLEDEBUG" in inputs.get_state(state)["EVENTS"]
                    for state in inputs.STATES]):
                execute_console_command(G)
        
        G["CLOCK"].tick(30)


def execute_console_command(G):
    cmd = get_text_input(G, (0, G["H"] - 32))
    if cmd is None: return
    if "REFERENCE" not in G:
        G["REFERENCE"] = "player10"

    if cmd.startswith("REF:"):
        G["REFERENCE"] = cmd.split(":")[-1]
    else:
        for frame in frames.get_frames():
            if frame.active:
                scripts.resolve(G["REFERENCE"], [scripts.parse_tokens(cmd)], frame.world)
                break
