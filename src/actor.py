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
from src import frames

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
    "and": lambda n1, n2: n1 and n2,
    "or": lambda n1, n2: n1 or n2,
    "nor": lambda n1, n2: n1 and not n2,

    "in": lambda n1, n2: n1 in n2,
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
        self.attributes = {}
        self.sprites = {}
        self.img = None # scripts can overwrite the sprite
        for key in template["sprites"]:
            self.sprites[key] = sprites.get_sprite(template["sprites"][key])
        self.spriteoffset = (0, 0) if "spriteoffset" not in template else template["spriteoffset"]
        
        self.state = "START"
        self.frame = 0
        self.direction = 1

        self.platform = False # scripts will flip this for platform draw
        
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

                    surf.blit(img, (x*32, y*32))
            surf.set_colorkey((1, 255, 1))
            return surf

        sprite = sprites.get_sprite(self.img) if self.img is not None else self._index(self.sprites)
        if sprite is not None:
            if self.direction == 1:
                return pygame.transform.flip(sprite, 1, 0)
            return sprite
            
        placeholder = Surface((self.w, self.h))
        placeholder.fill((1, 255, 1))
        placeholder.blit(
            HEL16.render("{}:{}".format(self.state, self.frame), 0, (0, 0, 0)),
            (0, 0)
        )
        return placeholder

    def update(self, world):
        self.img = None
        
        script = self._index(self.scripts)
        if script is not None:
            self.resolve(script, world)

        xflag, yflag = self.x_vel, self.y_vel
        if self.tangible:
            self.collision_check(world)
            self.x += int(self.x_vel)
            self.y += int(self.y_vel)

        if xflag != self.x_vel:
            if "XCOLLISION" in self.scripts:
                self.resolve(self.scripts["XCOLLISION"], world)
        if yflag and self.y_vel == 0:
            if "YCOLLISION" in self.scripts:
                self.resolve(self.scripts["YCOLLISION"], world)

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
                    if token == "None":
                        cmd[idx] = None

                    try:
                        cmd[idx] = int(token)
                        continue
                    except ValueError:
                        pass
                    try:
                        cmd[idx] = float(token)
                        continue
                    except ValueError:
                        pass

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
                            cmd[idx] = None

                    if token.startswith("inp"):
                        inp = token[3:]
                        if inp in inputs.STATE:
                            cmd[idx] = inputs.STATE[inp]

                except Exception as e:
                    logfunc("Error evaluating {} {}... {}".format(token, cmd_idx, e))

            # STEP 2: OPERATORS / ALGEBRA
            evaluated = []
            idx = 0
            while idx < len(cmd):
                token = cmd[idx]
                try:
                    if token == "abs":
                        calculated = abs(cmd.pop(idx+1))
                        evaluated.append(calculated)
                    elif token == "not":
                        calculated = not cmd.pop(idx+1)
                        evaluated.append(calculated)
                    elif type(token) == str and token in operators:
                        calculated = operators[token](evaluated.pop(), cmd.pop(idx+1))
                        evaluated.append(calculated)
                    else:
                        evaluated.append(token)
                except Exception as e:
                    logfunc("Error applying operators {} {}... {}".format(token, cmd_idx, e))
                idx += 1
            cmd = evaluated
            # step 3: COMMANDS
            try:
                verb = cmd.pop(0)
                if verb == "set":
                    actor, att, value = cmd
                    actor = self if actor == "self" else get_actor(actor)
                    if hasattr(actor, att):
                        if att in ["attributes", "scripts", "hitboxes", "hurtboxes"]:
                            raise Exception("Cannot write over {} {}".format(att, cmd_idx))
                        setattr(actor, att, value)                        
                    else:
                        self.attributes[att] = value
                if verb == "if":
                    conditional = cmd.pop(0)
                    if not conditional:
                        nest = 1
                        while nest > 0:
                            cmd_idx += 1
                            if cmd_idx > len(script):
                                raise Exception("End of script while parsing if")
                            if script[cmd_idx].split()[0] == "if":
                                nest += 1
                            if script[cmd_idx].split()[0] == "endif":
                                nest -= 1
                if verb == "exec":
                    key = cmd.pop(0)
                    if key not in self.scripts:
                        raise Exception("Cannot exec {}. Does not exist.".format(key))
                    self.resolve(self.scripts[key], world, logfunc=logfunc)
                if verb == "print":
                    print(cmd.pop(0))
                if verb == "img":
                    self.img = cmd.pop(0)
                if verb == "focus":
                    frame = frames.get_frame(cmd.pop(0))
                    actor = cmd.pop(0)
                    actor = self if actor == "self" else get_actor(actor)
                    frame.focus = actor
            except Exception as e:
                logfunc("Error resolving {} {}... {}".format(verb, cmd_idx, e))

            cmd_idx += 1
