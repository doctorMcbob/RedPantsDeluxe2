from src.lib.SCRIPTS import SCRIPTS
from src.lib.WORLDS import WORLDS
from src.lib import ACTORS
from src import actor
from src.lib import SPRITESHEETS
from src.lib import BOXES

from pathlib import Path
import subprocess

import os

C_PATH = Path('./c_src')
BUILD_TO = Path('./build')
MAKEFILE_PATH = Path('./c_src/Makefile')

if not os.path.isdir(C_PATH): raise IOError("Could not find /c_src directory")
if not os.path.isdir(BUILD_TO): os.mkdir(BUILD_TO)

LITERAL_KEYWORDS = {
    "None": 0,
    "RAND?": 6,
    "WORLD?": 7,
    "SONG?": 8,
    "COLLIDE?": 9,
    "[]": 10,
    "inpA": 11,
    "inpB": 12,
    "inpX": 13,
    "inpY": 14,
    "inpLEFT": 15,
    "inpUP": 16,
    "inpRIGHT": 17,
    "inpDOWN": 18,
    "inpSTART": 19,
    "inpEVENTS": 20,
}

OPERATORS = {
    "+": 0,
    "-": 1,
    "*": 2,
    "//": 4,
    "////": 3,
    "%": 5,
    "**": 6,
    "==": 7,
    "<": 8,
    ">": 9,
    "<=": 10,
    ">=": 11,
    "!=": 12,
    "and": 13,
    "or": 14,
    "not": 15,
    "nor": 16,
    "in": 17,
    "at": 18,
    "int": 19,
    "str": 20,
    "min": 21,
    "max": 22,
    "len": 23,
    "countof": 24,
    "exists": 25,
    "hasframe": 26,
    "choiceof": 27,
    "isframe": 28,
    "isinputstate": 29,
    "abs": 30,
    "range": 31,
    "inworld": 32,
}

VERBS = {
    "quit": 0,
    "goodbye": 1,
    "break": 2,
    "reset": 3,
    "set": 4,
    "reassign": 5,
    "if": 6,
    "endif": 7,
    "exec": 8,
    "back": 9,
    "front": 10,
    "img": 11,
    "activate": 12,
    "deactivate": 13,
    "killframe": 14,
    "makeframe": 15,
    "focus": 16,
    "scrollbound": 17,
    "view": 18,
    "move": 19,
    "place": 20,
    "take": 21,
    "takeall": 22,
    "rebrand": 23,
    "remove": 24,
    "add": 25,
    "hitboxes": 26,
    "hurtboxes": 27,
    "create": 28,
    "update": 29,
    "sfx": 30,
    "song": 31,
    "sfxoff": 32,
    "songoff": 33,
    "offsetbgscrollx": 34,
    "offsetbgscrolly": 35,
    "for": 36,
    "endfor": 37,
    "print": 38,
    "update_sticks": 39,
}

def build():
    print("Building Script Data...")
    make_script_data()
    print("    ...Done")
    print("Building Script Data...")
    make_actor_data()
    print("    ...Done")
    print("Building Script Data...")
    make_spritesheet_data()
    print("    ...Done")
    print("Building Script Data...")
    make_box_data()
    print("    ...Done")
    print("Building Script Data...")
    make_world_data()
    print("    ...Done")

    print(f"Running Makefile at {MAKEFILE_PATH}")
    make_result = subprocess.run(
        ['make', '-f', str(MAKEFILE_PATH), f'C_DIR={str(C_PATH)}', f'BUILD_DIR={str(BUILD_TO)}'],
        capture_output=True, text=True)

    # Check for errors
    if make_result.returncode != 0:
        print(make_result.stderr)
    else:
        print("Build finished.")

def make_script_data():
    script_data_dot_c = _convert_scripts(SCRIPTS)
    with open(C_PATH / "scriptdata.c", "w+") as f:
        f.write(script_data_dot_c)

