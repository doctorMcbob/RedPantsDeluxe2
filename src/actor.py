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

import operator as ops

from src import inputs
from src import sprites

HEL16 = pygame.font.SysFont("Helvetica", 16)

ACTORS = {}

operators = {
    "+": ops.add,
    "-": ops.sub,
    "*": ops.mul,
    "//": ops.truediv,
    "/": ops.floordiv,
    "%": ops.mod,
    "**": ops.pos,
    "==": lambda n1, n2: n1==n2,
    ">": lambda n1, n2: n1>n2,
    "<": lambda n1, n2: n1<n2,
    ">=": lambda n1, n2: n1>=n2,
    "<=": lambda n1, n2: n1<=n2,
    "!=": lambda n1, n2: n1!=n2,
}

def load():
    from src.lib import ACTORS as A

    for name in A.ACTORS.keys():
        ACTORS[name] = Actor(A.ACTORS[name])

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
        self.scripts = template["scripts"]
        self.sprites = {}
        for key in template["sprites"]:
            self.sprites[key] = sprites.get_sprite(template["sprites"][key])
        self.spriteoffset = (0, 0) if "spriteoffset" not in template else template["spriteoffset"]
            
        self.state = "START"
        self.frame = 0
        self.direction = 1

        self.tangible = False if "tangible" not in template else template["tangible"]

    def _index(self, data):
        _frame = self.frame
        while _frame >= 0:
            name = "{}:{}".format(self.state, _frame)
            if name in data: return data[name]
            _frame -= 1
        return None

    def get_hitboxes(self):
        return self._index(self.hitboxes)

    def get_hurtboxes(self):
        return self._index(self.hurtboxes)

    def get_sprite(self):
        sprite = self._index(self.sprites)
        if sprite is not None:
            if self.direction == -1:
                pygame.transform.flip(sprite, 1, 0)
            return sprite
        placeholder = Surface((self.w, self.h))
        placeholder.fill((1, 255, 1))
        placeholder.blit(
            HEL16.render("{}:{}".format(self.state, self.frame), 0, (0, 0, 0)),
            (0, 0)
        )
        return placeholder

    def update(self, world):
        script = self._index(self.scripts)
        if script is not None:
            self.resolve(script, world)

        if self.tangible:
            self.collision_check(world)
            self.x += self.x_vel
            self.y += self.y_vel

    def debug(self, G):
        Y = 0
        X = 0
        G["SCREEN"].blit(G["HEL16"].render("STATE {}".format(self.state), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        Y += 16
        G["SCREEN"].blit(G["HEL16"].render("FRAME {}".format(self.frame), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))

        script = self._index(self.scripts)
        if script is None: return

        Y += 16
        G["SCREEN"].blit(G["HEL16"].render("CURRENT SCRIPT", 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        X += 16
        for cmd in script:
            Y += 16
            G["SCREEN"].blit(G["HEL16"].render("{}".format(cmd), 0, (0, 0, 0)), (self.x+self.w+X, self.y+Y))
        
        
    def collision_check(self, world):
        actors = world.get_actors()
        tangibles = list(filter(lambda actor: actor.tangible, actors))
        hits = self.collidelistall(actors)
        for hit in hits:
            self.collision_with(actors[hit], world)
            actors[hit].collision_with(self, world)

        # X axis
        if self.x_vel:
            direction = 1 if self.x_vel < 0 else -1
            for n in range(self.x_vel // self.w):
                if self.move(w*n, 0).collidelist(tangibles) != -1:
                    self.x_vel = (w*n) + (self.x_vel % w)
                    break

            hits = self.move(self.x_vel, 0).collidelistall(tangibles)
            for hit in hits:
                self.collision_with(tangibles[hit], world)
                tangibles[hit].collision_with(self, world)

            while self.move(self.x_vel, 0).collidlelist(tangibles) != -1:
                self.x_vel += direction
                
        # Y axis
        if self.y_vel:
            direction = 1 if self.y_vel < 0 else -1
            for n in range(self.y_vel // self.h):
                if self.move(0, h*n).collidelist(tangibles) != -1:
                    self.y_vel = (h*n) + (self.y_vel % h)
                    break

            hits = self.move(0, self.y_vel).collidelistall(tangibles)
            for hit in hits:
                self.collision_with(tangibles[hit], world)
                tangibles[hit].collision_with(self, world)

            while self.move(0, self.y_vel).collidlelist(tangibles) != -1:
                self.y_vel += direction

        # cross check
        if self.x_vel and self.y_vel:
            while self.move(self.x_vel, self.y_vel).collidelist(tangibles) != -1:
                self.x_vel += 1 if self.x_vel < 0 else -1
                self.y_vel += 1 if self.y_vel < 0 else -1
            
        
    def collision_with(self, actor, world):
        if "COLLIDE" in self.scripts:
            actor.resolve(self.scripts["COLLIDE"], world)
    
    def resolve(self, script, world, logfunc=print):
        cmd_idx = 0
        while cmd_idx < len(script):
            if not script[cmd_idx] or script[cmd_idx].startswith("#"): # comments
                cmd_idx += 1
                continue
            cmd = script[cmd_idx].split()
            # STEP 1: EVALUATE
            for idx in range(len(cmd)):
                token = cmd[idx]
                try:
                    if "." in token:
                        actor, a = token.split(".")
                        if actor == "self": actor = self
                        else: actor = get_actor(actor)
                        if hasattr(actor, a):
                            if a in ["attributes", "scripts", "hitboxes", "hurtboxes"]:
                                raise Exception("Cannot refrence {}".format(a))
                            cmd[idx] = getattr(actor, a)
                        elif a in actor.attributes:
                            cmd[idx] = actor.attributes[a]
                        else:
                            raise Exception("Actor {} has no refrence {}".format(actor, a))
                    try:
                        if int(cmd[idx]) != float(cmd[idx]):
                            cmd[idx] = float(cmd[idx])
                        else:
                            cmd[idx] = int(cmd[idx])
                    except ValueError:
                        continue

                except Exception as e:
                    logfunc("Error evaluating {}... {}".format(token, e))

            # STEP 2: OPERATORS / ALGEBRA
            evaluated = []
            idx = 0
            while idx < len(cmd):
                token = cmd[idx]
                try:
                    if token in operators:
                        calculated = operators[token](evaluated.pop(), cmd.pop(idx+1))
                        evaluated.append(calculated)
                    else:
                        evaluated.append(token)
                except Exception as e:
                    logfunc("Error applying operators {}... {}".format(token, e))
                idx += 1
            cmd = evaluated
            # step 3: COMMANDS
            try:
                verb = cmd.pop(0)
                if verb == "set":
                    actor, att, value = cmd
                    if actor == "self": actor = self
                    else: actor = get_actor(actor)
                    if hasattr(actor, att):
                        if att in ["attributes", "scripts", "hitboxes", "hurtboxes"]:
                            raise Exception("Cannot write over {}".format(att))
                        setattr(actor, att, value)                        
                    else:
                        self.attributes[att] = value
                if verb == "if":
                    nest = 1
                    while nest > 0:
                        cmd_idx += 1
                        if cmd_idx > len(script):
                            raise Exception("End of script while parsing if")
                        if script[cmd_idx].split()[0] == "if":
                            nest += 1
                        if script[cmd_idx].split()[0] == "endif":
                            nest -= 1
                        
            except Exception as e:
                logfunc("Error resolving {}... {}".format(cmd, e))

            cmd_idx += 1
