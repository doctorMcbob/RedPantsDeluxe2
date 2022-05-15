"""
Okay. Here goes nothing.

Actors have
  ECB - Environmental Collision Box
  Hurtboxes
  Hitboxes
  scripts
  sprites
  state
  frame
  attributes

  x_velocity
  y_velocity


hurtboxes, hitboxes, scripts and sprites
  will all use the same key index system.

  get hitboxes for this frame? get scripts for this frame? ...
  {actor state}:{actor frame} -> scripts/hitboxes/...
  example
  RUNNING:3 -> frame 3 of running state
  
  important note, indexing in this way will ALWAYS FALLBACK TO MOST RECENT KEY
  so for example, if you put a key in at RUNNING:3 and then no more keys until RUNNING:10,
  frames 4 trough 9 will index at RUNNING:3

"""
import pygame
from pygame import Surface, Rect

from copy import deepcopy

from src import worlds
from src import inputs
from src import sprites
from src import frames
from src import scripts

TEMPLATES = {}
ACTORS = {}

def load():
    from src.lib import ACTORS as A

    for name in A.ACTORS.keys():
        TEMPLATES[name] = A.ACTORS[name]
    for name in A.ACTORS.keys():
        ACTORS[name] = Actor(A.ACTORS[name])

def add_actor_from_template(actor_name, template_name, updated_values={}):
    template = deepcopy(TEMPLATES[template_name])
    for key in updated_values.keys():
        if key not in template: continue
        template[key] = updated_values[key]
    ACTORS[actor_name] = Actor(template)

def get_actor(name):
    return ACTORS[name]

