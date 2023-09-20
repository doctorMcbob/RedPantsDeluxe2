"""
Ah shit, here we go again.

BUTTONS:
  a button will be a piece of functionality attached to a rect.
  if we happen upon a click event we check the mouse position 
  against a bunch of rects and call the function mapped to the first one

(rect style tuple) -> Function

"""
import pygame
from pygame.locals import *
from pygame import Surface, Rect

import sys
import os

from copy import deepcopy
from pprint import pformat
from src.utils import *

from src import game
from src import inputs
from src import frames
from src import scripts
from src import sprites
from src import boxes
from src import actor
from src import worlds


# # # # # # # #
# Soft Layer  #
#-------------#
# to be saved #
# and loaded  #
#    over     #
# # # # # # # #
WORLDS = {}
SPRITESHEETS = {}
ACTORS = {}
SCRIPTS = {}
SPRITEMAPS = {}
OFFSETS = {}
HITBOXES = {}
HURTBOXES = {}

# GLOBALS
# BUTTONS ARE AT THE END OF FILE :) SORRY FOR BEING A LOON
WORLD_TEMPLATE = {"actors":[], "background":None, "x_lock": None, "y_lock": None}
SAVED = False
IMG_LOCATION = "img/"
SCRIPT_LOCATION = "scripts/"

TEMPLATES = {}

RECT_SCROLL = {
    "EVENTS": [],
}

TEMPLATES_SCROLL = {
    "SCROLL": 0,
    "EVENTS": [],
    "SELECTED": None,
}

ACTOR_SCROLL = {
    "SEG_H" : 128,
    "SEG_W" : 384,
    "SCROLL" : 0,
    "EVENTS" : [], 
}

CURSOR_SCROLLER = {
    "CX": 0, "CY": 0,
     "X": 0,  "Y": 0,
    "CORNER": None,
    "DRAG": False,
}

def load_game():
    sprites.swap_in(offsets=OFFSETS, sprite_maps=SPRITEMAPS)
    scripts.swap_in(SCRIPTS)
    worlds.swap_in(WORLDS)
    actor.swap_in(ACTORS)
    boxes.swap_in(hitboxes=HITBOXES, hurtboxes=HURTBOXES)
    
def make_rect(pos, pos2):
    x1 = min(pos[0], pos2[0])
    x2 = max(pos[0], pos2[0])
    y1 = min(pos[1], pos2[1])
    y2 = max(pos[1], pos2[1])
    return (x1, y1), ((x2 - x1), (y2 - y1))

def load():
    global WORLDS, SPRITESHEETS, SPRITEMAPS, ACTORS, SCRIPTS, OFFSETS, HITBOXES, HURTBOXES
    from src.lib import WORLDS as W
    from src.lib import SPRITESHEETS as S
    from src.lib import ACTORS as A
    from src.lib import SCRIPTS as SC
    from src.lib import BOXES as B
    WORLDS = W.WORLDS
    SPRITESHEETS = S.SPRITESHEETS
    SPRITEMAPS = S.SPRITEMAPS
    OFFSETS = S.OFFSETS
    ACTORS = A.ACTORS
    SCRIPTS = SC.SCRIPTS
    HITBOXES = B.HITBOXES
    HURTBOXES = B.HURTBOXES

def save(noload=False):
    global SAVED
    with open("src/lib/WORLDS.py", "w+") as f:
        f.write("WORLDS = {}".format(pformat(WORLDS)))
    with open("src/lib/SPRITESHEETS.py", "w+") as f:
        f.write("SPRITESHEETS = {}\nSPRITEMAPS = {}\nOFFSETS = {}".format(pformat(SPRITESHEETS), pformat(SPRITEMAPS), pformat(OFFSETS)))
    with open("src/lib/ACTORS.py", "w+") as f:
        f.write("ACTORS = {}".format(pformat(ACTORS)))
    with open("src/lib/SCRIPTS.py", "w+") as f:
        f.write("SCRIPTS = {}".format(pformat(SCRIPTS)))
    with open("src/lib/BOXES.py", "w+") as f:
        f.write("HITBOXES = {}\nHURTBOXES = {}".format(pformat(HITBOXES), pformat(HURTBOXES)))
    
    SAVED = True
    if noload: return
    load_game()
    
def update_frames(G):
    frames.clear()
    frames.add_frame("EDITOR_VIEW", G["WORLD"], (1152, 640))
    frames.add_frame("MAIN", G["WORLD"], (1152, 640))
    G["FRAMEMAP"] = {
        "MAIN": (0, 0),
    }

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

    from src import printer
    printer.GIF_SIZE = 30 * 4
    G["PRINTER"] = printer
    
    G["FRAMES"] = frames
    G["WORLDS"] = worlds
    G["ACTOR"] = actor
    G["CLOCK"] = pygame.time.Clock()
    update_frames(G)
    return G

