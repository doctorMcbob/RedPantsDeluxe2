import pygame
from pygame import Surface, Rect
from pygame.locals import *

import sys
from copy import deepcopy

from src.inputs import DEFAULT_MAP, DEFAULT_CONTROLLER_MAP

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
                    print(len(players))
                    G["INPUTS"].add_state(
                        "PLAYER{}".format(len(players)), inp_map=players[-1], joy=joy)
                elif btn == DEFAULT_CONTROLLER_MAP["START"]: inmenu = False

        for name in DEFAULT_MAP.keys():
            key = DEFAULT_MAP[name] 
            if keys[key] and "key" not in registered:
                registered.append("key")
                players.append(deepcopy(DEFAULT_MAP))
                G["INPUTS"].add_state(
                    "PLAYER{}".format(len(players)), inp_map=players[-1])

        if "key" in registered and keys[DEFAULT_MAP["START"]]:
            inmenu = False

    if len(players) == 1:
        G["FRAMES"].add_frame("MAIN", G["ROOT"], (G["W"], G["H"]))
        G["FRAMEMAP"] = {
            "MAIN": (0, 0),
        }        
    elif len(players) == 2:
        G["FRAMES"].add_frame("MAIN", G["ROOT"], (G["W"]//2, G["H"]))
        G["FRAMES"].add_frame("MAIN2", G["ROOT"], (G["W"]//2, G["H"]))
        G["FRAMEMAP"] = {
            "MAIN": (0, 0),
            "MAIN2": (G["W"]//2, 0)
        }
    elif len(players) == 3:
        G["FRAMES"].add_frame("MAIN", G["ROOT"], (G["W"], G["H"]//2))
        G["FRAMES"].add_frame("MAIN2", G["ROOT"], (G["W"]//2, G["H"]//2))
        G["FRAMES"].add_frame("MAIN3", G["ROOT"], (G["W"]//2, G["H"]//2))
        G["FRAMEMAP"] = {
            "MAIN": (0, 0),
            "MAIN2": (0, G["H"] // 2),
            "MAIN3": (G["W"] // 2, G["H"] // 2)
        }
    elif len(players) >= 4:
        G["FRAMES"].add_frame("MAIN", G["ROOT"], (G["W"]//2, G["H"]//2))
        G["FRAMES"].add_frame("MAIN2", G["ROOT"], (G["W"]//2, G["H"]//2))
        G["FRAMES"].add_frame("MAIN3", G["ROOT"], (G["W"]//2, G["H"]//2))
        G["FRAMES"].add_frame("MAIN4", G["ROOT"], (G["W"]//2, G["H"]//2))
        G["FRAMEMAP"] = {
            "MAIN": (0, 0),
            "MAIN2": (G["W"] // 2, 0),
            "MAIN3": (0, G["H"] // 2),
            "MAIN4": (G["W"] // 2, G["H"] // 2)
        }

