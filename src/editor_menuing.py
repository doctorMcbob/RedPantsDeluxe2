import pygame
from pygame import Surface, Rect

class MenuHeader:
    # THEMES
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
    # when a click happens this will be True otherwise it will be False
    CLICK = False

    def __init__(self, destination, template, pos=(0, 0), theme="SOULLESS"):
        self.HEL16 = pygame.font.SysFont("Helvetica", 16)

        self.menu_items = template
        self.pos = pos
        self.destination = destination
        # Active submenus
        # key "File", value { ... } or Callable
        self.sub_menus = {}
        self.path = set()
        self.set_theme(theme)
        
    def set_theme(self, name):
        self.MENU_BG = MenuHeader.THEMES[name]["MENU_BG"]
        self.MENU_BG_ALT = MenuHeader.THEMES[name]["MENU_BG_ALT"]
        self.MENU_BG_SEL = MenuHeader.THEMES[name]["MENU_BG_SEL"] 
        self.MENU_TXT = MenuHeader.THEMES[name]["MENU_TXT"]
        self.MENU_TXT_SEL = MenuHeader.THEMES[name]["MENU_TXT_SEL"]
        
    def draw_header(self):
        w = self.destination.get_width()
        surf = Surface((w, 32))
        surf.fill(self.MENU_BG)
        w_ = 4
        mpos = pygame.mouse.get_pos()

        picked = None, None

        for item in self.menu_items:
            text = self.HEL16.render(item, 0, self.MENU_TXT)
            rect = Rect((self.pos[0] + w_, self.pos[1]), (text.get_width(), 32))
            if rect.collidepoint(mpos):
                text = self.HEL16.render(item, 0, self.MENU_TXT_SEL)
                pygame.draw.rect(surf, self.MENU_BG_SEL, Rect((w_, 0), (text.get_width(), 32)))
                if self.CLICK:
                    picked = item, (w_, 32)

            surf.blit(text, (w_, 8))

            pygame.draw.line(surf, self.MENU_BG_ALT, (w_, 26), (w_ + text.get_width(), 26))
            w_ += text.get_width() + 4

            pygame.draw.line(surf, self.MENU_BG_ALT, (w_, 8), (w_, 26))

            w_ += 4

        self.destination.blit(surf, self.pos)

        return picked

    def draw_dropdown(self, items, pos=(0, 0)):
        if not items:
            return None, None
        h = len(items) * 32
        longest = max(
            list(items.keys()),
            key=lambda s: self.HEL16.render(s, 0, (0, 0, 0)).get_width()
        )
        _longest = self.HEL16.render(longest, 0, self.MENU_TXT)
        w = _longest.get_width() + 16

        surf = Surface((w, h))
        surf.fill(self.MENU_BG)

        mpos = pygame.mouse.get_pos()
        h_ = 0

        picked = None, None

        for item in items:
            text = self.HEL16.render(item, 0, self.MENU_TXT)
            rect = Rect((pos[0], pos[1] + h_), (w, 32))
            if rect.collidepoint(mpos):
                text = self.HEL16.render(item, 0, self.MENU_TXT_SEL)
                pygame.draw.rect(surf, self.MENU_BG_SEL, Rect((0, h_), (w, 32)))
                if self.CLICK:
                    picked = item, (pos[0] + w, pos[1] + h_)
            surf.blit(text, (8, h_ + 8))
            h_ += 32
            pygame.draw.line(surf, self.MENU_BG_ALT, (8, h_), (w-8, h_))

        self.destination.blit(surf, pos)

        return picked

    def update(self):
        mkey = self.draw_header()
        submenu, pos = mkey
        passed = False
        if submenu is not None:
            if isinstance(self.menu_items[submenu], dict):
                self.sub_menus = {}
                if self.menu_items[submenu]:
                    self.sub_menus[(submenu, pos)] = self.menu_items[submenu]
                    self.path.add(submenu)
            else:
                self.menu_items[submenu]()
            passed = True
                
        for key in list(self.sub_menus.keys()):
            if key not in self.sub_menus:
                continue
            _, pos = key
            choice, pos2 = self.draw_dropdown(self.sub_menus[key], pos)
            if choice is None:
                continue

            passed = True
            if isinstance(self.sub_menus[key][choice], dict):
                _sub_menus = {}
                for name, pos in self.sub_menus:
                    if name in self.path:
                        _sub_menus[(name, pos)] = self.sub_menus[(name, pos)]
                self.sub_menus = _sub_menus
                if self.sub_menus[key][choice]:
                    self.sub_menus[(choice, pos2)] = self.sub_menus[key][choice]
                    self.path.add(choice)
                else:
                    self.sub_menus = {}
                    self.path.clear()
            else:
                self.sub_menus[key][choice]()
                self.sub_menus = {}
                self.path.clear()

        if not passed and self.CLICK:
            self.sub_menus = {}
            self.path.clear()
