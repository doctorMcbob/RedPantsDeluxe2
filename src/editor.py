"""
A note on the UX of this editor:
  fuck, i know, i really just need to re make this completely at this point, gone too deep
  maybe move the read/write logic out of this file into utils or something
  use the freaking mouse you idiot
  show text on mouse over (use actor debug?)

"""
import pygame
from pygame.locals import *
from pygame import Surface, Rect

import sys
import os

from copy import deepcopy

from src import inputs
from src import frames
from src import game
from src import sprites
from src import scripts
from src import boxes
from src.utils import expect_input, select_from_list, get_text_input, expect_click

WORLDS = {}
SPRITESHEETS = {}
ACTORS = {}
SCRIPTS = {}
SPRITEMAPS = {}
OFFSETS = {}
HITBOXES = {}
HURTBOXES = {}

WORLD_TEMPLATE = {"actors":[], "background":None, "x_lock": None, "y_lock": None}

SAVED = False

IMG_LOCATION = "img/"
SCRIPT_LOCATION = "scripts/"
X, Y = 0, 0
W, H = 1152, 640
CX, CY = 0, 0
X_LOCK, Y_LOCK = None, None
CORNER = None
TEMPLATES = {}

SPLITSCREEN = False

def make_single_player(G):
    frames.clear()
    frames.add_frame("EDITOR_VIEW", G["WORLD"], (W, H))
    frames.add_frame("MAIN", G["WORLD"], (W, H))
    G["FRAMEMAP"] = {
        "MAIN": (0, 0),
    }

