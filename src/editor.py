import pygame
from pygame.locals import *
from pygame import Surface, Rect

import sys
import os

from src.utils import expect_input, select_from_list, get_text_input

WORLDS = {}
SPRITESHEETS = {}
ACTORS = {}

SAVED = False

IMG_LOCATION = "img/"


def set_up():
    pygame.init()
    G = {}
    G["SCREEN"] = pygame.display.set_mode((1200, 720))
    G["HEL16"] = pygame.font.SysFont("Helvetica", 16)
    G["HEL32"] = pygame.font.SysFont("Helvetica", 32)

    return G

def draw(G):
    G["SCREEN"].fill((255, 255, 255))

def run(G):
    while True:
        draw(G)
        inp = expect_input()
        mods = pygame.key.get_mods()
        if inp == K_ESCAPE and (SAVED or mods & KMOD_CTRL):
            sys.exit()

        if inp == K_s and mods & KMOD_CTRL:
            save()

        if inp == K_o and mods & KMOD_CTRL:
            load()

        if inp == K_s and mods & KMOD_SHIFT:
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
            if choice is not None: spritesheet_menu(G, choice)

def save():
    global SAVED
    with open("src/lib/WORLDS.py", "w+") as f:
        f.write("WORLDS = {}".format(repr(WORLDS)))
    with open("src/lib/SPRITESHEETS.py", "w+") as f:
        f.write("SPRITESHEETS = {}".format(repr(SPRITESHEETS)))
    with open("src/lib/WORLDS.py", "w+") as f:
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
                sheet[get_text_input(G, (0, 0))] = make_rect(corner, (CX, CY))
                keys = list(sheet.keys())
