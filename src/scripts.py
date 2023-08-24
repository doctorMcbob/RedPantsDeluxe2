from src import inputs
from src import frames
from src import actor as a
from src import worlds
from src import boxes
from src import sounds
from src import game

import operator as ops

from copy import deepcopy

from random import randint, choice

import string
import math
import traceback
import sys

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
    "at": lambda n1, n2: n1[n2],

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
    return deepcopy(SCRIPTS[name]) if name in SCRIPTS else None

def resolve(reference, script, world, related=None, logfunc=print):
    cmd_idx = 0

    while cmd_idx < len(script):
        cmd = script[cmd_idx]
        if not cmd:
            cmd_idx += 1
            continue
        cmd = evaluate_literals(cmd, reference, world, related=related, logfunc=logfunc)
        cmd = resolve_operators(cmd, world, logfunc=logfunc, actor=reference)

        # resolve command!
        try:
            verb = cmd.pop(0)

            if verb == "quit":
                sys.exit()

            elif verb == "goodbye":
                for w in worlds.get_worlds():
                    if reference in w.actors:
                        w.actors.remove(reference)
                a.delete_actor(reference)
                return 'goodbye'

            elif verb == "break":
                return

            elif verb == "reset":
                game.load()

            elif verb == "update_sticks":
                inputs.update_sticks()

            elif verb == "set":
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

            elif verb == "reassign":
                actor, oldkey, newkey = cmd
                if actor == "related" and related is not None:
                    actor = related
                actor = a.get_actor(reference) if actor == "self" else a.get_actor(actor)
                actor.scripts[newkey] = actor.scripts.pop(oldkey)
                    
            elif verb == "if":
                conditional = cmd.pop(0)
                if not conditional:
                    nest = 1
                    while nest > 0:
                        cmd_idx += 1
                        if cmd_idx >= len(script):
                            raise Exception("End of script while parsing if")
                        if script[cmd_idx]:
                            if script[cmd_idx][0] == "if":
                                nest += 1
                            if script[cmd_idx][0] == "endif":
                                nest -= 1

            elif verb == "exec":
                key = cmd.pop(0)
                if key not in a.get_actor(reference).scripts:
                    raise Exception("Cannot exec {}. Does not exist.".format(key))
                if resolve(reference, a.get_actor(reference).scripts[key], world, logfunc=logfunc) == 'goodbye':
                    return 'goodbye'

            elif verb == "print":
                print(cmd.pop(0))

            elif verb == "back":
                if reference in world.actors:
                    me = world.actors.index(reference)
                    i = 0
                    while "back" in world.actors[i]:
                        i += 1
                    if i < me:
                        world.actors.insert(i, world.actors.pop(me))

            elif verb == "front":
                if reference in world.actors and world.actors[-1] != reference:
                    me = world.actors.index(reference)
                    world.actors.append(world.actors.pop(me))
                
            elif verb == "img":
                a.get_actor(reference).img = cmd.pop(0)

            elif verb == "activate":
                frame = frames.get_frame(cmd.pop(0))
                if frame is not None:
                    frame.active = True

            elif verb == "deactivate":
                frame = frames.get_frame(cmd.pop(0))
                if frame is not None:
                    frame.active = False

            elif verb == "killframe":
                name = cmd.pop(0)
                frames.delete_frame(name)

            elif verb == "makeframe":
                name, world, x, y, w, h = cmd
                frames.add_frame(name, world, (w, h), (x, y))

            elif verb == "focus":
                frame = frames.get_frame(cmd.pop(0))
                actor = cmd.pop(0)
                actor = a.get_actor(reference) if actor == "self" else a.get_actor(actor)
                frame.focus = actor

            elif verb == "scrollbound":
                frame = frames.get_frame(cmd.pop(0))
                direction = cmd.pop(0)
                value = cmd.pop(0)
                if value is not None:
                    value = int(value)
                frame.scrollbound[direction] = value
                
            elif verb == "view":
                frame = frames.get_frame(cmd.pop(0))
                newworld = worlds.get_world(cmd.pop(0))
                frame.world = newworld

            elif verb == "move":
                name = cmd.pop(0)
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                if name not in world.actors:
                    raise Exception("{} not in world".format(name))
                world.actors.remove(name)
                newworld = worlds.get_world(cmd.pop())
                newworld.actors.append(name)

            elif verb == "place":
                name = cmd.pop(0)
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                world_ref = cmd.pop(0)
                world = worlds.get_world(world_ref)
                if world is None:
                    raise Exception("Invalid Place destination {}".format(world_ref))
                if name not in world.actors:
                    world.actors.append(name)
                
            elif verb == "take":
                world_ref = cmd.pop(0)
                world = worlds.get_world(world_ref)
                name = cmd.pop(0)
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                if name in world.actors:
                    world.actors.remove(name)

            elif verb == "takeall":
                name = cmd.pop()
                if name == "self": name = a.get_actor(reference).name
                if name == "related": name = a.get_actor(related).name
                for w in worlds.get_worlds():
                    if reference in w.actors:
                        w.actors.remove(name)


            elif verb == "rebrand":
                keyname = cmd.pop(0)
                actor = a.get_actor(reference)
                actor.load_scripts(keyname)
                actor.load_sprites(keyname)
                if "START:0" in actor.scripts:
                    if resolve(reference, actor.scripts["START:0"], world, logfunc=logfunc) == 'goodbye':
                        return 'goodbye'

            elif verb == "remove":
                l = cmd.pop(0)
                if type(l) is not list:
                    raise Exception("Could not remove from {}, not type list".format(l))
                l.remove(cmd.pop(0))

            elif verb == "add":
                l = cmd.pop(0)
                if type(l) is not list:
                    raise Exception("Could not remove from {}, not type list".format(l))
                l.append(cmd.pop(0))

            elif verb == "hitboxes":
                keyname = cmd.pop(0)
                actor = a.get_actor(reference)
                actor.hitboxes = boxes.get_hitbox_map(keyname)

            elif verb == "hurtboxes":
                keyname = cmd.pop(0)
                actor = a.get_actor(reference)
                actor.hurtboxes = boxes.get_hurtbox_map(keyname)

            elif verb == "create":
                template_name, actor_name, x, y = cmd
                a.add_actor_from_template(actor_name, template_name, {
                    "name": actor_name, "POS": (int(x), int(y))
                })
                actor = a.get_actor(actor_name)
                if actor_name not in world.actors:
                    world.actors.append(actor_name)
                if "START:0" in actor.scripts:
                    if resolve(actor_name, actor.scripts["START:0"], world, logfunc=logfunc) == 'goodbye':
                        return 'goodbye'

            elif verb == "update":
                world_ref = cmd.pop(0)
                worlds.get_world(world_ref).flagged_for_update = True

            elif verb == "sfx":
                sounds.play_sound(cmd.pop(0))

            elif verb == "song":
                sounds.play_song(cmd.pop(0))

            elif verb == "sfxoff":
                sounds.stop_sounds()

            elif verb == "songoff":
                sounds.stop_song()

            elif verb == "offsetbgscrollx":
                world_ref = cmd.pop(0)
                worlds.get_world(world_ref).background_xscroll += float(cmd.pop(0))
                
            elif verb == "offsetbgscrolly":
                world_ref = cmd.pop(0)
                worlds.get_world(world_ref).background_yscroll += float(cmd.pop(0))
                
            elif verb == "add_input_state":
                input_state_name = cmd.pop(0)
                inputs.add_state(input_state_name)

            elif verb == "setjoy":
                input_state_name, joy_count = cmd
                inputstate = inputs.get_state(input_state_name)
                if inputstate is None:
                    raise Exception("Invalid input state {}".format(input_state_name))
                inputs.add_joystick_to_input_state(input_state_name, joy_count)

            elif verb == "for": # gulp
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
                    miniscript.append(script[cmd_idx])

                    line = script[cmd_idx]

                    if line:
                        if line[0] == "for":
                            nest += 1
                        if line[0] == "endfor":
                            nest -= 1
                            
                # pop off the final remaining endfor
                miniscript.pop()

                for value in target:
                    to_run = []
                    for cmd in miniscript:
                        _cmd = []
                        for token in cmd:
                            if key in token:
                                token = str(value).join(token.split(key))
                            _cmd.append(token)
                        to_run.append(_cmd)

                    if resolve(reference, to_run, world, related=related, logfunc=logfunc) == 'goodbye':
                        return 'goodbye'

                    
        except Exception as e:
            logfunc("{} Error on line {}".format(reference, cmd_idx))
            logfunc(cmd)
            for i, cmd in enumerate(script):
                logfunc(("> " if i == cmd_idx else "") + "{} ".format(i),  cmd)
            print(traceback.format_exc())
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

                if token == "":
                    parsed.append(token)


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
    cmd = cmd[:]

    for idx in range(len(cmd)):
        token = cmd[idx]
        try:
            if token == "WORLD?":
                cmd[idx] = world.name
            if token == "RAND?":
                cmd[idx] = randint(0, 1)
            if token == "STICKS?":
                cmd[idx] = inputs.get_num_sticks()
            if token == "song?":
                cmd[idx] = sounds.get_song()
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
                _path = token.split(".")
                path = deepcopy(_path)
                ref = path.pop(0)
                while path:
                    if ref == "related" and related is not None:
                        ref = related
                    ref = a.get_actor(reference) if ref == "self" else a.get_actor(ref)
                    if hasattr(ref, path[0]):
                        ref = getattr(ref, path.pop(0))
                    elif path[0] in ref.attributes:
                        ref = ref.attributes[path.pop(0)]
                    else:
                        ref = None
                        break
                cmd[idx] = ref
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

