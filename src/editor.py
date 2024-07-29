"""
Another stab at it eh?
I know src/editor_v2.py exists and this is editor.py
   but this is actually v3 where v1 is long lost to the git history
   maybe someday i dig around for it for funsies

This time I want to change the focus to be on UX
This should include drop down menus and selecting multiple actors

So far I have staretd with a header bar i put in src/editor_menuing.py 
  it acceps a dictionary reperesenting the structure of options as
  HEADER_OPTIONS["File"]["Save As"] <- a callable save as function
  so that would mean if the option is a dictionary then that shall
  be rendered as a submenu

Additionally there are little windows (src/editor_windows.py)
  I made a window for changing worlds so far.

  They take in two callbacks, a callback for updating and a callback for handling events

~~~ TODO ~~~
   Windows
   -------
  [] Sys
   \ [] Info (needs to add metadata around selectors)
     [x] Text Entry
      \ list display?
  [] Worlds
   \ [x] Edit (name, background)
     [x] World Select
     [x] World Actor View
      \ [] send actors to different world
        [] get actor from another world
  [] Actors
   \ [] Edit (name, direction, tabgible, physics, keys
              tileset, spritesheet, script map, hutbox, hurtbox)
      \ [] Hitbox Editor
     [] Template Select
  [] Sprites
   \ [] Spritesheet Editor
     [] Sprite List
     [] Sprite Map Editor

   Selector
   --------
  [] Drag Rect Selector (multiple actors)
  [] Single Click Selector
  [] Move Selected Actor(s)

Good luck, feel free to email me if you have questions about any of this
"""
import pygame
from pygame.locals import *

import sys
import os

from pprint import pformat
from copy import deepcopy

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
from src import build

import threading
BUILD_THREAD = None

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

OFFX, OFFY = -224, -160

SCROLLER = {
    "CX": OFFX, "CY": OFFY,
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

def build_on_thread(no_make=False, make_only=False):
    global BUILD_THREAD
    if BUILD_THREAD is not None and BUILD_THREAD.is_alive():
        return
    BUILD_THREAD = threading.Thread(target=build.build, args=(no_make, make_only))
    BUILD_THREAD.start()

    
def off(*args, **kwargs): pass
# ~~~ menu buttons ~~~
MENU_ITEMS = {
    "File": {
        "Load": load,
        "Save": save,
        "Build": {
            "Build Executable": lambda: build_on_thread(),
            "Run Makefile Only": lambda: build_on_thread(make_only=True),
            "Build Without Makefile": lambda: build_on_thread(no_make=True),
        },
        "Quit": quit,
    },
    "Info": lambda: windows.activate_window("Info"),
    "Worlds": {
        "World Edit": lambda: windows.activate_window("World Edit"),
        "World Select": lambda: windows.activate_window("World Select"),
        "World Actors": lambda: windows.activate_window("World Actors"),
    },
    "Template Select": lambda: windows.activate_window("Not Implemented"),
    "Sprites": {
        "Sprites List": lambda: windows.activate_window("Not Implemented"),
        "Spritesheet Editor": lambda: windows.activate_window("Not Implemented"),
        "Sprite Map Editor": lambda: windows.activate_window("Not Implemented"),
    }
}

# ~~~ window functions ~~~
def update_info_window(G, window):
    windows.window_base_update(G, window)
    x, y = 4, 36
    window["BODY"].blit(G["HEL16"].render(
        f"World: {G['WORLD']}",
        0,
        windows.THEMES[window["THEME"]].get("MENU_TXT")
    ), (x, y))
    y += 32
    tangible_count = len(list(filter(lambda a: G["ACTOR"].get_actor(a).tangible, G["WORLDS"].get_world(G['WORLD']).actors)))
    window["BODY"].blit(G["HEL16"].render(
        f"Actors in world (tangible): {len(G['WORLDS'].get_world(G['WORLD']).actors)} {tangible_count}",
        0,
        windows.THEMES[window["THEME"]].get("MENU_TXT"),
    ), (x, y))

def update_worlds_window(G, window):
    windows.window_base_update(G, window)
    # enforced strings
    for s, default in [("SELECTED", None), ("SEARCH", ""), ("SCROLL", 0)]:
        if s not in window: window[s] = default
#    text_rect = Rect((4, 36), (window["BODY"].get_width()-8, window["BODY"].get_height()-32-8))
#    pygame.draw.rect(window["BODY"], (255, 255, 255), text_rect)

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
        new=True,
    )
    window["BODY"].blit(surf, (4, 36))
    window["SELECTED"] = selected        

