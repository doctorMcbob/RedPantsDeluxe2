from src import inputs
from src import frames
from src import actor as a
from src import worlds
from src import boxes

import operator as ops

from copy import deepcopy

from random import randint

import string

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
    "at": lambda n1, n2: n1[n2]
}

SCRIPTS = {}

def swap_in(scripts):
    global SCRIPTS
    SCRIPTS = deepcopy(scripts)

def load():
    from src.lib import SCRIPTS as S

    for name in S.SCRIPTS.keys():
        SCRIPTS[name] = S.SCRIPTS[name]

def get_script_map(name):
    return SCRIPTS[name] if name in SCRIPTS else None

def resolve(reference, script, world, related=None, logfunc=print):
    cmd_idx = 0
    while cmd_idx < len(script):
        if not script[cmd_idx].split() or script[cmd_idx].startswith("#"): # comments
            cmd_idx += 1
            continue

        cmd = parse_tokens(script[cmd_idx], logfunc=logfunc)
        cmd = evaluate_literals(cmd, reference, world, related=related, logfunc=logfunc)
        cmd = resolve_operators(cmd, logfunc=logfunc)
        # resolve command!
        try:
            verb = cmd.pop(0)

            if verb == "goodbye":
                for w in worlds.get_worlds():
                    if reference in w.actors:
                        w.actors.remove(reference)

            if verb == "set":
                actor, att, value = cmd
                if actor == "related" and related is not None:
                    actor = related
                actor = a.get_actor(reference) if actor == "self" else a.get_actor(actor)
                if hasattr(actor, att):
                    if att in ["attributes", "scripts", "hitboxes", "hurtboxes"]:
                        raise Exception("Cannot write over {} {}".format(att, cmd_idx))
                    setattr(actor, att, value)                        
                else:
                    actor.attributes[att] = value

            if verb == "if":
                conditional = cmd.pop(0)
                if not conditional:
                    nest = 1
                    while nest > 0:
                        cmd_idx += 1
                        if cmd_idx >= len(script):
                            raise Exception("End of script while parsing if")
                        if script[cmd_idx].split():
                            if script[cmd_idx].split()[0] == "if":
                                nest += 1
                            if script[cmd_idx].split()[0] == "endif":
                                nest -= 1

            if verb == "exec":
                key = cmd.pop(0)
                if key not in a.get_actor(reference).scripts:
                    raise Exception("Cannot exec {}. Does not exist.".format(key))
                resolve(reference, a.get_actor(reference).scripts[key], world, logfunc=logfunc)

            if verb == "print":
                print(cmd.pop(0))

            if verb == "back":
                if reference in world.actors:
                    me = world.actors.index(reference)
                    i = 0
                    while "back" in world.actors[i]:
                        i += 1
                    if i < me:
                        world.actors.insert(i, world.actors.pop(me))
                        
            if verb == "img":
                a.get_actor(reference).img = cmd.pop(0)

            if verb == "focus":
                frame = frames.get_frame(cmd.pop(0))
                actor = cmd.pop(0)
                actor = a.get_actor(reference) if actor == "self" else a.get_actor(actor)
                frame.focus = actor

            if verb == "view":
                frame = frames.get_frame(cmd.pop(0))
                newworld = worlds.get_world(cmd.pop(0))
                frame.world = newworld

            if verb == "move":
                name = cmd.pop(0)
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                if name not in world.actors:
                    raise Exception("{} not in world".format(name))
                world.actors.remove(name)
                newworld = worlds.get_world(cmd.pop())
                newworld.actors.append(name)

            if verb == "place":
                name = cmd.pop(0)
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                world_ref = cmd.pop(0)
                world = worlds.get_world(world_ref)
                if world is None:
                    raise Exception("Invalid Place destination {}".format(world_ref))
                if name not in world.actors:
                    world.actors.append(name)
                
            if verb == "take":
                world_ref = cmd.pop(0)
                world = worlds.get_world(world_ref)
                name = cmd.pop(0)
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                if world is None:
                    raise Exception("Invalid Place destination {}".format(world_ref))
                if name in world.actors:
                    world.actors.remove(name)

            if verb == "rebrand":
                keyname = cmd.pop(0)
                actor = a.get_actor(reference)
                actor.load_scripts(keyname)
                actor.load_sprites(keyname)
                if "START:0" in actor.scripts:
                    resolve(reference, actor.scripts["START:0"], world, logfunc=logfunc)

            if verb == "remove":
                l = cmd.pop(0)
                if type(l) is not list:
                    raise Exception("Could not remove from {}, not type list".format(l))
                l.remove(cmd.pop(0))

            if verb == "add":
                l = cmd.pop(0)
                if type(l) is not list:
                    raise Exception("Could not remove from {}, not type list".format(l))
                l.append(cmd.pop(0))

            if verb == "hitboxes":
                keyname = cmd.pop(0)
                actor = a.get_actor(reference)
                actor.hitboxes = boxes.get_hitbox_map(keyname)

            if verb == "hurtboxes":
                keyname = cmd.pop(0)
                actor = a.get_actor(reference)
                actor.hurtboxes = boxes.get_hurtbox_map(keyname)

            if verb == "create":
                template_name, actor_name, x, y = cmd
                a.add_actor_from_template(actor_name, template_name, {
                    "name": actor_name, "POS": (int(x), int(y))
                })
                actor = a.get_actor(actor_name)
                if actor_name not in world.actors:
                    world.actors.append(actor_name)
                if "START:0" in actor.scripts:
                    resolve(actor_name, actor.scripts["START:0"], world, logfunc=logfunc)

            if verb == "update":
                world_ref = cmd.pop(0)
                worlds.get_world(world_ref).flagged_for_update = True

            if verb == "for": # gulp
                key = cmd.pop(0)
                target = deepcopy(cmd.pop())
                if not hasattr(target, '__iter__'):
                    raise Exception('Target of for loop is not iterable')

                miniscript = []
                nest = 1
                while nest > 0:
                    cmd_idx += 1
                    if cmd_idx >= len(script):
                        raise Exception("End of script while parsing for")
                    miniscript .append(script[cmd_idx])

                    line = script[cmd_idx].split()

                    if line:
                        if line[0] == "for":
                            nest += 1
                        if line[0] == "endfor":
                            nest -= 1
                            
                # pop off the final remaining endfor
                miniscript.pop()

                miniscript = miniscript
                
                for value in target:
                    to_run = [str(value).join(s.split(key)) for s in miniscript]
                    resolve(reference, to_run, world, related=related, logfunc=logfunc)
                    
        except Exception as e:
            logfunc("{} Error on line {}".format(reference, cmd_idx))
            for i, cmd in enumerate(script):
                logfunc(("> " if i == cmd_idx else "") + "{} ".format(i) + cmd)
            logfunc("Error resolving {}... {}".format(verb, e))

        cmd_idx += 1

