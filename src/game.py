import pygame

import sys
import os

from src import inputs
from src.editor import template_from_script, SCRIPT_LOCATION
from src import menu

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
    from src import sprites
    from src import worlds
    from src import frames
    from src import actor
    from src import printer
    from src import scripts
    from src import boxes
    G["PRINTER"] = printer
    G["WORLDS"] = worlds
    G["FRAMES"] = frames
    G["SCRIPTS"] = scripts
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
    return G

def run(G, noquit=False):
    while True:
        if inputs.update(noquit) == "QUIT":
            return

        worlds_updated = []
        for name in G["FRAMEMAP"]:
            frame = G["FRAMES"].get_frame(name)
            if frame.world not in worlds_updated:
                frame.world.update()
                worlds_updated.append(frame.world)
            frame.update()
            position = G["FRAMEMAP"][name]
            drawn = frame.drawn(DEBUG=G) if "DEBUG" in G and G["DEBUG"] else frame.drawn()
            G["SCREEN"].blit(drawn, position)

        # maybe remove FPS counter before release, dont worry about it for a while though
        fps = G["CLOCK"].get_fps()
        # if fps < 15:
        #     print('very significant lag')
        # elif fps < 20:
        #     print('legit lag')
        # elif fps < 25:
        #     print('lag')
        # elif fps < 29:
        #     print('minor lag')

        G["SCREEN"].blit(
            G["HEL16"].render("FPS:{}".format(fps), 0, (0, 0, 0)),
            (0, 0))
        pygame.display.update()
        if "PRINTER" in G:
            G["PRINTER"].save_surface(G["SCREEN"])
            if any(["CLIP" in inputs.get_state(state)["EVENTS"] for state in inputs.STATES]):
                G["PRINTER"].save_em()
                G["PRINTER"].make_gif()
                G["PRINTER"].clear_em()
        
        G["CLOCK"].tick(30)
        
