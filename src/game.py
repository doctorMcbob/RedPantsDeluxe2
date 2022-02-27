import pygame

import sys
import os

from src import inputs
from src.editor import template_from_script, SCRIPT_LOCATION
from src import menu

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
    G = {"W":W,"H":H}
    G["SCREEN"] = pygame.display.set_mode((W, H)) if "-f" not in sys.argv else pygame.display.set_mode((W, H), pygame.FULLSCREEN)
    pygame.display.set_caption("Red Pants Deluxe 2")
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["SCREEN"].fill((255, 255, 255))
    G["SCREEN"].blit(G["HEL32"].render("Loading...", 0, (0, 0, 0)), (0, 0))
    pygame.display.update()
    from src import sprites
    from src import worlds
    from src import frames
    from src import actor
    from src import printer
    G["PRINTER"] = printer
    G["WORLDS"] = worlds
    G["FRAMES"] = frames
    G["WORLD"] = worlds.root if "-r" not in sys.argv else sys.argv[sys.argv.index("-r")+1]
    sprites.load()
    worlds.load()
    actor.load()
    if loadscripts:
        update_all_scripts(actor)

    G["INPUTS"] = inputs
    G["FRAMES"] = frames
    menu.run_controller_menu(G)
    
    G["CLOCK"] = pygame.time.Clock()
    G["DEBUG"] = "-d" in sys.argv
    return G

def run(G, noquit=False):
    while True:
        if inputs.update(noquit) == "QUIT":
            return
        world = G["WORLDS"].get_world(G["WORLD"])
        world.update()
        for name in G["FRAMEMAP"]:
            frame = G["FRAMES"].get_frame(name)
            frame.update()
            position = G["FRAMEMAP"][name]
            drawn = frame.drawn(world, DEBUG=G) if "DEBUG" in G and G["DEBUG"] else frame.drawn(world)
            G["SCREEN"].blit(drawn, position)

        # maybe remove FPS counter before release, dont worry about it for a while though
        G["SCREEN"].blit(
            G["HEL16"].render("FPS:{}".format(G["CLOCK"].get_fps()), 0, (0, 0, 0)),
            (0, 0))
        pygame.display.update()
        if "PRINTER" in G:
            G["PRINTER"].save_surface(G["SCREEN"])
            if any(["CLIP" in inputs.get_state(state)["EVENTS"] for state in inputs.STATES]):
                G["PRINTER"].save_em()
                G["PRINTER"].make_gif()
                G["PRINTER"].clear_em()
        
        G["CLOCK"].tick(30)
        
