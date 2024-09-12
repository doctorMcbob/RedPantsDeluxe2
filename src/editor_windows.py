import pygame
from pygame import Surface, Rect

from src import editor
from src import utils
from src.utils import THEMES
from src import sprites
from src import scripts

WINDOWS = {}

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

def make_actor_edit_window(G, actor_template):
    sprite_map = sprites.get_sprite_map(actor_template["sprites"])
    script_map = scripts.get_script_map(actor_template["scripts"])
    states = set()
    for key in list(sprite_map.keys()) + list(script_map.keys()):
        if ":" not in key: continue
        state, frame = key.split(":")
        states.add(state)
    states = list(states)

    def actor_edit_window_callback(G, window):
        window_base_update(G, window)
        for key, default in [("STATE", "START"), ("FRAME", 0)]: #required keys
            if key not in window:
                window[key] = default
        sprite_name = sprite_map.get(f"{window['STATE']}:{window['FRAME']}")
        sprite = sprites.get_sprite(sprite_name)
        f = window["FRAME"]
        while sprite is None:
            f -= 1
            if f < 0: break
            sprite_name = sprite_map.get(f"{window['STATE']}:{f}")
            sprite = sprites.get_sprite(sprite_name)

        if sprite is not None:
            window["BODY"].blit(sprite, (16, 48))

        state_text = G["HEL16"].render(window["STATE"], 0, THEMES[window["THEME"]]["MENU_TXT"])
        frame_text = G["HEL16"].render(f"{window['FRAME']}", 0, THEMES[window["THEME"]]["MENU_TXT"])
        state_left_btn = Rect((16, 256), (16, 16))
        state_right_btn = Rect((48 + state_text.get_width(), 256), (16, 16))
        frame_left_btn = Rect((16, 288), (16, 16))
        frame_right_btn = Rect((48 + frame_text.get_width(), 288), (16, 16))
        window["STATE_L_BTN"] = state_left_btn
        window["STATE_R_BTN"] = state_right_btn
        window["FRAME_L_BTN"] = frame_left_btn
        window["FRAME_R_BTN"] = frame_right_btn

        window["BODY"].blit(state_text, (32, 256))
        window["BODY"].blit(frame_text, (32, 288))

        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_SEL"], state_left_btn)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], state_left_btn, width=2)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_SEL"], state_right_btn)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], state_right_btn, width=2)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_SEL"], frame_left_btn)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], frame_left_btn, width=2)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_SEL"], frame_right_btn)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], frame_right_btn, width=2)
        x, y = 256, 48

        tangible_button = Rect((x, y), (16, 16))
        window["TANG_BTN"] = tangible_button
        tangible_text = G["HEL16"].render(
            f"Tangible: {actor_template['tangible']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        color = "MENU_BG_SEL" if actor_template["tangible"] else "MENU_BG"
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]][color], tangible_button)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], tangible_button, width=2)
        window["BODY"].blit(tangible_text, (x + 32, y))

        y += 32

        physics_button = Rect((x, y), (16, 16))
        window["PHYS_BTN"] = physics_button
        physics_text = G["HEL16"].render(
            f"Physics: {actor_template['physics']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        color = "MENU_BG_SEL" if actor_template["physics"] else "MENU_BG"
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]][color], physics_button)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], physics_button, width=2)
        window["BODY"].blit(physics_text, (x + 32, y))

        y += 32

        direction_button = Rect((x, y), (16, 16))
        window["DIR_BTN"] = direction_button
        direction_text = G["HEL16"].render(
            f"Direction: {actor_template['direction']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        color = "MENU_BG_SEL" if actor_template["direction"] else "MENU_BG"
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]][color], direction_button)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], direction_button, width=2)
        window["BODY"].blit(direction_text, (x + 32, y))

        y += 32

        sprites_text = G["HEL16"].render(
            f"Sprites: {actor_template['sprites']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        window["BODY"].blit(sprites_text, (x + 32, y))

        y += 32

        scripts_text = G["HEL16"].render(
            f"Scripts: {actor_template['scripts']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        window["BODY"].blit(scripts_text, (x + 32, y))

    def actor_edit_window_event_handler(e, G, window):
        mpos = pygame.mouse.get_pos()
        mpos = (mpos[0] - window["POS"][0], mpos[1] - window["POS"][1])
        update = False
        if e.type == pygame.MOUSEBUTTONDOWN:
            update = True
            if window["TANG_BTN"].collidepoint(mpos):
                actor_template["tangible"] = not actor_template["tangible"]
            elif window["PHYS_BTN"].collidepoint(mpos):
                actor_template["physics"] = not actor_template["physics"]
            elif window["DIR_BTN"].collidepoint(mpos):
                actor_template["direction"] = -1 if actor_template["direction"] == 1 else 1
            else:
                update = False
            if window["STATE_L_BTN"].collidepoint(mpos):
                window["STATE"] = states[(states.index(window["STATE"]) - 1) % len(states)]
                window["FRAME"] = 0
            elif window["STATE_R_BTN"].collidepoint(mpos):
                window["STATE"] = states[(states.index(window["STATE"]) + 1) % len(states)]
                window["FRAME"] = 0
            elif window["FRAME_L_BTN"].collidepoint(mpos):
                window["FRAME"] = window["FRAME"] - 1
            elif window["FRAME_R_BTN"].collidepoint(mpos):
                window["FRAME"] = window["FRAME"] + 1 

        if update:
            editor.load_game()
    return actor_edit_window_callback, actor_edit_window_event_handler

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