def make_split_screen(G):
    frames.clear()
    frames.add_frame("EDITOR_VIEW", G["WORLD"], (W, H))
    frames.add_frame("MAIN", G["WORLD"], (W//2, H))
    frames.add_frame("MAIN2", G["WORLD"], (W//2, H))
    G["FRAMEMAP"] = {
        "MAIN": (0, 0),
        "MAIN2": (W//2, 0),
    }
    
def set_up():
    pygame.init()
    G = {}
    G["SCREEN"] = pygame.display.set_mode((1200, 720))
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)
    G["WORLD"] = 'root' if "-l" not in sys.argv else sys.argv[sys.argv.index("-l") + 1]

    inputs.add_state("PLAYER1")
    inputs.add_state("PLAYER2")
    
    from src import worlds
    from src import actor
    sprites.load()
    scripts.load()
    worlds.load()
    actor.load()
    boxes.load()
    
    G["FRAMES"] = frames
    G["WORLDS"] = worlds
    G["ACTOR"] = actor
    G["CLOCK"] = pygame.time.Clock()
    load()

    make_single_player(G)
        
    return G

def draw(G):
    global SAVED
    G["SCREEN"].fill((255, 255, 255))
    frame = frames.get_frame("EDITOR_VIEW")
    frame.scroll_x = CX
    frame.scroll_y = CY
    
    world = G["WORLDS"].get_world(G["WORLD"])
    frame.world = world
    drawn = frame.drawn(DEBUG=G)
    pygame.draw.rect(drawn, (255, 0, 0), Rect(make_rect((CORNER[0]-CX, CORNER[1]-CY) if CORNER is not None else (X-CX+16, Y-CY+16), (X-CX, Y-CY))), width=2)
    G["SCREEN"].blit(drawn, (0, 0))
    G["SCREEN"].blit(G["HEL32"].render("WORLD: {}".format(G["WORLD"]), 0, (0, 0, 0)), (0, G["SCREEN"].get_height() - 32))
    if X_LOCK is not None:
        pygame.draw.line(G["SCREEN"], (0, 255, 0), (X_LOCK, 0), (X_LOCK + 32, 0), 10)
    if Y_LOCK is not None:
        pygame.draw.line(G["SCREEN"], (0, 255, 0), (0, Y_LOCK), (0, Y_LOCK + 32), 10)      

def run(G):
    global CORNER, X, Y, CX, CY, SPLITSCREEN, SAVED, X_LOCK, Y_LOCK
    while True:
        inp = expect_input(args=G, cb=draw)
        mods = pygame.key.get_mods()
        if inp == K_ESCAPE and (SAVED or mods & KMOD_CTRL):
            sys.exit()

        if inp == K_LEFT and mods & KMOD_SHIFT:
            CX += 64
            X += 64
        elif inp == K_LEFT: X -= 32

        if inp == K_UP and mods & KMOD_SHIFT:
            CY += 64
            Y += 64
        elif inp == K_UP: Y -= 32

        if inp == K_RIGHT and mods & KMOD_SHIFT:
            CX -= 64
            X -= 64
        elif inp == K_RIGHT: X += 32

        if inp == K_DOWN and mods & KMOD_SHIFT:
            CY -= 64
            Y -= 64
        elif inp == K_DOWN: Y += 32

        if inp == K_o and mods & KMOD_CTRL:
            load()
            
        if inp == K_w and mods & KMOD_SHIFT:
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("WORLDS:", 0, (0, 0, 0)),
                    (0, 0)
                )
            world_keys = list(WORLDS.keys())
            world_keys.append("New...")
            if not world_keys: continue
            choice = select_from_list(G, world_keys, (0, 32), args=G, cb=update)
            if not choice: continue
            if choice != "New...":
                G["WORLD"] = choice
                continue
            draw(G)
            G["SCREEN"].blit(
                G["HEL32"].render("New world name:", 0, (0, 0, 0)),
                (0, 0)
            )
            name = get_text_input(G, (0, 32))
            if name:
                WORLDS[name] = deepcopy(WORLD_TEMPLATE)
                SAVED = False

        if inp == K_BACKSPACE:
            shift = mods & KMOD_SHIFT
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Delete Actor:", 0, (0, 0, 0)),
                     (0, 0)
                )
            actor_keys = WORLDS[G["WORLD"]]["actors"]
            if not actor_keys: continue
            choice = select_from_list(G, actor_keys, (0, 32), args=G, cb=update)
            if not choice: continue
            WORLDS[G["WORLD"]]["actors"].remove(choice)
            if shift: ACTORS.pop(choice)
            SAVED = False

        if inp == K_SPACE:
            if CORNER is None:
                CORNER = (X, Y)
                continue

            rect = make_rect(CORNER, (X, Y))
            CORNER = None
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Select template", 0, (0, 0, 0)),
                     (0, 0)
                )
            template_keys = list(TEMPLATES.keys())
            if not template_keys: continue
            choice = select_from_list(G, template_keys, (0, 32), args=G, cb=update)
            if not choice: continue
            template = deepcopy(TEMPLATES[choice])
            template["POS"], template["DIM"] = rect
            draw(G)
            G["SCREEN"].blit(
                G["HEL32"].render("Template Name", 0, (0, 0, 0)),
                (0, 0)
            )
            name = get_text_input(G, (0, 32))
            if name is not None:
                template["name"] = name
                ACTORS[name] = template
                G["ACTOR"].TEMPLATES[name] = template
                G["ACTOR"].add_actor_from_template(name, name)
                WORLDS[G["WORLD"]]["actors"].append(name)
                SAVED = False
                
        if inp == K_RETURN:
            save()
            sprites.load()
            G["ACTOR"].load()
            G["WORLDS"].load()
            boxes.load()
            game.run(G, noquit=True)
            sprites.load()
            G["ACTOR"].load()
            G["WORLDS"].load()
            boxes.load()

        if inp == K_x and mods & KMOD_SHIFT:
            X_LOCK = CX if X_LOCK is None else None
            WORLDS[G["WORLD"]]["x_lock"] = X_LOCK
            
        if inp == K_y and mods & KMOD_SHIFT:
            Y_LOCK = CY if Y_LOCK is None else None
            WORLDS[G["WORLD"]]["y_lock"] = Y_LOCK

        if inp == K_s and mods & KMOD_CTRL:
            save()
            boxes.load()
            sprites.load()
            G["ACTOR"].load()
            G["WORLDS"].load()

        elif inp == K_s and mods & KMOD_SHIFT:
            filenames = []
            for _, _, files in os.walk(IMG_LOCATION):
                for f in files:
                    if f[-4:] == ".png":
                        filenames.append(f)
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Select spritesheet filename", 0, (0, 0, 0)),
                     (0, 0)
                )
            choice = select_from_list(G, filenames, (0, 32), args=G, cb=update)
            if choice: spritesheet_menu(G, choice)
            SAVED = False

        if inp == K_h and mods & KMOD_SHIFT:
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Actor Box Menu:", 0, (0, 0, 0)),
                     (0, 0)
                )
            actor_keys = WORLDS[G["WORLD"]]["actors"]
            if not actor_keys: continue
            choice = select_from_list(G, actor_keys, (0, 32), args=G, cb=update)
            if not choice: continue
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("hitbox key:", 0, (0, 0, 0)),
                    (0, 0)
                )
            box_keys = list(HITBOXES.keys()) + ["New..."]
            hitboxkey = select_from_list(G, box_keys, (0, 32), args=G, cb=update)            
            if hitboxkey is None: continue
            if hitboxkey == "New...": hitboxkey = get_text_input(G, (0, 32))
            if not hitboxkey: continue
            hitbox_menu(G, choice, hitboxkey)
            
        elif inp == K_s:
            SPLITSCREEN = not SPLITSCREEN
            if SPLITSCREEN:
                make_split_screen(G)
            else:
                make_single_player(G)

        if inp == K_t and mods & KMOD_SHIFT:
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
            if choice is None: continue
            G["SCREEN"].blit(
                G["HEL32"].render("Template Name", 0, (0, 0, 0)),
                (0, 0)
            )
            name = get_text_input(G, (0, 32))
            if name is None: continue
            template = template_from_script(choice)
            TEMPLATES[name] = template
            SAVED = False

