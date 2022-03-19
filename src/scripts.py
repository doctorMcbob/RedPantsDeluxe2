from src import inputs
from src import frames
from src import actor as a
from src import worlds

import operator as ops

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

SCRIPTS = {}

def load():
    from src.lib import SCRIPTS as S

    for name in S.SCRIPTS.keys():
        SCRIPTS[name] = S.SCRIPTS[name]

def get_script_map(name):
    return SCRIPTS[name] if name in SCRIPTS else None

def resolve(reference, script, world, logfunc=print):
    cmd_idx = 0
    while cmd_idx < len(script):
        if not script[cmd_idx] or script[cmd_idx].startswith("#"): # comments
            cmd_idx += 1
            continue
        cmd = script[cmd_idx].split()
        cmd = evaluate_literals(cmd, reference, logfunc=logfunc)
        cmd = resolve_operators(cmd, logfunc=logfunc)
        # resolve command!
        try:
            verb = cmd.pop(0)
            if verb == "goodbye":
                world.actors.remove(reference)
            if verb == "set":
                actor, att, value = cmd
                actor = a.get_actor(reference) if actor == "self" else a.get_actor(actor)
                if hasattr(actor, att):
                    if att in ["attributes", "scripts", "hitboxes", "hurtboxes"]:
                        raise Exception("Cannot write over {} {}".format(att, cmd_idx))
                    setattr(actor, att, value)                        
                else:
                    a.get_actor(reference).attributes[att] = value
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
                if key not in a.get_actor(reference).scripts:
                    raise Exception("Cannot exec {}. Does not exist.".format(key))
                resolve(reference, a.get_actor(reference).scripts[key], world, logfunc=logfunc)
            if verb == "print":
                print(cmd.pop(0))
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
                if name not in world.actors:
                    raise Exception("{} not in world".format(name))
                world.actors.remove(name)
                newworld = worlds.get_world(cmd.pop())
                newworld.actors.append(name)
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

        except Exception as e:
            logfunc(cmd)
            logfunc("Error resolving {}... {}".format(verb, e))

        cmd_idx += 1

def evaluate_literals(cmd, reference, logfunc=print):
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
                actor, att = token.split(".")
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
            logfunc(cmd)
            logfunc("Error applying operators {}... {}".format(token, e))
        idx += 1
    return evaluated