def handle_worlds_window_events(e, G, window):
    if e.type == pygame.MOUSEBUTTONDOWN:
        if e.button == 4: window["SCROLL"] -= 16
        if e.button == 5: window["SCROLL"] += 16
        if e.button == 1 and window["SELECTED"] is not None:
            if window["SELECTED"] == "new":
                name = window["SEARCH"] if window["SEARCH"] else "NewWorld"
                num = ""
                while name + num in WORLDS:
                    if num == "":
                        num = "0"
                    else:
                        num = int(num) + 1
                name = name + num
                create_new_world(name)
                G["WORLD"] = name
            else:
                G["WORLD"] = window["SELECTED"]
                
            G["SEARCH"] = ""

    if e.type == pygame.KEYDOWN:
        if e.key == pygame.K_UP: window["SCROLL"] += 128
        if e.key == pygame.K_DOWN: window["SCROLL"] -= 128

        if e.key == pygame.K_RETURN:
            if window["SEARCH"] in worlds.get_all_worlds():
                G["WORLD"] = wiondow["SEARCH"]
                G["SEARCH"] = ""

        if e.key == pygame.K_BACKSPACE: window["SEARCH"] = window["SEARCH"][:-1]
        if pygame.key.get_mods() & pygame.KMOD_SHIFT:
            if e.key in utils.ALPHABET_SHIFT_MAP:
                window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_SHIFT_MAP[e.key]
            elif e.key in utils.ALPHABET_KEY_MAP:
                window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_KEY_MAP[e.key].upper()
        elif e.key in utils.ALPHABET_KEY_MAP:
            window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_KEY_MAP[e.key]

def update_world_window(G, window):
    windows.window_base_update(G, window)
    for s, default in [("SELECTED", None)]:
        if s not in window: window[s] = default

    mpos = pygame.mouse.get_pos()
    theme = windows.THEMES[window["THEME"]]
    name_text = G["HEL16"].render(
        f"Name: {G['WORLD']}",
        0,
        theme.get("MENU_TXT"),
    )

    background_text = G["HEL16"].render(
        f"Background: {WORLDS[G['WORLD']]['background']}",
        0,
        theme.get("MENU_TXT"),
    )

    actor_count = len(G["WORLDS"].get_world(G['WORLD']).actors)
    tangible_count = len(list(filter(lambda a: G["ACTOR"].get_actor(a).tangible, G["WORLDS"].get_world(G['WORLD']).actors)))

    actors_text = G["HEL16"].render(
        f"Actors (tangible): {actor_count} ({tangible_count})",
        0,
        theme.get("MENU_TXT")
    )

    window["SELECTED"] = None

    x, y = 4, 36
    for text, name in [
            (name_text, "name"), (background_text, "background"), (actors_text, "actors")
    ]:
        rect = Rect(
            (window["POS"][0] + window["BODY"].get_width() - 68, window["POS"][1] + y),
            (64, 32)
        )
        if rect.collidepoint(mpos):
            window["SELECTED"] = name

        window["BODY"].blit(text, (x, y))
        button_rect = Rect((window["BODY"].get_width() - 68, y), (64, 32))
        if window["SELECTED"] == name:
            pygame.draw.rect(window["BODY"], theme.get("MENU_BG_SEL"), button_rect)
        else:
            pygame.draw.rect(window["BODY"], theme.get("MENU_BG"), button_rect)
        pygame.draw.rect(window["BODY"], theme.get("MENU_BG_ALT"), button_rect, width=1)
        
        y += 32

def handle_world_window_events(e, G, window):
    if e.type == MOUSEBUTTONDOWN:
        if window["SELECTED"] == "name":
            def world_name_on_entry(G, text):
                WORLDS[text] = WORLDS[G["WORLD"]]
                G["WORLD"] = text
                load_game()

            updater, event_handler = windows.make_text_entry_window(
                G, "World Name:", G["WORLD"], on_entry=world_name_on_entry)

            windows.add_window(
                "World Name Edit",
                (64, 64), (288, 256),
                theme=window["THEME"],
                update_callback=updater,
                event_callback=event_handler,
                args=[G]
            )
            windows.activate_window("World Name Edit")

        elif window["SELECTED"] == "background":
            def background_on_entry(G, text):
                if sprites.get_sprite(text) is not None:
                    WORLDS[G["WORLD"]]["background"] = text
                    load_game()

            updater, event_handler = windows.make_text_entry_window(
                G, "Background:", WORLDS[G["WORLD"]]["background"], on_entry=background_on_entry)

            windows.add_window(
                "World Background Edit",
                (64, 64), (288, 256),
                theme=window["THEME"],
                update_callback=updater,
                event_callback=event_handler,
                args=[G]
            )
            windows.activate_window("World Background Edit")

        elif window["SELECTED"] == "actors":
            windows.activate_window("World Actors")