def save():
    global SAVED
    with open("src/lib/WORLDS.py", "w+") as f:
        f.write("WORLDS = {}".format(repr(WORLDS)))
    with open("src/lib/SPRITESHEETS.py", "w+") as f:
        f.write("SPRITESHEETS = {}\nSPRITEMAPS = {}\nOFFSETS = {}".format(repr(SPRITESHEETS), repr(SPRITEMAPS), repr(OFFSETS)))
    with open("src/lib/ACTORS.py", "w+") as f:
        f.write("ACTORS = {}".format(repr(ACTORS)))
    with open("src/lib/SCRIPTS.py", "w+") as f:
        f.write("SCRIPTS = {}".format(repr(SCRIPTS)))
    with open("src/lib/BOXES.py", "w+") as f:
        f.write("HITBOXES = {}\nHURTBOXES = {}".format(repr(HITBOXES), repr(HURTBOXES)))
    
    SAVED = True

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

    scripts = {}
    while segments:
        key = segments.pop(0)
        cmds = segments.pop(0)
        scripts[key] = cmds.splitlines()

    offsetkey = filename.split(".")[0]
    OFFSETS[offsetkey]= offsets
    
    spritekey = filename.split(".")[0]
    SPRITEMAPS[spritekey] = sprites

    scriptkey = filename.split(".")[0]
    SCRIPTS[scriptkey] = scripts
    
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
        surf.blit(G["HEL16"].render(str(d[key]), 0 , col), (128, (i * 16)-offset))
    if idx is not None:
        col = (200, 0, 120) if idx == len(d) else (0, 0, 0)
        surf.blit(G["HEL16"].render("ADD...", 0 , col), (0, (len(keys) * 16) - offset))
    return surf

