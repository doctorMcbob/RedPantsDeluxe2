import pygame
from pygame import Surface, Rect

WINDOWS = {}

THEMES = {
    "SOULLESS": {
        "MENU_BG" : (100, 100, 100),
        "MENU_BG_ALT" : (200, 200, 200),
        "MENU_BG_SEL"  : (150, 150, 150),
        "MENU_TXT" : (210, 210, 255),
        "MENU_TXT_SEL" : (255, 200, 200),
    },
    "FUNKY": {
        "MENU_BG" : (90, 36, 115),
        "MENU_BG_ALT" : (190, 106, 200),
        "MENU_BG_SEL"  : (115, 59, 145),
        "MENU_TXT" : (210, 215, 30),
        "MENU_TXT_SEL" : (180, 190, 35),
    }
}

def activate_window(name):
    WINDOWS.get(name, {})["ACTIVE"] = True

def window_base_update(G, window):
    pos, dim = window["POS"], window["BODY"].get_size()
    mpos = pygame.mouse.get_pos()
    theme = THEMES[window["THEME"]]

    header = Rect((4, 4), (dim[0] - 28, 28))
    close_button = Rect((0 + dim[0]-32, 0), (32, 32))

    window["BODY"].fill(theme["MENU_BG"])
    pygame.draw.rect(window["BODY"], theme["MENU_BG_ALT"], header)
    pygame.draw.rect(window["BODY"], (255, 80, 80), close_button)
    text = G["HEL32"].render(window["NAME"], 0, theme["MENU_TXT"])
    X = G["HEL32"].render("X", 0, theme["MENU_TXT"])
    window["BODY"].blit(text, (4, 0))
    window["BODY"].blit(X, (close_button.x, close_button.y))

def off(*args, **kwargs): pass

def add_window(name, pos, dim, sys=False, theme="SOULLESS", update_callback=off, event_callback=off, args=[], kwargs={}):
    WINDOWS[name] = {
        "NAME": name,
        "BODY": Surface(dim),
        "POS": pos,
        "THEME": theme,
        "UPDATE": lambda : update_callback(*args + [WINDOWS.get(name)], **kwargs),
        "EVENTS": lambda e: event_callback(*([e] + [*args] + [WINDOWS.get(name)]), **kwargs),
        "ACTIVE": False,
        "SYS": sys,
        "DRAG": False,
    }

def update_windows(G):
    for window in WINDOWS.values():
        if window["ACTIVE"]:
            window["UPDATE"]()

            G["SCREEN"].blit(window["BODY"], window["POS"])


def window_at_mouse():
    pos = pygame.mouse.get_pos()
    # scan through backwards so that "top" gets clicked first
    keys = list(WINDOWS.keys())
    for name in keys[::-1]:
        if not WINDOWS[name]["ACTIVE"]:
            continue
        if WINDOWS[name]["DRAG"]:
            return WINDOWS[name]
        if Rect(WINDOWS[name]["POS"], WINDOWS[name]["BODY"].get_size()).collidepoint(pos):
            return WINDOWS[name]
    return None
        
def handle_window_events(G, e, window):
    pos, dim = window["POS"], window["BODY"].get_size()
    mpos = pygame.mouse.get_pos()

    real_header = Rect(pos, (dim[0] - 32, 32))
    real_close_button = Rect((pos[0] + dim[0]-32, pos[1]), (32, 32))

    if e.type == pygame.MOUSEBUTTONDOWN:
        if real_close_button.collidepoint(mpos):
            if window["SYS"]:
                window["ACTIVE"] = False
            else:
                WINDOWS.pop(name)
            return

        elif real_header.collidepoint(mpos):
            window["DRAG"] = True

    if e.type == pygame.MOUSEMOTION and window["DRAG"]:
        window["POS"] = (
            window["POS"][0] + e.rel[0],
            window["POS"][1] + e.rel[1],
        )

    if e.type == pygame.MOUSEBUTTONUP:
        window["DRAG"] = False
        window["POS"] = (
            max(
                0,
                min(
                    G["SCREEN"].get_width()-window["BODY"].get_width(),
                    window["POS"][0] // 16 * 16
                )
            ),
            max(
                32,
                min(
                    G["SCREEN"].get_height()-window["BODY"].get_height(),
                    window["POS"][1] // 16 * 16,
                )
            )
        )

    window["EVENTS"](e)