def draw_buttons(G, mpos):
    for rect in BUTTONS.keys():
        name = BUTTON_TEXT[rect]
        pos, dim = rect
        col = (80, 150, 180) if Rect(pos, dim).collidepoint(mpos) else (255, 255, 255)
        pygame.draw.rect(G["SCREEN"], (0, 0, 0), Rect(pos, dim))
        pygame.draw.rect(G["SCREEN"], col, Rect((pos[0]+2, pos[1]+2), (dim[0]-4, dim[1]-4)))
        G["SCREEN"].blit(G["HEL16"].render(name, 0, (0, 0, 0)), (pos[0]+4, pos[1]+4))

def draw_actors_bar(G, world, mpos):
    seg_h = ACTOR_SCROLL["SEG_H"]
    seg_w = ACTOR_SCROLL["SEG_W"]
    scroll = ACTOR_SCROLL["SCROLL"]
    for i, a in enumerate(world.actors):
        if  (i+1+scroll) * seg_h < 0 or (i+scroll) * seg_h > G["SCREEN"].get_height():
            continue
        friend = actor.get_actor(a)
        col = (255, 0, 0) if Rect((1168, (i+scroll) * seg_h), (seg_w, seg_h)).collidepoint(mpos) else (0, 0, 0)
        pygame.draw.rect(G["SCREEN"], col, Rect((1168, (i+scroll) * seg_h), (seg_w, seg_h)))
        pygame.draw.rect(G["SCREEN"], (255,255,255), Rect((1168 + 4, (i+scroll) * seg_h + 4), (seg_w - 8, seg_h - 8))) 
        G["SCREEN"].blit(G["HEL32"].render(friend.name, 0, (0,0,0)), (1168+8, (i+scroll) * seg_h + 4))

        surf = Surface((80, 80))
        surf.fill((150, 150, 150))
        surf.blit(friend.get_sprite(), (0, 0))
        G["SCREEN"].blit(surf, (1168+8, (i+scroll) * seg_h + 4 + 32))

        hitbox_button = Surface((80, 48))
        hitbox_button.fill((150, 150, 255))
        hitbox_button.blit(G["HEL16"].render("HITBOX", 0, (0, 0, 0)), (8, 4))
        hitbox_button.blit(G["HEL16"].render("EDIT", 0, (0, 0, 0)), (20, 24))
        G["SCREEN"].blit(hitbox_button, (1168+8 + 100, (i+scroll) * seg_h + 4 + 32))

        delete_button = Surface((80, 48))
        delete_button.fill((255, 150, 150))
        delete_button.blit(G["HEL16"].render("REMOVE", 0, (0, 0, 0)), (8, 4))
        delete_button.blit(G["HEL16"].render("ACTOR", 0, (0, 0, 0)), (16, 24))
        G["SCREEN"].blit(delete_button, (1168+8 + 100 + 100, (i+scroll) * seg_h + 4 + 32))

        up_button = Surface((64, 20))
        up_button.fill((155, 155, 155))
        up_button.blit(G["HEL16"].render("TOP", 0, (0, 0, 0)), (8, 2))
        G["SCREEN"].blit(up_button, (1168+8 + 100 + 100 + 100, (i+scroll) * seg_h + 4 + 32))

        down_button = Surface((64, 20))
        down_button.fill((155, 155, 155))
        down_button.blit(G["HEL16"].render("BOT", 0, (0, 0, 0)), (8, 2))
        G["SCREEN"].blit(down_button, (1168+8 + 100 + 100 + 100, (i+scroll) * seg_h + 4 + 32+32))

def update_actors_bar(G, mpos, btn):
    seg_h = ACTOR_SCROLL["SEG_H"]
    seg_w = ACTOR_SCROLL["SEG_W"]
    scroll = ACTOR_SCROLL["SCROLL"]
    world = worlds.get_world(G["WORLD"])
    if not Rect((1168, 0), (seg_w, G["SCREEN"].get_height())).collidepoint(mpos):
        return
    for e in ACTOR_SCROLL["EVENTS"]:
        if e == "SCROLL UP": ACTOR_SCROLL["SCROLL"] = max(ACTOR_SCROLL["SCROLL"] - 1, 0 - len(world.actors)+7)
        if e == "SCROLL DOWN": ACTOR_SCROLL["SCROLL"] = min(ACTOR_SCROLL["SCROLL"] + 1, 0)

    for i, a in enumerate(world.actors):
        if  (i+1+scroll) * seg_h < 0 or (i+scroll) * seg_h > G["SCREEN"].get_height():
            continue
        friend = actor.get_actor(a)

        if Rect((1168+8 + 100, (i+scroll) * seg_h + 4 + 32), (80, 48)).collidepoint(mpos) and btn in [0, 1]:
            hitbox_menu(G, friend.name)

        elif Rect((1168+8 + 100 + 100, (i+scroll) * seg_h + 4 + 32), (80, 48)).collidepoint(mpos) and btn in [0, 1]:
            WORLDS[G['WORLD']]['actors'].remove(friend.name)
            found = 0
            for w in WORLDS.keys():
                if friend.name in WORLDS[w]['actors']:
                     found = 1
                     break
            if found == 0:
                ACTORS.pop(friend.name)
            if friend.name in world.actors:
                world.actors.remove(friend.name)
            load_game()

        elif Rect((1168+8 + 100 + 100 + 100, (i+scroll) * seg_h + 4 + 32), (64, 20)).collidepoint(mpos) and btn in [0, 1]:
            WORLDS[G["WORLD"]]["actors"].remove(a)
            WORLDS[G["WORLD"]]["actors"].append(a)
            load_game()
        elif Rect((1168+8 + 100 + 100 + 100, (i+scroll) * seg_h + 4 + 32+32), (64, 20)).collidepoint(mpos) and btn in [0, 1]:
            WORLDS[G["WORLD"]]["actors"].remove(a)
            WORLDS[G["WORLD"]]["actors"] = [a] + WORLDS[G["WORLD"]]["actors"]
            load_game()
            
        elif Rect((1168, (i+scroll) * seg_h), (seg_w, seg_h)).collidepoint(mpos) and btn in [0, 1]:
            CURSOR_SCROLLER["X"] = friend.x
            CURSOR_SCROLLER["Y"] = friend.y
            CURSOR_SCROLLER["CX"] = friend.x
            CURSOR_SCROLLER["CY"] = friend.y

    