def make_rect(pos, pos2):
    x1 = min(pos[0], pos2[0])
    x2 = max(pos[0], pos2[0])
    y1 = min(pos[1], pos2[1])
    y2 = max(pos[1], pos2[1])
    return (x1, y1), ((x2 - x1), (y2 - y1))


def spritesheet_menu(G, filename):
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
        G["SCREEN"].blit(drawn_spritesheet_data(G, sheet, idx), (1072-256, 0))
        G["SCREEN"].blit(G["HEL32"].render("{}, {}".format((CX, CY), corner), 0, (200, 0, 80)), (1072, 840-32))
        pygame.draw.rect(G["SCREEN"], (255, 0, 0), Rect(make_rect((SX+corner[0], SY+corner[1]) if corner else (SX+CX+16, SY+CY+16), (SX+CX, SY+CY))), width=2)
        inp = expect_input()
        mods = pygame.key.get_mods()
        if inp == K_BACKSPACE: corner = None

        if mods & KMOD_CTRL:
            if inp == K_UP: idx = max(0, idx - 1) 
            if inp == K_DOWN: idx = min(len(keys), idx + 1)

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
                SX, SY = (0-pos[0], pos[1])
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

def drawn_script(G, script, idx=None, offset=0):
    surf = Surface((812, 32 * len(script)))
    surf.fill((255, 255, 255))
    for i, cmd in enumerate(script):
        surf.blit(G["HEL32"].render(cmd, 0, (0, 0, 0)), (0, i*32))
        if idx is not None and idx == i+offset:
            pygame.draw.line(surf, (255, 0, 0), (0, (i*32)-1), (512, (i*32)-1), 2)
    return surf

def drawn_actor_spritesheet(G, sprites, idx=None, offset=0):
    surf = Surface((812, 32*len(sprites)))
    surf.fill((255, 255, 255))
    for i, key in enumerate(list(sprites.keys())):
        surf.blit(G["HEL32"].render("{}: {}".format(key, sprites[key]), 0, (0, 0, 0)), (0, i*32))
        if idx is not None and idx == i+offset:
            pygame.draw.line(surf, (255, 0, 0), (0, (i*32)-2), (512, (i*32)-2), 2)
    return surf

def drawn_actor_template(G, template, scroll=0, idx=None):
    surf = Surface((812, 512))
    surf.fill((255, 255, 255))
    x, y = 0, 0-scroll
    i = 0
    keys = ["name", "POS", "DIM"]
    for key in keys:
        surf.blit(G["HEL32"].render(("{}: {}".format(key, template[key])), 0, (0,0,0)), (x, y))
        if idx is not None and idx == i:
            pygame.draw.line(surf, (255, 0, 0), (0, y+31), (512, y+31), 2)
        i += 1
        y += 32

    spritesheet_data = drawn_actor_spritesheet(G, template["sprites"], idx=idx, offset=i-1)
    surf.blit(spritesheet_data, (x, y))
    i += len(template["sprites"])
    y += spritesheet_data.get_height()

    for key in list(template["scripts"].keys()):
        surf.blit(G["HEL32"].render(key, 0, (0, 0, 0)), (x+64, y))
        if idx is not None and idx == i:
            pygame.draw.line(surf, (255, 0, 0), (0, y+31), (512, y+31), 2)
        y += 32
        i += 1
        script_data = drawn_script(G, template["scripts"][key], idx=idx, offset=i-1)
        surf.blit(script_data, (x, y))
        i += len(template["scripts"][key])
        y += script_data.get_height()
    return surf

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