def resolve_operators(cmd, world, logfunc=print, actor=None):
    evaluated = []
    idx = 0
    line = deepcopy(cmd)
    while idx < len(cmd):
        token = cmd[idx]
        try:
            if token == "isframe":
                frame = frames.get_frame(cmd.pop(idx+1))
                evaluated.append(frame is not None)
            elif token == "hasframe":
                world = worlds.get_world(cmd.pop(idx+1))
                hasframe = False
                for frame in frames.get_frames():
                    if not frame.active: continue
                    if frame.world == world:
                        hasframe = True
                        break
                evaluated.append(hasframe)
                    
            elif token == "isinputstate":
                inputstate = inputs.get_state(cmd.pop(idx+1))
                evaluated.append(inputstate is not None)
            elif token == "choiceof":
                item = cmd.pop(idx+1)
                if not type(item) == list:
                    raise Exception("target of choice must be list, got {}".format(type(item)))
                evaluated.append(choice(item))
            elif token == "len":
                calculated = len(cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "int":
                calculated = int(cmd.pop(idx+1))
                evaluated.append(calculated)
            elif token == "str":
                calculated = str(cmd.pop(idx+1))
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
            elif token == "inworld":
                check = cmd.pop(idx+1)
                calculated = check in world.actors 
                evaluated.append(calculated)
            elif token == "range":
                evaluated.append(list(range(int(cmd.pop(idx+1)))))
            elif token == "sin":
                evaluated.append(math.sin(cmd.pop(idx+1)))
            elif token == "cos":
                evaluated.append(math.cos(cmd.pop(idx+1)))
            elif token == "tan":
                evaluated.append(math.tan(cmd.pop(idx+1)))
            elif token == "atan":
                evaluated.append(math.atan(cmd.pop(idx+1)))
            elif type(token) == str and token in operators:
                left, right = evaluated.pop(), cmd.pop(idx+1)
                if token != "at":
                    if (type(left) == str and type(right) == int) or (type(right) == str and type(left) == int):
                        left, right = str(left), str(right)
                else:
                    right = int(right)
                calculated = operators[token](left, right)
                evaluated.append(calculated)
            else:
                evaluated.append(token)
        except Exception as e:
            print(actor)
            logfunc(line)
            logfunc("Error applying operators {}... {}".format(token, e))
        idx += 1
    return evaluated

