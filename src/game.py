import pygame

import sys

from src import inputs

def set_up():
    pygame.init()
    W = 1200 if "-w" not in sys.argv else int(sys.argv[sys.argv.index("-w")+1])
    H = 780 if "-h" not in sys.argv else int(sys.argv[sys.argv.index("-h")+1])
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

    G["WORLDS"] = worlds
    G["WORLD"] = worlds.root if "-r" not in sys.argv else sys.argv[sys.argv.index("-r")+1]
    sprites.load()
    worlds.load()
    actor.load()
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
            world.draw(G["SCREEN"]
        pygame.display.update()
    