def hitbox_menu(G, actor, hitboxkey):
    if hitboxkey not in HITBOXES: HITBOXES[hitboxkey] = {}
    if hitboxkey not in HURTBOXES: HURTBOXES[hitboxkey] = {}
    actor = G["ACTOR"].get_actor(actor)
    actor.hitboxes = HITBOXES[hitboxkey]
    actor.hurtboxes = HURTBOXES[hitboxkey]
    ctx = {
        "actor": actor,
        "scrollx": 128,
        "scrolly": 128,
        "key": hitboxkey,
        "identifier": "{}:{}".format(actor.state, actor.frame),
    }
    G["ctx"] = ctx
    while True:
        if ctx["identifier"] not in HITBOXES[hitboxkey]: HITBOXES[hitboxkey][ctx["identifier"]] = []
        if ctx["identifier"] not in HURTBOXES[hitboxkey]: HURTBOXES[hitboxkey][ctx["identifier"]] = []
        box_menu_draw(G)
        inp = expect_input()
        if inp is None or inp == K_ESCAPE: return None
        if inp == K_i:
            rect = input_rect(G, (100, 0, 0))
            if rect is not None:
                HITBOXES[hitboxkey][ctx["identifier"]].append(rect)
            
        if inp == K_u:
            rect = input_rect(G, (0, 100, 0))
            if rect is not None:
                HURTBOXES[hitboxkey][ctx["identifier"]].append(rect)

        if inp == K_RETURN:
            ctx["identifier"] = "{}:{}".format(actor.state, actor.frame)

        if inp == K_w:
            ctx["scrolly"] += 32

        if inp == K_a:
            ctx["scrollx"] -= 32

        if inp == K_s:
            ctx["scrolly"] -= 32

        if inp == K_d:
            ctx["scrollx"] += 32

            
        if inp == K_LEFT:
            actor.frame -= 1

        if inp == K_RIGHT:
            actor.frame += 1

        if inp == K_SPACE:
            state = get_text_input(G, (0, 32))
            actor.state = actor.state if not state else state

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

def resize_up(r):
    return Rect(r.x, r.y, r.w*4, r.h*4)

def input_rect(G, col=(100, 100, 100)):
    scrollx, scrolly = G["ctx"]["scrollx"], G["ctx"]["scrolly"]
    G["SCREEN"].blit(G["HEL32"].render("DRAW RECT", 0, (0, 0, 0)), (0, G["SCREEN"].get_height() - 128))
    def draw_helper_(G):
        box_menu_draw(G)
        mpos = pygame.mouse.get_pos()
        G["SCREEN"].blit(G["HEL16"].render("{}".format((mpos[0] // 4, mpos[1] // 4)), 0, (0, 0, 0)), mpos)
    inp = expect_click(G, cb=draw_helper_)
    if inp is None: return None
    pos, btn = inp
    pos = pos[0] - scrollx, pos[1] - scrolly
    def draw_helper(G):
        draw_helper_(G)
        scrollx, scrolly = G["ctx"]["scrollx"], G["ctx"]["scrolly"]
        pos2 = pygame.mouse.get_pos()
        pos2 = pos2[0] - scrollx, pos2[1] - scrolly
        x1 = min(pos[0], pos2[0]) // 4
        x2 = max(pos[0], pos2[0]) // 4
        y1 = min(pos[1], pos2[1]) // 4
        y2 = max(pos[1], pos2[1]) // 4
        pygame.draw.rect(
            G["SCREEN"],
            col,
            Rect((x1*4+scrollx, y1*4+scrolly), ((x2 - x1)*4, (y2 - y1)*4)),
            width=2
        )
    inp = expect_click(G, draw_helper)
    if inp is None: return None
    pos2, btn2 = inp
    if not pos2: return None
    pos2 = pos2[0] - scrollx, pos2[1] - scrolly
    x1 = min(pos[0], pos2[0])
    x2 = max(pos[0], pos2[0])
    y1 = min(pos[1], pos2[1])
    y2 = max(pos[1], pos2[1])

    x1 -= x1 % 4
    y1 -= y1 % 4
    x2 -= x2 % 4
    y2 -= y2 % 4

    return (x1 // 4, y1 // 4), ((x2 - x1)//4, (y2 - y1)//4)