def add_actor(G, pos, template_name):
    G["ctx"] = CURSOR_SCROLLER
    if 'plat' in template_name:
        pos = pos[0] // 32 * 32, pos[1] // 32 * 32
        rect = input_rect(G, (0, 0, 100), cb=main_click_helper, snap=32, pos=pos)
    else:
        rect = input_rect(G, (0, 0, 100), cb=main_click_helper, snap=16, pos=pos)
    if not rect: return
    rect = ((rect[0][0] + CURSOR_SCROLLER["CX"], rect[0][1] + CURSOR_SCROLLER["CY"]), rect[1])
    template = deepcopy(TEMPLATES[template_name])
    template["POS"], template["DIM"] = rect
    n = 0
    
    while actor.get_actor("{}{}".format(template_name, n)) is not None:
        print(actor.get_actor("{}{}".format(template_name, n)))
        n += 1
    name = "{}{}".format(template_name, n)
    template["name"] = name
    ACTORS[name] = template
    WORLDS[G["WORLD"]]["actors"].append(name)
    load_game()
    update_frames(G)
    
def update_templates_scroll(G, mpos, btn):
    zone = Rect((288, 640), (6*128, G["SCREEN"].get_height()-640))
    if zone.collidepoint(mpos):
        for e in ACTOR_SCROLL["EVENTS"]:
            if e == "SCROLL UP": TEMPLATES_SCROLL["SCROLL"] = max(TEMPLATES_SCROLL["SCROLL"] - 1, 2 - len(TEMPLATES.keys()) // 6)
            if e == "SCROLL DOWN": TEMPLATES_SCROLL["SCROLL"] = min(TEMPLATES_SCROLL["SCROLL"] + 1, 0)

        keys = list(TEMPLATES.keys())
        keys.sort()
        for i, template in enumerate(keys):
            y = (i // 6 + TEMPLATES_SCROLL["SCROLL"]) * 64 + 640
            x = (i % 6) * 128 + 288
            if y < 640 or y > G["SCREEN"].get_height():
                continue
            if Rect((x, y), (128, 64)).collidepoint(mpos) and btn in [0, 1]:
                TEMPLATES_SCROLL["SELECTED"] = template if TEMPLATES_SCROLL["SELECTED"] != template else None

def draw_templates(G, mpos):
    keys = list(TEMPLATES.keys())
    keys.sort()
    for i, template in enumerate(keys):
        y = (i // 6 + TEMPLATES_SCROLL["SCROLL"]) * 64 + 640
        x = (i % 6) * 128 + 288
        if y < 640 or y > G["SCREEN"].get_height():
            continue
        col = (255, 0, 0) if Rect((x, y), (128, 64)).collidepoint(mpos) else (0, 0, 0)
        col2 = (150, 250, 150) if template == TEMPLATES_SCROLL["SELECTED"] else (255, 255, 255)
        surf = Surface((128, 64))
        surf.fill(col)
        pygame.draw.rect(surf, col2, Rect((2, 2), (124, 60)))
        surf.blit(G["HEL16"].render(template, 0, (0, 0, 0)), (4, 4))
        G["SCREEN"].blit(surf, (x, y))
        
def draw(G, mpos=None):
    global SAVED

    CORNER = CURSOR_SCROLLER["CORNER"]
    X, Y = CURSOR_SCROLLER["X"], CURSOR_SCROLLER["Y"]
    CX, CY = CURSOR_SCROLLER["CX"], CURSOR_SCROLLER["CY"]
    
    G["SCREEN"].fill((255, 255, 255))
    frame = frames.get_frame("EDITOR_VIEW")
    frame.scroll_x = CX
    frame.scroll_y = CY
    
    world = G["WORLDS"].get_world(G["WORLD"])
    frame.world = world
    drawn = frame.drawn(DEBUG=G)
    
    pygame.draw.rect(drawn, (255, 0, 0), Rect(make_rect((CORNER[0]-CX, CORNER[1]-CY) if CORNER is not None else (X-CX+16, Y-CY+16), (X-CX, Y-CY))), width=2)
    G["SCREEN"].blit(drawn, (0, 0))
    G["SCREEN"].blit(G["HEL16"].render("WORLD: {}".format(G["WORLD"]), 0, (0, 0, 0)), (0, G["SCREEN"].get_height() - 16))
    G["SCREEN"].blit(G["HEL16"].render("TEMPL: {}".format(TEMPLATES_SCROLL["SELECTED"]), 0, (0, 0, 0)), (0, G["SCREEN"].get_height() - 32))
    if world.x_lock is not None:
        pygame.draw.line(G["SCREEN"], (0, 255, 0), (world.x_lock, 0), (world.x_lock + 32, 0), 10)
    if world.y_lock is not None:
        pygame.draw.line(G["SCREEN"], (0, 255, 0), (0, world.y_lock), (0, world.y_lock + 32), 10)

    mpos = mpos if mpos is not None else pygame.mouse.get_pos()
    draw_buttons(G, mpos)
    draw_actors_bar(G, world, mpos)
    draw_templates(G, mpos)
    
def main_click_helper(G):
    global WORLDS
    mpos = pygame.mouse.get_pos()
    draw(G, mpos)
    if mpos[0] < 1152 and mpos[1] < 640:
        pygame.draw.circle(G["SCREEN"], (0, 0, 0), (mpos[0]//16*16, mpos[1]//16*16), 2)
    update_cursor(G)
    mods = pygame.key.get_mods()
    if "EVENTS" in G:
        for e in G["EVENTS"]:
            if e.type == KEYDOWN and e.key == K_RETURN:
                load_game()
                game.run(G, noquit=True)
                load_game()
                update_frames(G)

            if e.type == KEYDOWN and e.key == K_s and mods & KMOD_CTRL:
                save()

def update_cursor(G):
    mods = pygame.key.get_mods()
    drag = CURSOR_SCROLLER["DRAG"]
    for e in G["EVENTS"]:
        if e.type == KEYDOWN:
            if e.key == K_LEFT:
                CURSOR_SCROLLER["X"] -= 16 if mods & KMOD_SHIFT else 32
                if mods & KMOD_CTRL:
                    CURSOR_SCROLLER["CX"] -= 16 if mods & KMOD_SHIFT else 32
            if e.key == K_UP:
                CURSOR_SCROLLER["Y"] -= 16 if mods & KMOD_SHIFT else 32
                if mods & KMOD_CTRL:
                    CURSOR_SCROLLER["CY"] -= 16 if mods & KMOD_SHIFT else 32
            if e.key == K_RIGHT:
                CURSOR_SCROLLER["X"] += 16 if mods & KMOD_SHIFT else 32
                if mods & KMOD_CTRL:
                    CURSOR_SCROLLER["CX"] += 16 if mods & KMOD_SHIFT else 32
            if e.key == K_DOWN:
                CURSOR_SCROLLER["Y"] += 16 if mods & KMOD_SHIFT else 32
                if mods & KMOD_CTRL:
                    CURSOR_SCROLLER["CY"] += 16 if mods & KMOD_SHIFT else 32

        if e.type == MOUSEMOTION and drag:
            CURSOR_SCROLLER["CX"] -= e.rel[0]
            CURSOR_SCROLLER["CY"] -= e.rel[1]
            CURSOR_SCROLLER["X"] -= e.rel[0]
            CURSOR_SCROLLER["Y"] -= e.rel[1]

        if e.type == MOUSEBUTTONUP:
            CURSOR_SCROLLER["DRAG"] = False
            CURSOR_SCROLLER["CX"] = CURSOR_SCROLLER["CX"] // 16 * 16
            CURSOR_SCROLLER["CY"] = CURSOR_SCROLLER["CY"] // 16 * 16
            CURSOR_SCROLLER["X"] = CURSOR_SCROLLER["X"] // 16 * 16
            CURSOR_SCROLLER["Y"] = CURSOR_SCROLLER["Y"] // 16 * 16

def do_buttons(G, pos):
    for rect in BUTTONS:
        if Rect(rect).collidepoint(pos):
            BUTTONS[rect](G)
    
def run(G):
    while True:
        update_frames(G)
        pos, btn = expect_click(args=G, cb=main_click_helper)
        mods = pygame.key.get_mods()
        if pos is None:
            if (SAVED or mods & KMOD_CTRL):
                sys.exit()
            continue
        
        ACTOR_SCROLL["EVENTS"] = []
        if btn == 4: ACTOR_SCROLL["EVENTS"].append("SCROLL UP")
        if btn == 5: ACTOR_SCROLL["EVENTS"].append("SCROLL DOWN")

        TEMPLATES_SCROLL["EVENTS"] = []
        if btn == 4: TEMPLATES_SCROLL["EVENTS"].append("SCROLL UP")
        if btn == 5: TEMPLATES_SCROLL["EVENTS"].append("SCROLL DOWN")

        update_actors_bar(G, pos, btn)
        update_templates_scroll(G, pos, btn)
        
        if pos[0] < 1152 and pos[1] < 640 and btn in [2, 3]:
            CURSOR_SCROLLER["DRAG"] = True
            
        if pos[0] < 1152 and pos[1] < 640 and btn in [0, 1] and TEMPLATES_SCROLL["SELECTED"] is not None:
            ctx = {
                "scrollx": 0,
                "scrolly": 0,
            }
            G["ctx"] = ctx
            add_actor(G, pos, TEMPLATES_SCROLL["SELECTED"])
            
        do_buttons(G, pos)
            
def template_from_script(filename, name=None):
    with open(SCRIPT_LOCATION + filename) as f:
        script = f.read()

    segments = script.split("|")
    
    if name is None:
        name = segments.pop(0)
    else:
        segments.pop(0)
    
    rect = segments.pop(0)
    x, y, w, h = [int(n) for n in rect.split(",")]
    
    tangible = segments.pop(0)

    sprites = {}
    offsets = {}
    for line in segments.pop(0).splitlines():
        if not line: continue
        key, sprite, offset = line.split()
        offset = [int(n) for n in offset.split(",")]

        sprites[key] = sprite
        offsets[sprite] = offset

    actor_scripts = {}
    while segments:
        key = segments.pop(0)
        cmds = segments.pop(0)
        script = [
            scripts.parse_tokens(cmd)
            for cmd in cmds.splitlines()
        ]
        actor_scripts[key] = list(filter(lambda n: bool(n), script))
        

    offsetkey = filename.split(".")[0]
    OFFSETS[offsetkey]= offsets
    
    spritekey = filename.split(".")[0]
    SPRITEMAPS[spritekey] = sprites

    scriptkey = filename.split(".")[0]
    SCRIPTS[scriptkey] = actor_scripts
    
    return {
        "name": name,
        "POS": (x, y),
        "DIM": (w, h),
        "sprites": spritekey,
        "scripts": scriptkey,
        "offsetkey": offsetkey,
        "tangible": tangible == "True"
    }

def drawn_spritesheet_data(G, d, idx=None):
    keys = d.keys()
    surf = Surface((512, (len(d.keys()) + 1) * 16))
    surf.fill((255, 255, 255))
    offset = 0 if idx < 20 else (idx - 20) * 16
        
    for i, key in enumerate(keys):
        col = (200, 0, 120) if i == idx else (0, 0, 0)
        surf.blit(G["HEL16"].render(key, 0 , col), (0, (i * 16) - offset))
        surf.blit(G["HEL16"].render(str(d[key]), 0 , col), (256, (i * 16)-offset))
    if idx is not None:
        col = (200, 0, 120) if idx == len(d) else (0, 0, 0)
        surf.blit(G["HEL16"].render("ADD...", 0 , col), (0, (len(keys) * 16) - offset))
    return surf

def spritesheet_menu(G):
    filenames = []
    for _, _, files in os.walk(IMG_LOCATION):
        for f in files:
            if f[-4:] == ".png":
                filenames.append(f)
    filenames.sort()
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Select spritesheet filename", 0, (0, 0, 0)),
            (0, 0)
        )

    filename = select_from_list(G, filenames, (0, 32), args=G, cb=update)
    if not filename: return
    if filename not in SPRITESHEETS:
        SPRITESHEETS[filename] = {}

    image = pygame.image.load(IMG_LOCATION + filename).convert()
    sheet = SPRITESHEETS[filename]
    SX, SY = (0, 0)
    CX, CY = (0, 0)
    corner = None
    
    idx = 0
    keys = list(sheet.keys())

    while True:
        G["SCREEN"].fill((150, 150, 150))
        G["SCREEN"].blit(image, (SX, SY))
        for key in sheet.keys():
            pos, dim = sheet[key]
            pygame.draw.rect(G["SCREEN"], (100, 0, 0), Rect((pos[0]+SX, pos[1]+SY), dim), width=1)
        G["SCREEN"].blit(drawn_spritesheet_data(G, sheet, idx), (1072-256, 0))
        G["SCREEN"].blit(G["HEL32"].render("{}, {}".format((CX, CY), corner), 0, (200, 0, 80)), (1072, 840-32))
        pygame.draw.rect(G["SCREEN"], (255, 0, 0), Rect(make_rect((SX+corner[0], SY+corner[1]) if corner else (SX+CX+16, SY+CY+16), (SX+CX, SY+CY))), width=2)
        inp = expect_input()
        mods = pygame.key.get_mods()
        if inp == K_BACKSPACE: corner = None

        if mods & KMOD_CTRL:
            if inp == K_UP: idx = (idx - 1) % (len(keys) + 1)
            if inp == K_DOWN: idx = (idx + 1) % (len(keys) + 1)

        else:
            if inp == K_LEFT: CX -= 16 + (48 * (mods & KMOD_SHIFT))
            if inp == K_UP: CY -= 16 + (48 * (mods & KMOD_SHIFT))
            if inp == K_RIGHT: CX += 16 + (48 * (mods & KMOD_SHIFT))
            if inp == K_DOWN: CY += 16 + (48 * (mods & KMOD_SHIFT))

        if inp == K_a: SX -= 16 + (48 * (mods & KMOD_SHIFT))
        if inp == K_w: SY -= 16 + (48 * (mods & KMOD_SHIFT))
        if inp == K_d: SX += 16 + (48 * (mods & KMOD_SHIFT))

        if inp == K_s and mods & KMOD_CTRL:
            save()

        elif inp == K_s: SY += 16 + (48 * (mods & KMOD_SHIFT))

        if inp == K_ESCAPE:
            return

        if inp == K_SPACE and mods & KMOD_CTRL:
            if idx < len(keys):
                pos, dim = sheet[keys[idx]]
                CX, CY = pos
                SX, SY = (0-pos[0], 0-pos[1])
                corner = pos[0] + dim[0], pos[1] + dim[1]
        
        elif inp == K_SPACE:
            corner = (CX, CY)

        if inp == K_RETURN and corner is not None:
            if idx < len(keys):
                sheet[keys[idx]] = make_rect(corner, (CX, CY))
            else:
                name = get_text_input(G, (0, 0))
                if name is not None and name != "":
                    if mods & KMOD_SHIFT:
                        X, Y = corner
                        sheet[name+"00"] = make_rect((X, Y), (CX, CY))
                        sheet[name+"01"] = make_rect((X+32, Y), (CX+32, CY))
                        sheet[name+"02"] = make_rect((X+64, Y), (CX+64, CY))
                        sheet[name+"10"] = make_rect((X, Y+32), (CX, CY+32))
                        sheet[name+"11"] = make_rect((X+32, Y+32), (CX+32, CY+32))
                        sheet[name+"12"] = make_rect((X+64, Y+32), (CX+64, CY+32))
                        sheet[name+"20"] = make_rect((X, Y+64), (CX, CY+64))
                        sheet[name+"21"] = make_rect((X+32, Y+64), (CX+32, CY+64))
                        sheet[name+"22"] = make_rect((X+64, Y+64), (CX+64, CY+64))
                    else:
                        sheet[name] = make_rect(corner, (CX, CY))
                    keys = list(sheet.keys())

def load_template_button(G):
    filenames = []
    for _, _, files in os.walk(SCRIPT_LOCATION):
        for f in files:
            if f[-3:] == ".rp":
                filenames.append(f)
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Select script filename", 0, (0, 0, 0)),
            (0, 0)
        )
    choice = select_from_list(G, filenames, (0, 32), args=G, cb=update)
    if not choice: return
    template = template_from_script(choice)
    TEMPLATES[choice.split(".")[0]] = template

def load_all_templates_button(G):
    filenames = []
    for _, _, files in os.walk(SCRIPT_LOCATION):
        for f in files:
            if f[-3:] == ".rp":
                filenames.append(f)

    for filename in filenames:
        try:
            template = template_from_script(filename)
            TEMPLATES[filename.split(".")[0]] = template
        except Exception as e:
            print("Failed to load {} because of {}".format(filename, e))

def box_menu_draw(G):
    G["SCREEN"].fill((200, 200, 200))
    actor = G["ctx"]["actor"]
    actor.direction = -1
    sprite = actor.get_sprite()
    sprite = pygame.transform.scale2x(sprite)
    sprite = pygame.transform.scale2x(sprite)
    scrollx = G["ctx"]["scrollx"]
    scrolly = G["ctx"]["scrolly"]
    hitboxkey = G["ctx"]["key"]
    offx, offy = actor.get_offset()
    G["SCREEN"].blit(sprite, (scrollx + (offx*4), scrolly + (offy*4)))
    
    x, y = 0-actor.w*4-1, 0-actor.h*4-1
    while x <= actor.w * 4 * 2:
        pygame.draw.line(G["SCREEN"], (100, 100, 100), (x+scrollx, 0-actor.h*4+scrolly), (x+scrollx, actor.h*4*2+scrolly))
        x += 4
    while y <= actor.h * 4 * 2:
        pygame.draw.line(G["SCREEN"], (100, 100, 100), (0-actor.w*4+scrollx, y+scrolly), (actor.w*4*2+scrollx, y+scrolly))
        y += 4

    # actor is a rect ;) ECB - environmental collision box
    ecb = Rect(scrollx, scrolly, actor.w, actor.h)
    pygame.draw.rect(G["SCREEN"], (0, 0, 255), resize_up(ecb), width=2)

    hurtboxes = actor.get_hurtboxes()
    if hurtboxes is not None:
        for hurtbox in hurtboxes:
            box = Rect((hurtbox.x - actor.x) * 4 + scrollx, (hurtbox.y - actor.y) * 4 + scrolly, hurtbox.w, hurtbox.h)
            pygame.draw.rect(G["SCREEN"], (0, 255, 0), resize_up(box), width=2)

    hitboxes = actor.get_hitboxes()
    if hitboxes is not None:
        for hitbox in hitboxes:
            box = Rect((hitbox.x - actor.x) * 4 + scrollx, (hitbox.y - actor.y) * 4 + scrolly, hitbox.w, hitbox.h)
            pygame.draw.rect(G["SCREEN"], (255, 0, 0), resize_up(box), width=2)

    G["SCREEN"].blit(G["HEL32"].render("{}:{}   |   {}".format(actor.state, actor.frame, G["ctx"]["identifier"]), 0, (0, 0, 0)), (0, G["SCREEN"].get_height()-32))

    x, y = G["SCREEN"].get_width() // 2, 16
    G["SCREEN"].blit(G["HEL16"].render("Hitboxes:", 0, (0, 0, 0)), (x, y))
    y += 16
    x += 32
    for key in HITBOXES[hitboxkey].keys():
        G["SCREEN"].blit(G["HEL16"].render("{}: {}".format(key, repr(HITBOXES[hitboxkey][key])), 0, (0, 0, 0)), (x, y))
        y += 16
    x -= 32
    G["SCREEN"].blit(G["HEL16"].render("Hurtboxes:", 0, (0, 0, 0)), (x, y))
    y += 16
    x += 32
    for key in HURTBOXES[hitboxkey].keys():
        G["SCREEN"].blit(G["HEL16"].render("{}: {}".format(key, repr(HURTBOXES[hitboxkey][key])), 0, (0, 0, 0)), (x, y))
        y += 16

def hitbox_menu(G, actor_name):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("hitbox key:", 0, (0, 0, 0)),
            (0, 0)
        )
    box_keys = list(HITBOXES.keys()) + ["New..."]
    hitboxkey = select_from_list(G, box_keys, (0, 32), args=G, cb=update)            
    if hitboxkey is None: return
    if hitboxkey == "New...": hitboxkey = get_text_input(G, (0, 32))
    if not hitboxkey: return
    if hitboxkey not in HITBOXES: HITBOXES[hitboxkey] = {}
    if hitboxkey not in HURTBOXES: HURTBOXES[hitboxkey] = {}
    act = actor.get_actor(actor_name)
    act.hitboxes = HITBOXES[hitboxkey]
    act.hurtboxes = HURTBOXES[hitboxkey]
    ctx = {
        "actor": act,
        "scrollx": 128,
        "scrolly": 128,
        "key": hitboxkey,
        "identifier": "{}:{}".format(act.state, act.frame),
    }
    G["ctx"] = ctx
    while True:
        if ctx["identifier"] not in HITBOXES[hitboxkey]: HITBOXES[hitboxkey][ctx["identifier"]] = []
        if ctx["identifier"] not in HURTBOXES[hitboxkey]: HURTBOXES[hitboxkey][ctx["identifier"]] = []
        box_menu_draw(G)
        inp = expect_input()
        if inp is None or inp == K_ESCAPE: return None
        if inp == K_i:
            rect = input_rect(G, (100, 0, 0), box_menu_draw)
            if rect is not None:
                rect = ((rect[0][0]//4, rect[0][1]//4), (rect[1][0]//4, rect[1][1]//4)) 
                HITBOXES[hitboxkey][ctx["identifier"]].append(rect)
            
        if inp == K_u:
            rect = input_rect(G, (0, 100, 0), box_menu_draw)
            if rect is not None:
                rect = ((rect[0][0]//4, rect[0][1]//4), (rect[1][0]//4, rect[1][1]//4)) 
                HURTBOXES[hitboxkey][ctx["identifier"]].append(rect)

        if inp == K_RETURN:
            ctx["identifier"] = "{}:{}".format(act.state, act.frame)

        if inp == K_w:
            ctx["scrolly"] += 32

        if inp == K_a:
            ctx["scrollx"] -= 32

        if inp == K_s:
            ctx["scrolly"] -= 32

        if inp == K_d:
            ctx["scrollx"] += 32

            
        if inp == K_LEFT:
            act.frame -= 1

        if inp == K_RIGHT:
            act.frame += 1

        if inp == K_SPACE:
            state = get_text_input(G, (0, 32))
            act.state = act.state if not state else state

        if inp == K_BACKSPACE:
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Remove from:", 0, (0, 0, 0)),
                     (0, 0)
                )
            choice = select_from_list(G, ["Hitbox", "Hurtbox"], (0, 32))
            if choice is not None:
                selection = HITBOXES[hitboxkey][ctx["identifier"]] if choice == "Hitbox" else HURTBOXES[hitboxkey][ctx["identifier"]]
                if not selection: continue
                rect = select_from_list(G, selection, (0, 32))
                if rect in selection:
                    selection.remove(rect)

def resize_up(r, n=4):
    return Rect(r.x, r.y, r.w*n, r.h*n)

def switch_worlds(G):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Select World: ", 0, (0, 0, 0)),
            (0, 0)
        )
    choice = select_from_list(G, worlds.get_all_worlds() + ["New..."], (0, 32), args=G, cb=update)
    if choice == "New...":
        choice = get_text_input(G, (0, 32))
        WORLDS[choice] = deepcopy(WORLD_TEMPLATE)
        load_game()
    if not choice: return
    WORLDS[choice]["name"] = choice
    G["WORLD"] = choice
    ACTOR_SCROLL["SCROLL"] = 0

def delete_world(G):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Delete World?? D:", 0, (255, 255, 255)),
            (0, 0)
        )
    choice = select_from_list(G, ["No", "Big No", "Yes", "Please Don't"], (0, 32), args=G, cb=update)
    if not choice == "Yes": return
    if G["WORLD"] != "root":
        WORLDS.pop(G["WORLD"])
        worlds.worlds.pop(G["WORLD"])
        G["WORLD"] = "root"
    
def change_world_background(G):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Select Background", 0, (0, 0, 0)),
            (0, 0)
        )
    if "background.png" not in SPRITESHEETS:
        return
    background = select_from_list(G, list(SPRITESHEETS["background.png"].keys()), (0, 32), args=G, cb=update)
    if not background: return
    WORLDS[G["WORLD"]]["background"] = background
    worlds.get_world(G["WORLD"]).background = sprites.get_sprite(background)

def add_actor_from_world(G):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("World to select actor from:", 0, (0, 0, 0)),
            (0, 0)
        )
    world_ref = select_from_list(G, worlds.get_all_worlds(), (0, 32), args=G, cb=update)
    if not world_ref: return
    world = worlds.get_world(world_ref)
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Actor from world {}".format(world_ref), 0, (0, 0, 0)),
            (0, 0)
        )
    name = select_from_list(G, world.actors, (0, 32), args=G, cb=update)
    if not name: return
    if name not in WORLDS[G["WORLD"]]["actors"]:
        WORLDS[G["WORLD"]]["actors"].append(name)
        load_game()

