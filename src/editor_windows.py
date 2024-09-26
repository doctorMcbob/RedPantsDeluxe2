import pygame
from pygame import Surface, Rect

from src import editor
from src import utils
from src.utils import THEMES
from src import boxes
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

        y += 32

        box_edit_button = Rect((x + 128, y), (128, 64))
        window["BOX_EDIT_BTN"] = box_edit_button
        box_edit_text = G["HEL16"].render(
            f"Edit Boxes", 0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG"], box_edit_button)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], box_edit_button, width=2)
        window["BODY"].blit(box_edit_text, (x + 128 + 32, y + 16))
        
        
        hitbox_text = G["HEL16"].render(
            f"Hitboxes: {actor_template['hitboxes']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        window["BODY"].blit(hitbox_text, (x + 32, y))

        y += 32

        hurtbox_text = G["HEL16"].render(
            f"Hurtboxes: {actor_template['hurtboxes']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        window["BODY"].blit(hurtbox_text, (x + 32, y))
        
        
        
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
            elif window["BOX_EDIT_BTN"].collidepoint(mpos):
                cb, eh = make_box_draw_window(G, actor_template,
                                              None) #TODO... gulp
                add_window(
                    f"Boxes: {actor_template['name']}",
                    (window["POS"][0]+32, window["POS"][1] + 32), (640, 480),
                    theme=window["THEME"],
                    update_callback=cb,
                    event_callback=eh,
                    args=[G],
                )
                activate_window(f"Boxes: {actor_template['name']}")
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
                on_entry(G, window["TEXT"])

            if e.key == pygame.K_BACKSPACE: window["TEXT"] = window["TEXT"][:-1]
            if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                if e.key in utils.ALPHABET_SHIFT_MAP:
                    window["TEXT"] = window["TEXT"] + utils.ALPHABET_SHIFT_MAP[e.key]
                elif e.key in utils.ALPHABET_KEY_MAP:
                    window["TEXT"] = window["TEXT"] + utils.ALPHABET_KEY_MAP[e.key].upper()
            elif e.key in utils.ALPHABET_KEY_MAP:
                window["TEXT"] = window["TEXT"] + utils.ALPHABET_KEY_MAP[e.key]

    return text_entry_window_callback, text_entry_window_event_handler

def make_select_from_list(G, selectList, allowNew=False, on_select=off):
    def update(G, window):
        window_base_update(G, window)
        # enforced strings
        for s, default in [("SELECTED", None), ("SEARCH", ""), ("SCROLL", 0)]:
            if s not in window: window[s] = default
            mpos = pygame.mouse.get_pos()
        mpos = (
            mpos[0] - window["POS"][0] - 4,
            mpos[1] - window["POS"][1] - 36,
        )
        surf, selected = utils.scroller_list(
            selectList,
            mpos,
            (window["BODY"].get_width()-8,
             window["BODY"].get_height()-40),
            G["HEL16"],
            scroll=window["SCROLL"],
            search=window["SEARCH"],
            theme=window["THEME"],
            button="New..." if allowNew else False,
        )
        window["BODY"].blit(surf, (4, 36))
        window["SELECTED"] = selected

    def events(e, G, window):
        if e.type == pygame.MOUSEBUTTONDOWN:
            if e.button == 4: window["SCROLL"] -= 16
            if e.button == 5: window["SCROLL"] += 16
            if e.button == 1 and window["SELECTED"] is not None:
                return on_select(window["SELECTED"])

        if e.type == pygame.KEYDOWN:
            if e.key == pygame.K_UP: window["SCROLL"] += 128
            if e.key == pygame.K_DOWN: window["SCROLL"] -= 128

            if e.key == pygame.K_RETURN:
                if window["SEARCH"] in selectList:
                    return on_select(window["SEARCH"])

            if e.key == pygame.K_BACKSPACE: window["SEARCH"] = window["SEARCH"][:-1]
            if pygame.key.get_mods() & pygame.KMOD_SHIFT:
                if e.key in utils.ALPHABET_SHIFT_MAP:
                    window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_SHIFT_MAP[e.key]
                elif e.key in utils.ALPHABET_KEY_MAP:
                    window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_KEY_MAP[e.key].upper()
            elif e.key in utils.ALPHABET_KEY_MAP:
                window["SEARCH"] = window["SEARCH"] + utils.ALPHABET_KEY_MAP[e.key]

    return update, events

def make_box_draw_window(G, actor, boxkey):
    def box_draw_window_callback(G, window):
        window_base_update(G, window)
        for key, default in [("STATE", "START"), ("FRAME", 0),
                             ("sx", 64),         ("sy", 64),
                             ("SCROLL", False),  ("BOXKEY", boxkey)]:
            if key not in window: window[key] = default
        # draw actor sprite, 2x scaled
        sprite_map = sprites.get_sprite_map(actor["sprites"])
        sprite_name = sprite_map.get(f"{window['STATE']}:{window['FRAME']}")
        sprite = sprites.get_sprite(sprite_name)
        f = window["FRAME"]
        while sprite is None:
            f -= 1
            if f < 0: break
            sprite_name = sprite_map.get(f"{window['STATE']}:{f}")
            sprite = sprites.get_sprite(sprite_name)
        if sprite is not None:
            sprite = pygame.transform.scale2x(sprite)
            sprite = pygame.transform.scale2x(sprite)
            offx, offy = sprites.get_offset(actor["sprites"], sprite_name)
            sx, sy = window["sx"], window["sy"]
            window["BODY"].blit(sprite, (sx + (offx * 4), sy + (offy * 4)))
        # draw 16,16 grid
        w, h = window["BODY"].get_size()
        h -= 32 # header
        for x in range(w // 16):
            pygame.draw.line(window["BODY"], (100,100,100), (x*16, 32), (x*16, h+32))
        for y in range(h // 16):
            pygame.draw.line(window["BODY"], (100,100,100), (0, y*16+32), (w, y*16+32))
        # draw ECB, hitboxes, hurtboxes
        hitboxes = boxes.get_hitbox_map(window["BOXKEY"])
        hurtboxes = boxes.get_hurtbox_map(window["BOXKEY"])
        pygame.draw.rect(
            window["BODY"], (0, 0, 255),
            Rect((window["sx"], window["sy"]),
                 (actor["DIM"][0]*4, actor["DIM"][1]*4)),
            width=2
        )
        for boxmap, color in [(hitboxes, (255, 0, 0)), (hurtboxes, (0, 255, 0))]:
            if boxmap is None: continue
            boxs = boxmap.get(f"{window['STATE']}:{window['FRAME']}")
            f = window["FRAME"]
            while boxs is None:
                f -= 1
                if f < 0: break
                boxs = boxmap.get(f"{window['STATE']}:{f}")
            if boxs is not None:
                for pos, dim in boxs:
                    pygame.draw.rect(
                        window["BODY"], color,
                        Rect(
                            (pos[0] * 4 + window["sx"], pos[1] * 4 + window["sy"]),
                            (dim[0] * 4, dim[1] * 4)
                        ),
                        width=2
                    )
        # draw buttons (hit/hurt)
        boxmap_button_text = G["HEL16"].render(
            'Select' if window["BOXKEY"] is None else window["BOXKEY"],
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        boxmap_button_rect = Rect((0, 32), (128, 32))
        window["BOXMAP_BTN"] = boxmap_button_rect
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG"], boxmap_button_rect)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], boxmap_button_rect, width=2)
        window["BODY"].blit(boxmap_button_text, (8, 40))
        state_button_text = G["HEL16"].render(
            window["STATE"],
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        state_button_rect = Rect((128, 32), (64, 32))
        window["STATE_BTN"] = state_button_rect
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG"], state_button_rect)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], state_button_rect, width=2)
        window["BODY"].blit(state_button_text, (136, 40))
        frame_l_button_rect = Rect((200, 40), (16, 16))
        window["FRAME_L_BTN"] = frame_l_button_rect
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG"], frame_l_button_rect)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], frame_l_button_rect, width=2)
        frame_text = G["HEL16"].render(
            f"{window['FRAME']}",
            0, THEMES[window["THEME"]]["MENU_TXT"]
        )
        window["BODY"].blit(frame_text, (224, 32))
        frame_r_button_rect = Rect((232 + frame_text.get_width(), 40), (16, 16))
        window["FRAME_R_BTN"] = frame_r_button_rect
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG"], frame_r_button_rect)
        pygame.draw.rect(window["BODY"], THEMES[window["THEME"]]["MENU_BG_ALT"], frame_r_button_rect, width=2)

    def box_draw_event_handler(e, G, window):
        mpos = pygame.mouse.get_pos()
        mpos = (mpos[0] - window["POS"][0], mpos[1] - window["POS"][1])
        if e.type == pygame.MOUSEBUTTONDOWN:
            if e.button == 3:
                window["SCROLL"] = True
            if e.button == 1:
                if window["STATE_BTN"].collidepoint(mpos):
                    name = f"Select State For Boxmap"
                    def state_select_callback(G, entry):
                        if entry:
                            window["STATE"] = entry
                            window["FRAME"] = 0
                    update, events = make_text_entry_window(
                        G, "Enter State:",
                        on_entry=state_select_callback
                    )
                    add_window(
                        name,
                        (window["POS"][0]+16, window["POS"][1]+16),
                        (256, 256),
                        update_callback=update,
                        event_callback=events,
                        args=[G],
                        theme=window["THEME"],
                    )
                    activate_window(name)
                elif window["FRAME_L_BTN"].collidepoint(mpos):
                    window["FRAME"] = window["FRAME"] - 1
                elif window["FRAME_R_BTN"].collidepoint(mpos):
                    window["FRAME"] = window["FRAME"] + 1 
                elif window["BOXMAP_BTN"].collidepoint(mpos):
                    name = f"Box map for {actor['name']}"
                    def selector_callback(selection):
                        WINDOWS.pop(name)
                        if selection == "New...":
                            _name = "Boxman Name Entry"
                            def add_new(G, entry):
                                if entry:
                                    window["BOXKEY"] = entry
                                    if entry not in editor.HITBOXES:
                                        editor.HITBOXES[entry] = {}
                                    if entry not in editor.HURTBOXES:
                                        editor.HURTBOXES[entry] = {}
                                    editor.load_game()
                            update, events = make_text_entry_window(
                                G, "Enter name:",
                                on_entry=add_new
                            )
                            add_window(
                                name,
                                (window["POS"][0]+16, window["POS"][1]+16),
                                (256, 256),
                                update_callback=update,
                                event_callback=events,
                                args=[G],
                                theme=window["THEME"],
                            )
                            activate_window(name)
                        else:
                            window["BOXKEY"] = selection
                        
                    update, events = make_select_from_list(
                        G, boxes.get_hitbox_maps(),
                        True, selector_callback
                    )
                    add_window(
                        name,
                        (window["POS"][0]+16, window["POS"][1]+16),
                        (256, 256),
                        update_callback=update,
                        event_callback=events,
                        args=[G],
                        theme=window["THEME"],
                    )
                    activate_window(name)

        if e.type == pygame.MOUSEMOTION:
            if window["SCROLL"]:
                window["sx"] += e.rel[0]
                window["sy"] += e.rel[1]

        if e.type == pygame.MOUSEBUTTONUP:
            if e.button == 3:
                window["SCROLL"] = False
                window["sx"] = window["sx"] // 16 * 16
                window["sy"] = window["sy"] // 16 * 16

    return box_draw_window_callback, box_draw_event_handler