class Actor(Rect):
    def __init__(self, template):
        self.name = template["name"]
        # The rect that this class *is* should be considered as the ECB
        Rect.__init__(self, template["POS"], template["DIM"])
        self.x_vel = 0
        self.y_vel = 0
        
        self.hurtboxes = {}
        self.hitboxes = {}
        self.attributes = {}
        self.sprites = {}
        self.img = None # scripts can overwrite the sprite

        self.load_scripts(template["scripts"])
        self.load_sprites(template["sprites"])
        self.offsetkey = template["scripts"]
        self._input_name = None # change in script

        self.state = "START"
        self.frame = 0
        self.direction = 1
        self.rotation = 0
        
        self.platform = False # scripts will flip this for platform draw
        
        self.tangible = False if "tangible" not in template else template["tangible"]

    def load_sprites(self, name):
        self.sprites = sprites.get_sprite_map(name)

    def load_scripts(self, name):
        self.offsetkey = name
        self.scripts = scripts.get_script_map(name)

    def _index_key(self, data):
        bestkey = None
        bestframe = None
        for key in data.keys():
            if ":" not in key: continue
            state, frame = key.split(":")
            frame = int(frame)
            if state == self.state:
                if frame <= self.frame and (bestframe is None or bestframe < frame):
                    bestkey = key
                    bestframe = frame
        return bestkey

    def _index(self, data):
        bestkey = None
        bestframe = None
        for key in data.keys():
            if ":" not in key: continue
            state, frame = key.split(":")
            frame = int(frame)
            if state == self.state:
                if frame <= self.frame and (bestframe is None or bestframe < frame):
                    bestkey = key
                    bestframe = frame
        return None if bestkey is None else data[bestkey]

    def _rotate_box(self, rect, deg):
        x1, y1 = self.x, self.y
        w1, h1 = self.w, self.h
        x2, y2 = rect.x - x1, rect.y - y1
        w2, h2 = rect.w, rect.h

        if deg == 0:
            return Rect((x1+x2, y1+y2), (w2, h2))
        if deg ==  90:
            if self.direction == 1:
                return Rect((x1+y2, (y1+h1+h1)-(x2+w2)), (h2, w2))
            else:
                return Rect((x1+y2, (y1+h1)-(x2+w2)), (h2, w2))
        if deg == 180:
            return Rect(((x1+w1)-(x2+w2), (y1+h1)-(y2+h2)), (w2, h2))
        if deg == 270:
            if self.direction == 1:
                return Rect(((x1+w1)-(y2+h2), y1+x2-h1), (h2, w2))
            else:
                return Rect(((x1+w1)-(y2+h2), y1+x2), (h2, w2))

    def get_hitboxes(self):
        hitboxes = self._index(self.hitboxes)
        if hitboxes is None: return None
        boxes = []
        for pos, dim in hitboxes:
            x, y = pos
            w, h = dim
            if self.direction == 1:
                box = Rect(((self.x + self.w)-(x + w), y + self.y), dim)
            else:
                box = Rect((x + self.x, y + self.y), dim)
            box = self._rotate_box(box, self.rotation)
            boxes.append(box)
        return boxes

    def get_hurtboxes(self):
        hurtboxes = self._index(self.hurtboxes)
        if hurtboxes is None: return None
        boxes = []
        for pos, dim in hurtboxes:
            x, y = pos
            w, h = dim
            if self.direction == 1:
                box = Rect(((self.x + self.w)-(x + w), y + self.y), dim)
            else:
                box = Rect((x + self.x, y + self.y), dim)
            box = self._rotate_box(box, self.rotation)
            boxes.append(box)
        return boxes

    def get_sprite(self):
        if self.platform:
            # dynamically drawing platfrms
            surf = Surface((self.w, self.h))
            surf.fill((1, 255, 1))
            for y in range(self.h // 32):
                for x in range(self.w // 32):
                    if (x, y) == (0, 0):
                        img = self.sprites["{}00".format(self.state)]
                    elif (x, y) == ((self.w // 32) - 1, 0):
                        img = self.sprites["{}02".format(self.state)]
                    elif (x, y) == (0, (self.h // 32) - 1):
                        img = self.sprites["{}20".format(self.state)]
                    elif (x, y) == ((self.w // 32) - 1, (self.h // 32) - 1):
                        img = self.sprites["{}22".format(self.state)]
                    elif x == 0:
                        img = self.sprites["{}10".format(self.state)]
                    elif x == (self.w // 32) - 1:
                        img = self.sprites["{}12".format(self.state)]
                    elif y == 0:
                        img = self.sprites["{}01".format(self.state)]
                    elif y == (self.h // 32) - 1:
                        img = self.sprites["{}21".format(self.state)]
                    else:
                        img = self.sprites["{}11".format(self.state)]

                    surf.blit(sprites.get_sprite(img), (x*32, y*32))
            surf.set_colorkey((1, 255, 1))
            return surf

        sprite = sprites.get_sprite(self.img) if self.img is not None else sprites.get_sprite(self._index(self.sprites))
        if sprite is not None:
            if self.direction == 1:
                sprite = pygame.transform.flip(sprite, 1, 0)
            if self.rotation != 0:
                sprite = pygame.transform.rotate(sprite, self.rotation)
            return sprite

        placeholder = Surface((self.w, self.h))
        placeholder.fill((1, 255, 1))
        
        return placeholder

    def get_offset(self):
        name = self.img if self.img is not None else self._index(self.sprites)
        return sprites.get_offset(self.offsetkey, name)

    def update(self, world):
        self.img = None

        script = self._index(self.scripts)
        if script is not None:
            scripts.resolve(self.name, script, world)

        xflag, yflag = self.x_vel, self.y_vel
        if self.tangible:
            self.collision_check(world)
            self.x += int(self.x_vel)
            self.y += int(self.y_vel)

        if xflag != self.x_vel and int(self.x_vel) == 0:
            if abs(self.x_vel) < 1:
                self.x_vel = 0
            if "XCOLLISION" in self.scripts:
                scripts.resolve(self.name, self.scripts["XCOLLISION"], world)
        if yflag != self.y_vel and int(self.y_vel) == 0:
            if abs(self.y_vel) < 1:
                self.y_vel = 0
            if "YCOLLISION" in self.scripts:
                scripts.resolve(self.name, self.scripts["YCOLLISION"], world)

        self.hit_check(world)
        self.frame += 1

    def debug(self, dest, pos, font, scrollx, scrolly, text=False):
        pygame.draw.rect(dest, (0, 0, 255), Rect(self.x-scrollx, self.y-scrolly, self.w, self.h), width=2)

        hitboxes = self.get_hitboxes()
        if hitboxes is not None:
            for box in hitboxes:
                pygame.draw.rect(dest, (255, 0, 0), Rect(box.x-scrollx, box.y-scrolly, box.w, box.h), width=2)
        hurtboxes = self.get_hurtboxes()
        if hurtboxes is not None:
            for box in hurtboxes:
                pygame.draw.rect(dest, (0, 255, 0), Rect(box.x-scrollx, box.y-scrolly, box.w, box.h), width=2)

        if text:
            surf = Surface((512, 256))
            pygame.draw.rect(surf, (255, 255, 255), Rect((8, 8), (512 - 16, 256 - 16)))
            
            X, Y = 16, 16
            surf.blit(font.render("NAME {}".format(self.name), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("STATE {}".format(self.state), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("FRAME {}".format(self.frame), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("X, Y {},{}".format(self.x, self.y), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("vel {},{}".format(self.x_vel, self.y_vel), 0, (0, 0, 0)), (X, Y))


            script = self._index(self.scripts)
            if script is None: return
            Y = 16
            X += 256
            surf.blit(font.render("TANGIBLE {}".format(self.tangible), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("SCRIPT KEY {}".format(self._index_key(self.scripts)), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("SPRITE KEY {}".format(self._index_key(self.sprites)), 0, (0, 0, 0)), (X, Y))
            Y += 24
            surf.blit(font.render("SPRIET NAME {}".format(self._index(self.sprites)), 0, (0, 0, 0)), (X, Y))
            Y += 24
            dest.blit(surf, (16, 16))


    def collision_check(self, world):
        actors = list(filter(lambda actor:not (actor is self), world.get_actors()))
        tangibles = list(filter(lambda actor: actor.tangible, actors))
        hits = self.collidelistall(actors)
        for hit in hits:
            self.collision_with(actors[hit], world)
            actors[hit].collision_with(self, world)

        # X axis
        if self.x_vel:
            direction = 1 if self.x_vel < 0 else -1
            for n in range(int(self.x_vel) // self.w):
                if self.move(self.w*n, 0).collidelist(tangibles) != -1:
                    self.x_vel = (self.w*n) + (self.x_vel % self.w)
                    break

            hits = self.move(int(self.x_vel), 0).collidelistall(tangibles)
            for hit in hits:
                self.collision_with(tangibles[hit], world)
                tangibles[hit].collision_with(self, world)

            while self.move(int(self.x_vel), 0).collidelist(tangibles) != -1:
                self.x_vel += direction

        # Y axis
        if self.y_vel:
            direction = 1 if self.y_vel < 0 else -1
            for n in range(int(self.y_vel) // self.h):
                if self.move(0, self.h*n).collidelist(tangibles) != -1:
                    self.y_vel = (self.h*n) + (self.y_vel % self.h)
                    break

            hits = self.move(0, int(self.y_vel)).collidelistall(tangibles)
            for hit in hits:
                self.collision_with(tangibles[hit], world)
                tangibles[hit].collision_with(self, world)

            while self.move(0, int(self.y_vel)).collidelist(tangibles) != -1:
                self.y_vel += direction

        # cross check
        if self.x_vel and self.y_vel:
            while self.move(self.x_vel, self.y_vel).collidelist(tangibles) != -1:
                self.x_vel += 1 if self.x_vel < 0 else -1
                self.y_vel += 1 if self.y_vel < 0 else -1
            
    def collision_with(self, actor, world):
        if "COLLIDE" in self.scripts:
            scripts.resolve(actor.name, self.scripts["COLLIDE"], world, related=self.name)

    def hit_check(self, world):
        hurtboxes = self.get_hurtboxes()
        if hurtboxes is None: return
        actors = list(filter(lambda actor:not (actor is self), world.get_actors()))
        for actor in actors:
            hitboxes = actor.get_hitboxes()
            if hitboxes is None: continue

            for hurtbox in hurtboxes:
                if Rect(hurtbox).collidelist(hitboxes) != -1:
                    actor.hit(self, world)
                    break
                    
    def hit(self, actor, world):
        if "HIT" in self.scripts:
            scripts.resolve(actor.name, self.scripts["HIT"], world, related=self.name)