def add_actor_to_world(G):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Actor:", 0, (0, 0, 0)),
            (0, 0)
        )
    world = worlds.get_world(G["WORLD"])
    name = select_from_list(G, world.actors, (0, 32), args=G, cb=update)
    if not name: return
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("World to send actor to:", 0, (0, 0, 0)),
            (0, 0)
        )
    world_ref = select_from_list(G, worlds.get_all_worlds(), (0, 32), args=G, cb=update)
    if not world_ref: return
    world = worlds.get_world(world_ref)
    if not world: return
    if name not in WORLDS[world_ref]["actors"]:
        WORLDS[world_ref]["actors"].append(name)
        load_game()

def add_actor_to_all_worlds(G):
    def update(G):
        draw(G)
        G["SCREEN"].blit(
            G["HEL32"].render("Actor:", 0, (0, 0, 0)),
            (0, 0)
        )
    world = worlds.get_world(G["WORLD"])
    name = select_from_list(G, world.actors, (0, 32), args=G, cb=update)
    if not name: return
    print("Here") 
    for world_ref in WORLDS.keys():
        if name not in WORLDS[world_ref]["actors"]:
            WORLDS[world_ref]["actors"].append(name)
    print("Done")
    load_game()
        

def off(*args, **kwargs): pass
BUTTONS = {
    ((16, 656), (256, 32)): spritesheet_menu,
    ((16, 656 + 32), (256, 32)): load_template_button,
    ((16, 656 + 32 * 2), (256, 32)): load_all_templates_button,
    ((16, 656 + 32 * 3), (256//3, 32)): add_actor_to_all_worlds,
    ((16+256//3, 656 + 32 * 3), (256//3, 32)): add_actor_to_world,
    ((16+(256//3*2), 656 + 32 * 3), (256//3, 32)): add_actor_from_world,
    ((16, 656 + 32 * 4), (256, 32)): switch_worlds,
    ((16, 656 + 32 * 5), (256, 32)): delete_world,
    ((16, 656 + 32 * 6), (256, 32)): change_world_background,
}

BUTTON_TEXT = {
    ((16, 656), (256, 32)): "Spritesheet Menu",
    ((16, 656 + 32), (256, 32)): "Load Template",
    ((16, 656 + 32 * 2), (256, 32)): "Load All Templates",
    ((16, 656 + 32 * 3), (256//3, 32)): "Actor All",
    ((16+256//3, 656 + 32 * 3), (256//3, 32)): "Actor To",
    ((16+(256//3*2), 656 + 32 * 3), (256//3, 32)): "Actor From",
    ((16, 656 + 32 * 4), (256, 32)): "Switch Worlds",
    ((16, 656 + 32 * 5), (256, 32)): "Delete World",    
    ((16, 656 + 32 * 6), (256, 32)): "Change World Background",
    
}