def _convert_scripts(SCRIPTS):
    script_data_dot_c = """// Generated with python from the red pants engine
    #include "scripts.h"

    void scripts_load() {
    """

    scriptKey = 0
    for mapKey in SCRIPTS.keys():
        script_data_dot_c += f"\n"
        script_data_dot_c += f"    add_script_map(\"{mapKey}\");\n"
        for key in SCRIPTS[mapKey]:
            state, frame = key.split(":") if ":" in key else (key, "0")
            script_data_dot_c += f"\n    add_script({scriptKey});\n"
            script_data_dot_c += f"    add_script_to_script_map(\"{mapKey}\", \"{state}\", {frame}, {scriptKey});\n"
            for i, statement in enumerate(SCRIPTS[mapKey][key]):
                if not statement: continue
                verb = statement[0]
                if verb.startswith("#"): continue
                if verb not in VERBS:
                    print(f"The lost verb: {verb}")
                    print(mapKey, key, statement)
                    continue
                script_data_dot_c += f"""
        //{verb} {statement}
        Statement* s{scriptKey}_{i} = new_statement({VERBS[verb]});
        add_statement_to_script({scriptKey}, s{scriptKey}_{i});\n"""

                for j, token in enumerate(statement[1:]):
                    if token in OPERATORS:
                        script_data_dot_c += f"""
        SyntaxNode* sn{scriptKey}_{i}_{j} = new_syntax_node(OPERATOR);
        sn{scriptKey}_{i}_{j}->data.i = {OPERATORS[token]};\n"""
                    elif token in LITERAL_KEYWORDS:
                        script_data_dot_c += f"""
        SyntaxNode* sn{scriptKey}_{i}_{j} = new_syntax_node({LITERAL_KEYWORDS[token]});\n"""
                    else:
                        try:
                            int(token)
                            script_data_dot_c += f"""
        SyntaxNode* sn{scriptKey}_{i}_{j} = new_syntax_node(INT);
        sn{scriptKey}_{i}_{j}->data.i = {token};\n"""
                        except ValueError:
                            try:
                                float(token)
                                script_data_dot_c += f"""
        SyntaxNode* sn{scriptKey}_{i}_{j} = new_syntax_node(FLOAT);
        sn{scriptKey}_{i}_{j}->data.f = {token};\n"""
                            except ValueError:
                                if "." in token:
                                    script_data_dot_c += f"""
        SyntaxNode* sn{scriptKey}_{i}_{j}_;\n"""
                                    words = token.split(".");
                                    first = words.pop(0);
                                    script_data_dot_c += f"""
        sn{scriptKey}_{i}_{j}_ = new_syntax_node(STRING);
        sn{scriptKey}_{i}_{j}_->data.s = (char*)malloc({len(first) + 1});
        strncpy(sn{scriptKey}_{i}_{j}_->data.s, "{first}", {len(first) + 1});
        add_node_to_statement(s{scriptKey}_{i}, sn{scriptKey}_{i}_{j}_);\n"""
                                    for word in words:
                                        script_data_dot_c += f"""
        sn{scriptKey}_{i}_{j}_ = new_syntax_node(DOT);
        add_node_to_statement(s{scriptKey}_{i}, sn{scriptKey}_{i}_{j}_);               
        sn{scriptKey}_{i}_{j}_ = new_syntax_node(STRING);
        sn{scriptKey}_{i}_{j}_->data.s = (char*)malloc({len(word) + 1});
        strncpy(sn{scriptKey}_{i}_{j}_->data.s, "{word}", {len(word) + 1});
        add_node_to_statement(s{scriptKey}_{i}, sn{scriptKey}_{i}_{j}_);\n"""
                                    continue

                                script_data_dot_c += f"""
        SyntaxNode* sn{scriptKey}_{i}_{j} = new_syntax_node(STRING);
        sn{scriptKey}_{i}_{j}->data.s = (char*)malloc({len(token) + 1});
        strncpy(sn{scriptKey}_{i}_{j}->data.s, "{token}", {len(token) + 1});\n"""

                    script_data_dot_c += f"    add_node_to_statement(s{scriptKey}_{i}, sn{scriptKey}_{i}_{j});\n"
            scriptKey += 1
    script_data_dot_c += "}\n"
    return script_data_dot_c

def make_actor_data():
    actordata_dot_c = _convert_actors(ACTORS)
    with open(C_PATH / "actordata.c", "w+") as f:
        f.write(actordata_dot_c)

