"""
Another stab at it eh?

This time I want to change the focus to be on UX
This should include drop down menus and selecting multiple actors
"""
import pygame
from pygame.locals import *

import sys
import os

from pprint import pformat

from src import game
from src import inputs
from src import frames
from src import scripts
from src import sprites
from src import boxes
from src import actor
from src import worlds
from src.editor_menuing import MenuHeader

# # # # # # # #
# Soft Layer  #
#-------------#
# to be saved #
# and loaded  #
#    over     #
# good idea.? #
# # # # # # # #
WORLDS = {}
SPRITESHEETS = {}
ACTORS = {}
SCRIPTS = {}
SPRITEMAPS = {}
OFFSETS = {}
TILEMAPS = {}
HITBOXES = {}
HURTBOXES = {}

# ~~~ globals ~~~
SAVED = True
IMG_LOCATION = "img/"
SCRIPT_LOCATION = "scripts/"
WORLD_TEMPLATE = {"actors":[], "background":None, "x_lock": None, "y_lock": None}

SCROLLER = {
    "X": 0, "Y": 0,
    "CX": 0, "CY": 0
}
# ~~~ menu functions ~~~
def load():
    global WORLDS, SPRITESHEETS, SPRITEMAPS, ACTORS, SCRIPTS, OFFSETS, HITBOXES, HURTBOXES, TILEMAPS
    from src.lib import WORLDS as W
    from src.lib import SPRITESHEETS as S
    from src.lib import ACTORS as A
    from src.lib import SCRIPTS as SC
    from src.lib import BOXES as B
    WORLDS = W.WORLDS
    SPRITESHEETS = S.SPRITESHEETS
    SPRITEMAPS = S.SPRITEMAPS
    OFFSETS = S.OFFSETS
    TILEMAPS = S.TILEMAPS
    ACTORS = A.ACTORS
    SCRIPTS = SC.SCRIPTS
    HITBOXES = B.HITBOXES
    HURTBOXES = B.HURTBOXES

def save(noload=False):
    global SAVED
    with open("src/lib/WORLDS.py", "w+") as f:
        f.write("WORLDS = {}".format(pformat(WORLDS)))
    with open("src/lib/SPRITESHEETS.py", "w+") as f:
        f.write("SPRITESHEETS = {}\nSPRITEMAPS = {}\nOFFSETS = {}\nTILEMAPS = {}".format(
            pformat(SPRITESHEETS), pformat(SPRITEMAPS), pformat(OFFSETS), pformat(TILEMAPS))
        )
    with open("src/lib/ACTORS.py", "w+") as f:
        f.write("ACTORS = {}".format(pformat(ACTORS)))
    with open("src/lib/SCRIPTS.py", "w+") as f:
        f.write("SCRIPTS = {}".format(pformat(SCRIPTS)))
    with open("src/lib/BOXES.py", "w+") as f:
        f.write("HITBOXES = {}\nHURTBOXES = {}".format(pformat(HITBOXES), pformat(HURTBOXES)))
    
    SAVED = True
    if noload: return
    load_game()

def off(*args, **kwargs): pass
# ~~~ menu buttons ~~~
MENU_ITEMS = {
    "File": {
        "Load": load,
        "Save": save,
        "----------": off,
        "quit": quit
    },
    
}

def load_game():
    sprites.swap_in(offsets=OFFSETS, sprite_maps=SPRITEMAPS, tile_maps=TILEMAPS)
    scripts.swap_in(SCRIPTS)
    worlds.swap_in(WORLDS)
    actor.swap_in(ACTORS)
    boxes.swap_in(hitboxes=HITBOXES, hurtboxes=HURTBOXES)

def draw(G):
    CX, CY = SCROLLER["CX"], SCROLLER["CY"]
    frame = frames.get_frame("EDITOR_VIEW")
    frame.scroll_x = CX
    frame.scroll_y = CY
    world = G["WORLDS"].get_world(G["WORLD"])
    frame.world = world
    drawn = frame.drawn(DEBUG=G)
    G["SCREEN"].blit(drawn, (0, 32))
    
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
    load_game()

    header = MenuHeader(G["SCREEN"], MENU_ITEMS)
    G["HEADER"] = header
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
    frames.add_frame(
        "EDITOR_VIEW",
        G["WORLD"], (
            G["SCREEN"].get_width(),
            G["SCREEN"].get_height() - 32
        )
    )
    frames.add_frame("MAIN", G["WORLD"], (1152, 640))
    G["FRAMEMAP"] = {
        "MAIN": (0, 0),
    }

def run(G):
    while True:
        draw(G)
        update_frames(G)
        G["HEADER"].update()
        pygame.display.update()
        G["HEADER"].CLICK = False
        for e in pygame.event.get():
            if e.type == pygame.MOUSEBUTTONUP:
                G["HEADER"].CLICK = True
