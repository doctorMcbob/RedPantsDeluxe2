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
from src.utils import expect_input, select_from_list, get_text_input

WORLDS = {}
SPRITESHEETS = {}
ACTORS = {}

WORLD_TEMPLATE = {"actors":[], "background":None}

SAVED = False

IMG_LOCATION = "img/"
SCRIPT_LOCATION = "scripts/"
X, Y = 0, 0
W, H = 1152, 640
CX, CY = 0, 0
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
    worlds.load()
    actor.load()
    
    G["FRAMES"] = frames
    G["WORLDS"] = worlds
    G["ACTOR"] = actor
    G["CLOCK"] = pygame.time.Clock()
    load()

    make_single_player(G)
        
    return G

def draw(G):
    G["SCREEN"].fill((255, 255, 255))
    frame = frames.get_frame("EDITOR_VIEW")
    frame.scroll_x = CX
    frame.scroll_y = CY
    
    world = G["WORLDS"].get_world(G["WORLD"])
    frame.world = world
    drawn = frame.drawn()
    pygame.draw.rect(drawn, (255, 0, 0), Rect(make_rect((CORNER[0]-CX, CORNER[1]-CY) if CORNER is not None else (X-CX+16, Y-CY+16), (X-CX, Y-CY))), width=2)
    G["SCREEN"].blit(drawn, (0, 0))
    G["SCREEN"].blit(G["HEL32"].render("WORLD: {}".format(G["WORLD"]), 0, (0, 0, 0)), (0, G["SCREEN"].get_height() - 32))

def run(G):
    global CORNER, X, Y, CX, CY, SPLITSCREEN
    while True:
        draw(G)
        inp = expect_input()
        mods = pygame.key.get_mods()
        if inp == K_ESCAPE and (SAVED or mods & KMOD_CTRL):
            sys.exit()


        if inp == K_LEFT and mods & KMOD_SHIFT: CX += 64
        elif inp == K_LEFT: X -= 32

        if inp == K_UP and mods & KMOD_SHIFT: CY += 64
        elif inp == K_UP: Y -= 32

        if inp == K_RIGHT and mods & KMOD_SHIFT: CX -= 64
        elif inp == K_RIGHT: X += 32

        if inp == K_DOWN and mods & KMOD_SHIFT: CY -= 64
        elif inp == K_DOWN: Y += 32

        if inp == K_o and mods & KMOD_CTRL:
            load()

        if inp == K_a and mods & KMOD_SHIFT:
            def update(G):
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Edit Actor:", 0, (0, 0, 0)),
                    (0, 0)
                )
            actor_keys = WORLDS[G["WORLD"]]["actors"]
            if not actor_keys: continue
            choice = select_from_list(G, actor_keys, (0, 32), args=G, cb=update)
            if not choice: continue
            actor_menu(G, ACTORS[choice])
            
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
                
        if inp == K_RETURN:
            save()
            sprites.load()
            G["ACTOR"].load()
            G["WORLDS"].load()
            game.run(G, noquit=True)
            sprites.load()
            G["ACTOR"].load()
            G["WORLDS"].load()

        if inp == K_s and mods & KMOD_CTRL:
            save()
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

def save():
    global SAVED
    with open("src/lib/WORLDS.py", "w+") as f:
        f.write("WORLDS = {}".format(repr(WORLDS)))
    with open("src/lib/SPRITESHEETS.py", "w+") as f:
        f.write("SPRITESHEETS = {}".format(repr(SPRITESHEETS)))
    with open("src/lib/ACTORS.py", "w+") as f:
        f.write("ACTORS = {}".format(repr(ACTORS)))

    SAVED = True

def load():
    global WORLDS, SPRITESHEETS, ACTORS
    from src.lib import WORLDS as W
    from src.lib import SPRITESHEETS as S
    from src.lib import ACTORS as A
    WORLDS = W.WORLDS
    SPRITESHEETS = S.SPRITESHEETS
    ACTORS = A.ACTORS

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
    
    spriteoffset = [int(n) for n in segments.pop(0).split(",")]    
    
    sprites = {}
    for line in segments.pop(0).splitlines():
        if not line: continue
        key, sprite = line.split()
        sprites[key] = sprite
    
    scripts = {}
    while segments:
        key = segments.pop(0)
        cmds = segments.pop(0)
        scripts[key] = cmds.splitlines()
        
    return {
        "name": name,
        "POS": (x, y),
        "DIM": (w, h),
        "spriteoffset": spriteoffset,
        "sprites": sprites,
        "scripts": scripts,
        "tangible": tangible == "True"
    }
    
def drawn_spritesheet_data(G, d, idx=None):
    keys = d.keys()
    surf = Surface((512, (len(d.keys()) + 1) * 16))
    surf.fill((255, 255, 255))
    offset = 0 if idx < 20 else 20 * 16
        
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
                if name is not None:
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
    keys = ["name", "POS", "DIM", "spriteoffset"]
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

