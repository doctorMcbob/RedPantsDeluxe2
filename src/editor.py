"""
Another stab at it eh?
I know src/editor_v2.py exists and this is editor.py
   but this is actually v3 where v1 is long lost to the git history
   maybe someday i dig around for it for funsies

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
from src import editor_windows as windows
from src import utils

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
    "CX": 0, "CY": 0,
    "DRAG": False,
}
CURSOR = {
    "X": 0, "Y": 0,
    "CORNER": None,
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
        "Build": {
            # TODO
            "Build Executable": off,
            "Run Makefile Only": off,
            "Build Without Makefile": off,
        },
        "Quit": quit,
    },
    "Windows": {
        "Info": lambda: windows.activate_window("Info"),
        "World Select": lambda: windows.activate_window("World Select"),
    },
}

# ~~~ window functions ~~~
def update_info_window(G, window):
    windows.window_base_update(G, window)
    text_rect = Rect((4, 36), (window["BODY"].get_width()-8, window["BODY"].get_height()-32-8))
    pygame.draw.rect(window["BODY"], (255, 255, 255), text_rect)
    x, y = 4, 36
    window["BODY"].blit(G["HEL16"].render(
        f"World: {G['WORLD']}",
        0,
        (0, 0, 0)
    ), (x, y))
    y += 32
    window["BODY"].blit(G["HEL16"].render(
        f"Actors in world: {len(worlds.get_world(G['WORLD']).actors)}",
        0,
        (0, 0, 0)
    ), (x, y))

def update_worlds_window(G, window):
    windows.window_base_update(G, window)
    # enforced strings
    for s, default in [("SELECTED", None), ("SEARCH", ""), ("SCROLL", 0)]:
        if s not in window: window[s] = default
    text_rect = Rect((4, 36), (window["BODY"].get_width()-8, window["BODY"].get_height()-32-8))
    pygame.draw.rect(window["BODY"], (255, 255, 255), text_rect)

    mpos = pygame.mouse.get_pos()
    mpos = (
        mpos[0] - window["POS"][0] - 4,
        mpos[1] - window["POS"][1] - 36,
    )
    surf, selected = utils.scroller_list(
        worlds.get_all_worlds(),
        mpos,
        (window["BODY"].get_width()-8,
         window["BODY"].get_height()-40),
        G["HEL16"],
        scroll=window["SCROLL"],
        search=window["SEARCH"],
        theme=window["THEME"],
    )
    window["BODY"].blit(surf, (4, 36))
    window["SELECTED"] = selected
    

def handle_worlds_window_events(e, G, window):
    if e.type == pygame.MOUSEBUTTONDOWN:
        if e.button == 4: window["SCROLL"] -= 16
        if e.button == 5: window["SCROLL"] += 16
        if e.button == 1 and window["SELECTED"] is not None:
            G["WORLD"] = window["SELECTED"]

    if e.type == pygame.KEYDOWN:
        if e.key == K_UP: window["SCROLL"] += 128
        if e.key == K_DOWN: window["SCROLL"] -= 128

# ~~~ other ~~~
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
    pygame.display.set_caption(",.+'*'+., Red Pants Editor V3 ,.+'*'+.,")
    inputs.add_state("PLAYER1")
    inputs.add_state("PLAYER2")

    load()
    load_game()

    header = MenuHeader(G["SCREEN"], MENU_ITEMS, theme="FUNKY")
    G["HEADER"] = header
    from src import printer
    printer.GIF_SIZE = 30 * 4
    G["PRINTER"] = printer
    
    G["FRAMES"] = frames
    G["WORLDS"] = worlds
    G["ACTOR"] = actor
    G["CLOCK"] = pygame.time.Clock()
    G["DEBUG"] = True
    update_frames(G)

    windows.add_window(
        "Info", (32, 32), (256, 256),
        sys=True, theme="FUNKY",
        update_callback=update_info_window, args=[G]
    )

    windows.add_window(
        "World Select", (40, 40), (512, 640),
        sys=True, theme="FUNKY",
        update_callback=update_worlds_window,
        event_callback=handle_worlds_window_events,
        args=[G],
    )
    
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

def draw_demo(G):
    # Thinking about how we could keep the menu bar
    #  updating along side the demo simulation...
    #  might need a seperate instance of frames module
    G["SCREEN"].blit(
        G["DEMO"]["SCREEN"],
        (
            (G["SCREEN"].get_width()/2) - (G["DEMO"]["SCREEN"].get_width()/2),
            (G["SCREEN"].get_height()/2) - (G["DEMO"]["SCREEN"].get_height()/2),
        )
    )

def demo(G):
    frames.clear()
    frames.add_frame("ROOT", G["WORLD"], (1152, 640))
    demo_G = {
        "W": 1152,"H": 640,
        "DEBUG": True,
        "CLOCK": G["CLOCK"],
    }
    demo_G["SCREEN"] = pygame.Surface((demo_G["W"], demo_G["H"]))
    G["DEMO"] = demo_G
    demo_G["ROOT"] = G["WORLD"]
    demo_G["HEL16"] = G["HEL16"]
    demo_G["HEL32"] = G["HEL32"]
    load_game()
    game.run(demo_G, noquit=True, cb=draw_demo, args=[G])
    load_game()
    update_frames(G)

def update_cursor_events(e):
    drag = SCROLLER["DRAG"]

    if e.type == MOUSEBUTTONDOWN:
        SCROLLER["DRAG"] = True

    if e.type == MOUSEMOTION and drag:
        SCROLLER["CX"] -= e.rel[0]
        SCROLLER["CY"] -= e.rel[1]
        CURSOR["X"] -= e.rel[0]
        CURSOR["Y"] -= e.rel[1]
        
    if e.type == MOUSEBUTTONUP:
        SCROLLER["DRAG"] = False
        SCROLLER["CX"] = SCROLLER["CX"] // 16 * 16
        SCROLLER["CY"] = SCROLLER["CY"] // 16 * 16
        CURSOR["X"] = CURSOR["X"] // 16 * 16
        CURSOR["Y"] = CURSOR["Y"] // 16 * 16

def run(G):
    while True:
        draw(G)
        update_frames(G)
        windows.update_windows(G)
        
        G["HEADER"].update()
        pygame.display.update()
        G["HEADER"].CLICK = False
        for e in pygame.event.get():
            if e.type == pygame.MOUSEBUTTONUP:
                G["HEADER"].CLICK = True

            if e.type == pygame.KEYDOWN and e.key == K_RETURN:
                # <>
                # For now the demo is going to darken the screen...
                # If i find a way to run the demo concurrently to the other windows/header then
                #  perhaps this will be removed
                G["SCREEN"].fill((0,0,0))
                pygame.display.update()
                # </>
                demo(G)

            window = windows.window_at_mouse()
            if window is not None:
                windows.handle_window_events(G, e, window)
            else:
                update_cursor_events(e)
