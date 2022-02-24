import pygame

import sys
import os

from src import inputs
from src.editor import template_from_script, SCRIPT_LOCATION

def update_all_scripts(actors):
    filenames = []
    for _, _, files in os.walk(SCRIPT_LOCATION):
        for f in files:
            if f[-3:] == ".rp": filenames.append(f)
    templates = [template_from_script(fn) for fn in filenames]
    for template in templates:
        if template["name"] in actors.ACTORS:
            actors.ACTORS[template["name"]] = actors.Actor(template)

def set_up(loadscripts=False):
    pygame.init()
    W = 1152 if "-w" not in sys.argv else int(sys.argv[sys.argv.index("-w")+1])
    H = 640 if "-h" not in sys.argv else int(sys.argv[sys.argv.index("-h")+1])
    G = {}
    G["SCREEN"] = pygame.display.set_mode((W, H))
    pygame.display.set_caption("Red Pants Deluxe 2")
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["SCREEN"].fill((255, 255, 255))
    G["SCREEN"].blit(G["HEL32"].render("Loading...", 0, (0, 0, 0)), (0, 0))
    pygame.display.update()
    from src import sprites
    from src import worlds
    from src import actor
    from src import printer
    G["PRINTER"] = printer
    G["WORLDS"] = worlds
    G["WORLD"] = worlds.root if "-r" not in sys.argv else sys.argv[sys.argv.index("-r")+1]
    sprites.load()
    worlds.load()
    actor.load()
    update_all_scripts(actor)
    G["CLOCK"] = pygame.time.Clock()
    G["DEBUG"] = "-d" in sys.argv
    return G

def run(G):
    while True:
        inputs.update()
        world = G["WORLDS"].get_world(G["WORLD"])
        world.update()
        if G["DEBUG"]:
            world.draw(G["SCREEN"], DEBUG=G)
        else:
            world.draw(G["SCREEN"])
        pygame.display.update()
        if "PRINTER" in G:
            G["PRINTER"].save_surface(G["SCREEN"])
            if "CLIP" in inputs.STATE["EVENTS"]:
                G["PRINTER"].save_em()
                G["PRINTER"].make_gif()
                G["PRINTER"].clear_em()
        G["CLOCK"].tick(30)
        