def index_template(template, idx):
    "returns (type, data)"
    i = 0
    keys = ["name", "POS", "DIM", "spriteoffset"]
    for key in keys:
        if i == idx:
            return 'key', key
        i += 1
    for sprite in list(template["sprites"].keys()):
        if i == idx:
            return 'sprite', sprite
        i += 1
    for key in list(template["scripts"].keys()):
        if i == idx:
            return 'script name', key
        i += 1
        for _i, cmd in enumerate(template["scripts"][key]):
            if i == idx:
                return 'cmd', '{}|{}'.format(key, _i)
            i += 1
    return None, None

def actor_menu(G, actor_template):
    scroll = 0
    idx = 0
    while True:
        draw(G)
        G["SCREEN"].blit(drawn_actor_template(G, actor_template, scroll=scroll, idx=idx), (250, 0))

        inp = expect_input()
        mods = pygame.key.get_mods()

        if inp == K_ESCAPE and mods & KMOD_CTRL: sys.exit()
        if inp == K_ESCAPE: return

        if inp == K_DOWN and mods & KMOD_SHIFT: scroll -= 64
        elif inp == K_DOWN: idx += 1
        if inp == K_UP and mods & KMOD_SHIFT: scroll += 64
        elif inp == K_UP: idx -= 1

        if inp == K_s and mods & KMOD_SHIFT:
            draw(G)
            G["SCREEN"].blit(
                G["HEL32"].render("Save as? /scripts/{your input}.rp", 0, (0, 0, 0)),
                (0, 0))
            filename = get_text_input(G, (0, 32))
            if not filename: continue
            try:
                with open(SCRIPT_LOCATION + filename + ".rp", "w") as f:
                    f.write(export_as_rp_script(actor_template))
            except IOError:
                continue
            
        if inp in [K_SPACE, K_BACKSPACE]:
            data_type, data = index_template(actor_template, idx)
            if data_type == "key":
                if data == "name":
                    draw(G)
                    G["SCREEN"].blit(
                        G["HEL32"].render("New name", 0, (0, 0, 0)),
                        (0, 0)
                    )
                    new = get_text_input(G, (0, 32))
                    if not new: continue
                    actor_template[data] = new
                else:
                    draw(G)
                    G["SCREEN"].blit(
                        G["HEL32"].render("(A, B) A:", 0, (0, 0, 0)),
                        (0, 0)
                    )
                    A = get_text_input(G, (0, 32), numeric=True)
                    if not A: continue
                    draw(G)
                    G["SCREEN"].blit(
                        G["HEL32"].render("(A, B) B:", 0, (0, 0, 0)),
                        (0, 0)
                    )
                    B = get_text_input(G, (0, 32), numeric=True)
                    if not B: continue
                    actor_template[data] = (A, B)
                        
            if data_type == "sprite":
                def update(G):
                    draw(G)
                    G["SCREEN"].blit(
                        G["HEL32"].render("Select sprite:", 0, (0, 0, 0)),
                        (0, 0)
                    )
                sprite_keys = []
                for key in SPRITESHEETS.keys():
                    for sprite in SPRITESHEETS[key].keys():
                        sprite_keys.append(sprite)
                if not sprite_keys: continue
                choice = select_from_list(G, sprite_keys, (0, 32), args=G, cb=update)
                if not choice: continue
                actor_template["sprites"][data] = choice

            if data_type == "script name":
                draw(G)
                G["SCREEN"].blit(
                    G["HEL32"].render("Rename Code Block", 0, (0, 0, 0)),
                    (0, 0)
                )
                new = get_text_input(G, (0, 32))
                if not new: continue
                actor_template["scripts"][new] = actor_template["scripts"].pop(data)

            if data_type == "cmd":
                script, index = data.split("|")
                try:
                    index = int(index)
                except ValueError:
                    continue
                if inp == K_SPACE:
                    draw(G)
                    G["SCREEN"].blit(
                        G["HEL32"].render("CMD", 0, (0, 0, 0)),
                        (0, 0)
                    )
                    new = get_text_input(G, (0, 32))
                    if not new: continue
                    actor_template["scripts"][script].insert(index+1, new)
                if inp == K_BACKSPACE:
                    actor_template["scripts"][script].pop(index)

def export_as_rp_script(template):
    name = template["name"]
    x, y = template["POS"]
    w, h = template["DIM"]
    tangible = template["tangible"]
    offx, offy = template["spriteoffset"]
    # look, im sorry
    RPS = "{}|{},{},{},{}|{}|{},{}|".format(name, x, y, w, h, tangible, offx, offy)
    for key in template["sprites"]:
        RPS += "\n{} {}".format(key, template["sprites"][key])
    for key in template["scripts"]:
        RPS += "\n|{}|\n{}\n".format(key, "\n".join(template["scripts"][key]))
    return RPS

