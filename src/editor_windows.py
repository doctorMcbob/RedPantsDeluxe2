import pygame
from pygame import Surface, Rect

from src import utils

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
    pygame.draw.rect(window["BODY"], theme["MENU_BG"], header)
    pygame.draw.rect(window["BODY"], (255, 80, 80), close_button)
    pygame.draw.rect(window["BODY"], theme["MENU_BG_ALT"], header, width=1)
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
                WINDOWS.pop(window["NAME"])
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

def make_text_entry_window(G, text, starting_text="",  on_entry=off):
    def text_entry_window_callback(G, window):
        window_base_update(G, window)
        if "TEXT" not in window:
            window["TEXT"] = starting_text

        rendered_text = G["HEL32"].render(
            text,
            0,
            THEMES.get(window["THEME"])["MENU_TXT"]
        )

        rendered_search = G["HEL16"].render(
            window["TEXT"],
            0,
            THEMES.get(window["THEME"])["MENU_TXT"]
        )

        rendered_submit = G["HEL16"].render(
            "Submit",
            0,
            THEMES.get(window["THEME"])["MENU_TXT"]
        )

        window["BODY"].blit(rendered_text, (32, 48))
        pygame.draw.rect(
            window["BODY"],
            THEMES.get(window["THEME"])["MENU_BG_SEL"],
            Rect((32, 80), (128, 16))
        )
        window["BODY"].blit(rendered_search, (32, 80))
        pygame.draw.rect(
            window["BODY"],
            THEMES.get(window["THEME"])["MENU_BG_ALT"],
            Rect((32, 112), (64, 16))
        )
        window["BODY"].blit(rendered_submit, (38, 112))

    def text_entry_window_event_handler(e, G, window):
        mpos = pygame.mouse.get_pos()
        if e.type == pygame.MOUSEBUTTONDOWN:
            if Rect(
                    (window["POS"][0] + 32, window["POS"][1] + 96), (64, 32)
            ).collidepoint(mpos):
                WINDOWS.pop(window["NAME"])
                on_entry(G, window["TEXT"])

        if e.type == pygame.KEYDOWN:
            if e.key == pygame.K_RETURN:
                WINDOWS.pop(window["NAME"])
                on_entry(G, text)

            if e.key == pygame.K_BACKSPACE: window["TEXT"] = window["TEXT"][:-1]
            if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                if e.key in utils.ALPHABET_SHIFT_MAP:
                    window["TEXT"] = window["TEXT"] + utils.ALPHABET_SHIFT_MAP[e.key]
                elif e.key in utils.ALPHABET_KEY_MAP:
                    window["TEXT"] = window["TEXT"] + utils.ALPHABET_KEY_MAP[e.key].upper()
            elif e.key in utils.ALPHABET_KEY_MAP:
                window["TEXT"] = window["TEXT"] + utils.ALPHABET_KEY_MAP[e.key]

    return text_entry_window_callback, text_entry_window_event_handler
