import pygame
from pygame import Surface, Rect
from pygame.locals import *

import sys
from copy import deepcopy

from src.inputs import DEFAULT_MAP, DEFAULT_CONTROLLER_MAP
from src import inputs
from src import frames

def run_controller_menu(G, cb=lambda *args: None, args=None, noquit=False):
    joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]
    players = []
    registered = []
    inmenu = True
    while inmenu:
        surf = Surface((G["SCREEN"].get_width()-64, G["SCREEN"].get_height()-64))
        surf.fill((0, 0, 0))
        pygame.draw.rect(surf, (150, 150, 150),
                         Rect((16, 16), (surf.get_width()-32, surf.get_height()-32)))
        surf.blit(G["HEL32"].render("Press Any to Join!", 0, (0, 0, 0)), (16, 16))
        x, y = 32, 16+32
        for i in range(len(registered)):
            name = registered[i] if registered[i] == "key" else registered[i].get_name()
            surf.blit(G["HEL32"].render("PLAYER{} with {}".format(i+1, name), 0, (0, 0, 0)), (x, y))
            y += 32
        
        G["SCREEN"].fill((255, 255, 255))
        G["SCREEN"].blit(surf, (32, 32))
        pygame.display.update()
        pygame.event.pump()
        keys = pygame.key.get_pressed()
        mods = pygame.key.get_mods()
        if pygame.event.peek(QUIT) or keys[K_ESCAPE]:
            return None if noquit else sys.exit()

        for joy in joysticks:
            if not joy.get_init():
                joy.init()

            for btn in range(joy.get_numbuttons()):
                if not joy.get_button(btn): continue
                if joy not in registered:
                    registered.append(joy)
                    players.append(deepcopy(DEFAULT_CONTROLLER_MAP))
                    inputs.add_state(
                        "PLAYER{}".format(len(players)), inp_map=players[-1], joy=joy)
                elif btn == DEFAULT_CONTROLLER_MAP["START"]: inmenu = False

        for name in DEFAULT_MAP.keys():
            key = DEFAULT_MAP[name]
            force = False
            registered_keys = registered.count("key")
            force = (keys[K_2] and registered_keys < 2) or force
            force = (keys[K_3] and registered_keys < 3) or force
            force = (keys[K_4] and registered_keys < 4) or force
            if keys[key] and "key" not in registered or force:
                registered.append("key")
                players.append(deepcopy(DEFAULT_MAP))
                inputs.add_state(
                    "PLAYER{}".format(len(players)), inp_map=players[-1])

        if "key" in registered and keys[DEFAULT_MAP["START"]]:
            inmenu = False

    frames.add_frame("ROOT", G["ROOT"], (G["W"], G["H"]), pos=(0, 0))


