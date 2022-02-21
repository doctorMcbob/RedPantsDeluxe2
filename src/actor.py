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

HEL32 = pygame.font.SysFont("Helvetica", 32)

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
        self.name = tempalte["name"]
        # The rect that this class *is* should be considered as the ECB
        Rect.__init__(self, template["POS"], template["DIM"])
        self.x_vel = 0
        self.y_vel = 0
        
        self.hurtboxes = {}
        self.hitboxes = {}
        self.scripts = {}
        self.sprites = {}
        for key in template["sprites"]:
            self.sprites[key] = sprites.get_spite(template["sprites"][key])
        
        self.state = "START"
        self.frame = 0
        self.direction = 1

        self.tangible = False if "tangible" not in template else temlpate["tangible"]

    def _index(self, data):
        _frame = self.frame
        while _frame >= 0:
            name = "{}:{}".format(self.state, _frame)
            if name in data: return data[name]
            _frame -= 1
        return None

    def get_hitboxes(self):
        return _index(self, self.hitboxes)

    def get_hurtboxes(self):
        return _index(self, self.hurtboxes)

    def get_sprite(self):
        sprite = _index(self, self.sprites)
        if sprite is not None:
            if self.direction == -1:
                pygame.transform.flip(sprite, 1, 0)
            return sprite
        placeholder = Surface((self.w, self.h))
        placeholder.fill((1, 255, 1))
        placeholder.blit(
            HEL32.render("{}:{}".format(self.state, self.frame), 0, (0, 0, 0)),
            (0, 0)
        )
        return placeholder

    def update(self, world):
        script = _index(self, self.scripts)

        self.resolve(script, world)

        if tangible:
            self.collision_check(world)
            self.x += self.x_vel
            self.y += self.y_vel
        
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
        if "COLLIDE" in self.scrpts:
            actor.resolve(self.scripts["COLLIDE"], world)
    
    def resolve(self, script, world, logfunc=print):
        cmd_idx = 0
        while cmd_idx < len(script):
            if script[cmd_idx].startswith("#"): continue # comments
            cmd = script[cmd_idx].split()
            # STEP 1: EVALUATE
            for idx in range(cmd):
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
                    except TypeError:
                        continue

                except Exception as e:
                    logfunc("Error evaluating {}... {}".format(token, e))

            # STEP 2: OPERATORS / ALGEBRA
            evaluated = []
            for idx in range(cmd):
                token = cmd[idx]
                try:
                    if token in operators:
                        calculated = operators(evaluated.pop(), cmd.pop(idx+1))
                        evaluated.append(calculated)
                    else:
                        evaluated.append(token)
                except Exception as e:
                    logfunc("Error applying operators {}... {}".format(token, e))
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
                    continue
                if verb == "if":
                    nest = 1
                    while nest:
                        cmd_idx += 1
                        if cmd_idx > len(script):
                            raise Exception("End of script while parsing if")
                        if script[cmd_idx].split()[0] == "if":
                            nest += 1
                        if script[cmd_idx].split()[0] == "endif":
                            nest -= 1
                        
            except Exception as e:
                logfunc("Error resolving {}... {}".format(token, e))

            cmd_idx += 1
