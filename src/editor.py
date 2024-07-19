"""
Another stab at it eh?

This time I want to change the focus to be on UX
This should include drop down menus and selecting multiple actors
"""
import pygame
from pygame.locals import *

import sys
import os

from src import game
from src import inputs
from src import frames
from src import scripts
from src import sprites
from src import boxes
from src import actor
from src import worlds

def set_up():
    pygame.init()
    pygame.mixer.init()
    G = {}
    G["SCREEN"] = pygame.display.set_mode((1600, 1000))
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["WORLD"] = 'root' if "-l" not in sys.argv else sys.argv[sys.argv.index("-l") + 1]

    inputs.add_state("PLAYER1")
    inputs.add_state("PLAYER2")

    load()
#    load_game()

    from src import printer
    printer.GIF_SIZE = 30 * 4
    G["PRINTER"] = printer
    
    G["FRAMES"] = frames
    G["WORLDS"] = worlds
    G["ACTOR"] = actor
    G["CLOCK"] = pygame.time.Clock()
    update_frames(G)
    return G


def update_frames(G):
    frames.clear()
    frames.add_frame("EDITOR_VIEW", G["WORLD"], (1152, 640))
    frames.add_frame("MAIN", G["WORLD"], (1152, 640))
    G["FRAMEMAP"] = {
        "MAIN": (0, 0),
    }

def run(G):
    while True:
        update_frames(G)
