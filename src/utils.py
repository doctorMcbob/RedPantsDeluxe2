import pygame
from pygame.locals import *
from pygame import Surface, Rect

NUMBERS_ONLY = {
    K_0: "0", K_1: "1", K_2: "2", K_3: "3", K_4: "4",
    K_5: "5", K_6: "6", K_7: "7", K_8: "8", K_9: "9",
}

ALPHABET_KEY_MAP = {
    K_a: "a", K_b: "b", K_c: "c", K_d: "d", K_e: "e",
    K_f: "f", K_g: "g", K_h: "h", K_i: "i", K_j: "j",
    K_k: "k", K_l: "l", K_m: "m", K_n: "n", K_o: "o",
    K_p: "p", K_q: "q", K_r: "r", K_s: "s", K_t: "t",
    K_u: "u", K_v: "v", K_w: "w", K_x: "x", K_y: "y",
    K_z: "z", K_SPACE: " ", K_UNDERSCORE: "_",
    K_0: "0", K_1: "1", K_2: "2", K_3: "3", K_4: "4",
    K_5: "5", K_6: "6", K_7: "7", K_8: "8", K_9: "9",
    K_PLUS: "+", K_MINUS: "-", K_SEMICOLON: ";", K_PERIOD:".",
    K_LEFTPAREN: "(", K_RIGHTPAREN: ")", K_COMMA: ",",
    K_ASTERISK: "*", K_SLASH: "/"
}
ALPHABET_SHIFT_MAP = {
    K_0: ")", K_1: "!", K_2: "@", K_3: "#", K_4: "$",
    K_5: "%", K_6: "^", K_7: "&", K_8: "*", K_9: "(",
    K_SEMICOLON: ":"
}

def get_text_input(G, pos, numeric=False):
    string = ''
    KEY_MAP = NUMBERS_ONLY if numeric else ALPHABET_KEY_MAP
    while True:
        surf = Surface((256, 32))
        surf.fill((230, 230, 230))
        surf.blit(G["HEL32"].render(string, 0, (0, 0, 0)), (0, 0))
        G["SCREEN"].blit(surf, pos)
        pygame.display.update()

        inp = expect_input()
        if inp == K_ESCAPE: return None
        if inp == K_BACKSPACE: string = string[:-1]
        if inp == K_RETURN: return int(string) if numeric else string 

        if pygame.key.get_mods() & KMOD_SHIFT and not numeric:
            if inp in ALPHABET_SHIFT_MAP:
                string = string + ALPHABET_SHIFT_MAP[inp]
            elif inp in KEY_MAP:
                string = string + KEY_MAP[inp].upper()
        elif inp in KEY_MAP:
            string = string + KEY_MAP[inp]

def expect_click(args=None, cb=lambda *args: None):
    while True:
        cb(args)
        pygame.display.update()
        for e in pygame.event.get():
            if e.type == QUIT or e.type == KEYDOWN and e.key == K_ESCAPE: quit()
            if e.type == MOUSEBUTTONDOWN:
                return e.pos

def expect_input(expectlist=[], args=None, cb=lambda *args:None):
    cb(args)
    while True:
        pygame.display.update()
        for e in pygame.event.get():
            if e.type == QUIT:
                return None
            if e.type == KEYDOWN:
                if expectlist:
                    if e.key in expectlist: return e.key
                else: return e.key

def select_from_list(G, list, pos, args=None, cb=lambda *args: None):
    idx = 0
    while True:
        surf = Surface((256, 32*len(list)))
        surf.fill((230, 230, 230))
        cb(args)
        for i, text in enumerate(list):
            col = (0, 0, 0) if i != idx else (160, 110, 190)
            surf.blit(G["HEL32"].render(str(text), 0, col), (0, i*32))
        G["SCREEN"].blit(surf, (pos[0], pos[1] - idx*32))
        inp = expect_input()

        if inp == K_UP: idx -= 1
        if inp == K_DOWN: idx += 1
        if inp in [K_RETURN, K_SPACE]: return list[idx]
        if inp in [K_ESCAPE, K_BACKSPACE] or not list: return False
        idx %= len(list)