def _convert_actors(ACTORS):   
    actordata_dot_c = """
    // The following code was generated by the Red Pants Engine
    // lets go baby

    # include "actors.h"
    # include <SDL2/SDL.h>
    # include <SDL2/SDL_image.h>

    void actor_load() {
    """

    for name in ACTORS.ACTORS.keys():
        template = ACTORS.ACTORS[name]
        a = actor.Actor(template)
        actordata_dot_c += f"    add_actor(\"{template['name']}\", {a.x}, {a.y}, {a.w}, {a.h}, {a.x_vel}, {a.y_vel}, NULL, NULL, \"{template['scripts']}\", \"{template['sprites']}\", NULL, {a._input_name if a._input_name else 'NULL'}, \"{a.state}\", {a.frame}, {a.direction}, {a.rotation}, {int(a.platform)}, {int(a.tangible)}, {int(a.physics)}, {int(a.updated)});\n"

    actordata_dot_c += "}\n"
    return actordata_dot_c

def make_spritesheet_data():
    spritesheets_dot_c = _convert_spritesheets(SPRITESHEETS)
    with open(C_PATH / "spritesheets.c", "w+") as f:
        f.write(spritesheets_dot_c)

def _convert_spritesheets(SPRITESHEETS):
    spritesheets_dot_c = """# include "sprites.h"
    # include <SDL2/SDL.h>
    # include <SDL2/SDL_image.h>

    void spritesheet_load(SDL_Renderer* rend) {
    """

    for filename in SPRITESHEETS.SPRITESHEETS.keys():
        names = list(SPRITESHEETS.SPRITESHEETS[filename].keys())
        xs    = [SPRITESHEETS.SPRITESHEETS[filename][name][0][0] for name in names]
        ys    = [SPRITESHEETS.SPRITESHEETS[filename][name][0][1] for name in names]
        ws    = [SPRITESHEETS.SPRITESHEETS[filename][name][1][0] for name in names]
        hs    = [SPRITESHEETS.SPRITESHEETS[filename][name][1][1] for name in names]
        length = len(names)
        name = filename.split(".")[0]

        spritesheets_dot_c += f"    const char* {name}names[{length}] = " + "{" + repr(names)[1:-1].replace("'", '"') + "};\n"
        spritesheets_dot_c += f"    int {name}xs[{length}] = " + "{" + ", ".join(str(v) for v in xs) + "};\n"
        spritesheets_dot_c += f"    int {name}ys[{length}] = " + "{" + ", ".join(str(v) for v in ys) + "};\n"
        spritesheets_dot_c += f"    int {name}ws[{length}] = " + "{" + ", ".join(str(v) for v in ws) + "};\n"
        spritesheets_dot_c += f"    int {name}hs[{length}] = " + "{" + ", ".join(str(v) for v in hs) + "};\n"


        spritesheets_dot_c += f"""
        load_spritesheet(rend, "img/{filename}", {name}names, {name}xs, {name}ys, {name}ws, {name}hs, {length})\n;
    """

    keys = set()
    for key in SPRITESHEETS.OFFSETS.keys():
        if type(SPRITESHEETS.OFFSETS[key]) == dict:
            for name in SPRITESHEETS.OFFSETS[key].keys():
                if name in keys: continue
                x, y = SPRITESHEETS.OFFSETS[key][name]
                spritesheets_dot_c += f"    add_offset(\"{name}\", {x}, {y});\n"
                keys.add(name)
        else:
            if key in keys: continue
            x, y = SPRITESHEETS.OFFSETS[key]
            spritesheets_dot_c += f"    add_offset(\"{key}\", {x}, {y});\n"
            keys.add(key)

    for key in SPRITESHEETS.SPRITEMAPS:
        if "\\" in key: continue
        spritemap = SPRITESHEETS.SPRITEMAPS[key]
        spritesheets_dot_c += f"    add_sprite_map(\"{key}\");\n"
        for index in spritemap:
            spritekey = spritemap[index]
            if "\\" in spritekey: continue
            state, frame = index.split(":") if ":" in index else (index, "0")
            spritesheets_dot_c += f"    add_to_sprite_map(\"{key}\", \"{state}\", {frame}, \"{spritekey}\");\n"

    spritesheets_dot_c += "}\n"
    return spritesheets_dot_c

def make_box_data():
    boxdata_dot_c = _convert_boxes()
    with open(C_PATH / "boxdata.c", "w+") as f:
        f.write(boxdata_dot_c)