def parse_tokens(cmd, logfunc=print):
    parsed = []
    token = ""
    idx = 0
    try:
        while idx < len(cmd):
            if cmd[idx] in string.whitespace:
                if token:
                    parsed.append(token)
                token = ""

            elif cmd[idx] in ["'", '"']:
                if token != "":
                    raise Exception('Bunked String Syntax')
                quote = cmd[idx]
                idx += 1
                while cmd[idx] != quote:
                    token += cmd[idx]
                    idx += 1

            else:
                token += cmd[idx]
            idx += 1
        if token:
            parsed.append(token)
        return parsed
    except Exception as e:
        logfunc(cmd)
        logfunc('Parse Error {}'.format(e))
        return []
        
def evaluate_literals(cmd, reference, world, related=None, logfunc=print):
    for idx in range(len(cmd)):
        token = cmd[idx]
        try:
            if token == "RAND?":
                cmd[idx] = randint(0, 1)
            if token == "COLLIDE?":
                actor = a.get_actor(reference)
                actors = list(filter(lambda actr:not (actr is actor), world.get_actors()[::-1]))
                tangibles = list(filter(lambda actr: actr.tangible, actors))
                hits = actor.collidelistall(tangibles)
                cmd[idx] = [tangibles[hit].name for hit in hits]
            if token == "None":
                cmd[idx] = None
            if token == "[]":
                cmd[idx] = []
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
                actor, att = token.split(".")
                if actor == "related" and related is not None:
                    actor = related
                actor = a.get_actor(reference) if actor == "self" else a.get_actor(actor)
                if hasattr(actor, att):
                    if att in ["attributes", "scripts", "hitboxes", "hurtboxes"]:
                        raise Exception("Cannot refrence {}".format(att))
                    cmd[idx] = getattr(actor, att)
                elif att in actor.attributes:
                    cmd[idx] = actor.attributes[att]
                else:
                    cmd[idx] = None
            if token.startswith("inp"):
                if a.get_actor(reference)._input_name is None:
                    cmd[idx] = 0 if token != "inpEVENTS" else []
                    continue
                inp = token[3:]
                state = inputs.get_state(a.get_actor(reference)._input_name)
                if state is None: continue
                if inp in state:
                    cmd[idx] = state[inp]
        except Exception as e:
            logfunc(cmd)
            logfunc("Error evaluating {}... {}".format(token, e))
    return cmd

def resolve_operators(cmd, logfunc=print):
    evaluated = []
    idx = 0
    line = deepcopy(cmd)
    while idx < len(cmd):
        token = cmd[idx]
        try:
            if token == "isframe":
                frame = frames.get_frame(cmd.pop(idx+1))
                evaluated.append(frame is not None)
            elif token == "len":
                calculated = len(cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "countof":
                calculated = cmd.pop(idx+1).count(cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "min":
                calculated = min(cmd.pop(idx+1), cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "max":
                calculated = max(cmd.pop(idx+1), cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "abs":
                calculated = abs(cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "not":
                calculated = not cmd.pop(idx+1)
                evaluated.append(calculated)
            elif token == "exists":
                check = cmd.pop(idx+1)
                calculated = any(check in w.actors for w in worlds.get_worlds())
                evaluated.append(calculated)
            elif token == "range":
                evaluated.append(list(range(int(cmd.pop(idx+1)))))
            elif type(token) == str and token in operators:
                left, right = evaluated.pop(), cmd.pop(idx+1)
                if token != "at":
                    if (type(left) == str and type(right) == int) or (type(right) == str and type(left) == int):
                        left, right = str(left), str(right)
                calculated = operators[token](left, right)
                evaluated.append(calculated)
            else:
                evaluated.append(token)
        except Exception as e:
            logfunc(line)
            logfunc("Error applying operators {}... {}".format(token, e))
        idx += 1
    return evaluated