def update_actors_in_world_window(G, window):
    windows.window_base_update(G, window)
    # enforced strings
    for s, default in [("SELECTED", None), ("SEARCH", ""), ("SCROLL", 0)]:
        if s not in window: window[s] = default

    mpos = pygame.mouse.get_pos()
    mpos = (
        mpos[0] - window["POS"][0] - 4,
        mpos[1] - window["POS"][1] - 36,
    )

    surf, selected = utils.scroller_list(
        worlds.get_world(G["WORLD"]).actors,
        mpos,
        (window["BODY"].get_width()-8,
         window["BODY"].get_height()-40),
        G["HEL16"],
        scroll=window["SCROLL"],
        search=window["SEARCH"],
        theme=window["THEME"],
        new=False,
    )
    window["BODY"].blit(surf, (4, 36))
    window["SELECTED"] = selected

def handle_actors_in_world_window_events(e, G, window):
    if e.type == pygame.MOUSEBUTTONDOWN:
        if e.button == 4: window["SCROLL"] -= 16
        if e.button == 5: window["SCROLL"] += 16
        if e.button == 1 and window["SELECTED"] is not None:
            a = actor.get_actor(window["SELECTED"])
            if a is not None:
                SCROLLER["CX"] = a.x + OFFX
                SCROLLER["CY"] = a.y + OFFY
            G["SEARCH"] = ""

    if e.type == pygame.KEYDOWN:
        if e.key == pygame.K_UP: window["SCROLL"] += 128
        if e.key == pygame.K_DOWN: window["SCROLL"] -= 128

        if e.key == pygame.K_RETURN:
            if window["SEARCH"] in worlds.get_all_worlds():
                G["WORLD"] = wiondow["SEARCH"]
                G["SEARCH"] = ""

        if e.key == pygame.K_BACKSPACE: window["SEARCH"] = window["SEARCH"][:-1]
        if pygame.key.get_mods() & pygame.KMOD_SHIFT:
            if e.key in utils.ALPHABET_SHIFT_MAP:
                window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_SHIFT_MAP[e.key]
            elif e.key in utils.ALPHABET_KEY_MAP:
                window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_KEY_MAP[e.key].upper()
        elif e.key in utils.ALPHABET_KEY_MAP:
            window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_KEY_MAP[e.key]

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
    pygame.draw.rect(
        G["SCREEN"],
        (0, 255, 0),
        Rect((G["SCREEN"].get_width()/2-576, G["SCREEN"].get_height()/2-320), (1152, 640)),
        width=1
    )
    
def set_up():
    pygame.init()
    pygame.mixer.init()
    pygame.mixer.music.set_volume(0)
    G = {}
    G["SCREEN"] = pygame.display.set_mode((1600, 1024))
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["WORLD"] = 'root' if "-l" not in sys.argv else sys.argv[sys.argv.index("-l") + 1]
    pygame.display.set_caption(",.+'*'+., Red Pants Editor V3 ,.+'*'+.,")
    inputs.add_state("PLAYER1")
    inputs.add_state("PLAYER2")

    theme = "FUNKY" if "-t" not in sys.argv else sys.argv[sys.argv.index("-t") + 1]

    load()
    load_game()

    header = MenuHeader(G["SCREEN"], MENU_ITEMS, theme=theme)
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
        sys=True, theme=theme,
        update_callback=update_info_window, args=[G]
    )

    windows.add_window(
        "World Edit", (56, 56), (256, 256),
        sys=True, theme=theme,
        update_callback=update_world_window,
        event_callback=handle_world_window_events,
        args=[G],
    )

    windows.add_window(
        "World Select", (40, 40), (512, 640),
        sys=True, theme=theme,
        update_callback=update_worlds_window,
        event_callback=handle_worlds_window_events,
        args=[G],
    )

    windows.add_window(
        "World Actors", (48, 48), (512, 256),
        sys=True, theme=theme,
        update_callback=update_actors_in_world_window,
        event_callback=handle_actors_in_world_window_events,
        args=[G],
    )

    windows.add_window(
        "Not Implemented",
        (80, 80), (320, 64),
        sys=True, theme=theme,
        update_callback=windows.window_base_update,
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

    if e.type == MOUSEBUTTONDOWN and e.button == 3:
        SCROLLER["DRAG"] = True

    if e.type == MOUSEMOTION and drag:
        SCROLLER["CX"] -= e.rel[0]
        SCROLLER["CY"] -= e.rel[1]
        CURSOR["X"] -= e.rel[0]
        CURSOR["Y"] -= e.rel[1]
        
    if e.type == MOUSEBUTTONUP:
        SCROLLER["DRAG"] = False
        SCROLLER["CX"] = (SCROLLER["CX"]+8) // 16 * 16
        SCROLLER["CY"] = (SCROLLER["CY"]+8) // 16 * 16
        CURSOR["X"] = (CURSOR["X"]+8) // 16 * 16
        CURSOR["Y"] = (CURSOR["Y"]+8) // 16 * 16

def create_new_world(name):
    WORLDS[name] = deepcopy(WORLD_TEMPLATE)
    load_game()

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