def _convert_boxes():
    boxdata_dot_c = """# include "boxes.h"
    # include <SDL2/SDL.h>

    void boxes_load() {
    """

    for mapkey in BOXES.HITBOXES.keys():
        boxdata_dot_c += f"    add_hitbox_map(\"{mapkey}\");\n"
        for key in BOXES.HITBOXES[mapkey]:
            if not BOXES.HITBOXES[mapkey][key]: continue
            count = len(BOXES.HITBOXES[mapkey][key])
            xs = [rect[0][0] for rect in BOXES.HITBOXES[mapkey][key]]
            ys = [rect[0][1] for rect in BOXES.HITBOXES[mapkey][key]]
            ws = [rect[1][0] for rect in BOXES.HITBOXES[mapkey][key]]
            hs = [rect[1][1] for rect in BOXES.HITBOXES[mapkey][key]]
            state, frame = key.split(":") if ":" in key else (key, "0")
            boxdata_dot_c += f"    int i{mapkey}{state}{frame}xs[{count}] = " + "{" + ", ".join(str(v) for v  in xs) + "};\n"
            boxdata_dot_c += f"    int i{mapkey}{state}{frame}ys[{count}] = " + "{" + ", ".join(str(v) for v  in ys) + "};\n"
            boxdata_dot_c += f"    int i{mapkey}{state}{frame}ws[{count}] = " + "{" + ", ".join(str(v) for v  in ws) + "};\n"
            boxdata_dot_c += f"    int i{mapkey}{state}{frame}hs[{count}] = " + "{" + ", ".join(str(v) for v  in hs) + "};\n"
            boxdata_dot_c += f"    add_to_hitbox_map(\"{mapkey}\", \"{state}\", {frame}, i{mapkey}{state}{frame}xs, i{mapkey}{state}{frame}ys, i{mapkey}{state}{frame}hs, i{mapkey}{state}{frame}ws, {count});\n"


    for mapkey in BOXES.HURTBOXES.keys():
        boxdata_dot_c += f"    add_hurtbox_map(\"{mapkey}\");\n"
        for key in BOXES.HURTBOXES[mapkey]:
            if not BOXES.HURTBOXES[mapkey][key]: continue
            count = len(BOXES.HURTBOXES[mapkey][key])
            xs = [rect[0][0] for rect in BOXES.HURTBOXES[mapkey][key]]
            ys = [rect[0][1] for rect in BOXES.HURTBOXES[mapkey][key]]
            ws = [rect[1][0] for rect in BOXES.HURTBOXES[mapkey][key]]
            hs = [rect[1][1] for rect in BOXES.HURTBOXES[mapkey][key]]
            state, frame = key.split(":") if ":" in key else (key, "0")
            boxdata_dot_c += f"    int u{mapkey}{state}{frame}xs[{count}] = " + "{" + ", ".join(str(v) for v  in xs) + "};\n"
            boxdata_dot_c += f"    int u{mapkey}{state}{frame}ys[{count}] = " + "{" + ", ".join(str(v) for v  in ys) + "};\n"
            boxdata_dot_c += f"    int u{mapkey}{state}{frame}ws[{count}] = " + "{" + ", ".join(str(v) for v  in ws) + "};\n"
            boxdata_dot_c += f"    int u{mapkey}{state}{frame}hs[{count}] = " + "{" + ", ".join(str(v) for v  in hs) + "};\n"
            boxdata_dot_c += f"    add_to_hurtbox_map(\"{mapkey}\", \"{state}\", {frame}, u{mapkey}{state}{frame}xs, u{mapkey}{state}{frame}ys, u{mapkey}{state}{frame}hs, u{mapkey}{state}{frame}ws, {count});\n"


    boxdata_dot_c += "}\n"
    return boxdata_dot_c

def make_world_data():
    worlddata_dot_c = _convert_worlds()
    with open(C_PATH / "worlddata.c", "w+") as f:
        f.write(worlddata_dot_c)

def _convert_worlds():
    worlddata_dot_c = """
    // The following code was generated by the Red Pants Engine
    // lets go baby

    # include "worlds.h"
    # include <SDL2/SDL.h>
    # include <SDL2/SDL_image.h>

    void world_load() {
    """

    for name in WORLDS.keys():
        background = WORLDS[name]["background"]
        x_lock = WORLDS[name]["x_lock"]
        y_lock = WORLDS[name]["y_lock"]
        worlddata_dot_c += f"    add_world(\"{name}\", \"{background}\", {x_lock if x_lock is not None else 0}, {y_lock if y_lock is not None else 0});\n"
        for actor in WORLDS[name]["actors"]:
            worlddata_dot_c += f"    add_actor_to_world(\"{name}\", \"{actor}\");\n"

    worlddata_dot_c += "}\n"
    return worlddata_dot_c

