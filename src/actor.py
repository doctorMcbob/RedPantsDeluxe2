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
        self.scripts = scripts.get_script_map(name)
        
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

    def get_hitboxes(self):
        return self._index(self.hitboxes)

    def get_hurtboxes(self):
        return self._index(self.hurtboxes)

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
        key = self.img if self.img is not None else self._index(self.sprites)
        return sprites.get_offset(key)

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

        if xflag != self.x_vel:
            if "XCOLLISION" in self.scripts:
                scripts.resolve(self.name, self.scripts["XCOLLISION"], world)
        if yflag and self.y_vel == 0:
            if "YCOLLISION" in self.scripts:
                scripts.resolve(self.name, self.scripts["YCOLLISION"], world)

        self.frame += 1

    def debug(self, G):
        Y = 0
        X = 0
        G["SCREEN"].blit(G["HEL16"].render("STATE {}".format(self.state), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        Y += 16
        G["SCREEN"].blit(G["HEL16"].render("FRAME {}".format(self.frame), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        Y += 16
        G["SCREEN"].blit(G["HEL16"].render("X, Y {},{}".format(self.x, self.y), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        Y += 16
        G["SCREEN"].blit(G["HEL16"].render("vel {},{}".format(self.x_vel, self.y_vel), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))

        script = self._index(self.scripts)
        if script is None: return
        Y = 0
        X += 128
        G["SCREEN"].blit(G["HEL16"].render("CURRENT SCRIPT", 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        X += 16
        for cmd in script:
            Y += 16
            G["SCREEN"].blit(G["HEL16"].render("{}".format(cmd), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))

        print("state {}".format(self.state))
        print("x, y: {},{}".format(self.x, self.y))
        print("veloc {},{}".format(self.x_vel, self.y_vel))
        print(self.attributes)
        

            
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
                    self.x_vel = (self.w*n) + (self.x_vel % w)
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
            scripts.resolve(actor.name, self.scripts["COLLIDE"], world)

