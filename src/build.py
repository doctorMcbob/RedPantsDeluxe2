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

LITERAL_TYPES = {
    "int": 1,
    "float": 2,
    "string": 3,
    "operator": 4,
    "dot": 5,
}

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
UNIQUE_FLOATS = []
UNIQUE_STRINGS = []
STRING_INDEXERS = {}
SCRIPT_MAP_MAP = {}

def build():
    print("Building Script Data...")
    make_script_data()
    print("    ...Done")
    print("Building Sprite Data...")
    make_spritesheet_data()
    print("    ...Done")
    print("Building Box Data...")
    make_box_data()
    print("    ...Done")
    print("Building World Data...")
    make_world_data()
    print("    ...Done")
    print("Building Actor Data...")
    make_actor_data()
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
    string_data_dot_c, string_data_dot_h =  _intern_strings(SCRIPTS)
    float_data_dot_c, float_data_dot_h =_intern_floats(SCRIPTS)
    script_data_dot_c, script_data_dot_h = _convert_scripts(SCRIPTS)
    with open(C_PATH / "scriptdata.c", "w+") as f:
        f.write(script_data_dot_c)
    with open(C_PATH / "scriptdata.h", "w+") as f:
        f.write(script_data_dot_h)

    with open(C_PATH / "stringdata.c", "w+") as f:
        f.write(string_data_dot_c)
    with open(C_PATH / "stringdata.h", "w+") as f:
        f.write(string_data_dot_h)

    with open(C_PATH / "floatdata.c", "w+") as f:
        f.write(float_data_dot_c)
    with open(C_PATH / "floatdata.h", "w+") as f:
        f.write(float_data_dot_h)

def _intern_strings(SCRIPTS):
    for scriptMap in SCRIPTS:
        for key in SCRIPTS[scriptMap]:
            state, frame = key.split(":") if ":" in key else (key, "0")
            if state not in UNIQUE_STRINGS: UNIQUE_STRINGS.append(state)
            for statement in SCRIPTS[scriptMap][key]:
                for token in statement:
                    if not token or token.isspace() or token.startswith("#"): continue
                    if token in VERBS: continue
                    if token in OPERATORS: continue
                    try:
                        int(token)
                        continue
                    except ValueError:
                        try:
                            float(token)
                            continue
                        except ValueError:
                            if "." in token:
                                for t in token.split("."):
                                    if t not in UNIQUE_STRINGS:
                                        UNIQUE_STRINGS.append(t)
                            else:
                                if token not in UNIQUE_STRINGS:
                                    UNIQUE_STRINGS.append(token)

    for name in ACTORS.ACTORS.keys():
        if name not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(name)

    for filename in SPRITESHEETS.SPRITESHEETS.keys():
        for name in SPRITESHEETS.SPRITESHEETS[filename].keys():
            if name not in UNIQUE_STRINGS:
                UNIQUE_STRINGS.append(name)

    for key in SPRITESHEETS.SPRITEMAPS:
        if key not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(key)

    for key in SPRITESHEETS.OFFSETS.keys():
        if key not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(key)
        if type(SPRITESHEETS.OFFSETS[key]) == dict:
            for name in SPRITESHEETS.OFFSETS[key].keys():
                if name not in UNIQUE_STRINGS:
                    UNIQUE_STRINGS.append(name)


    for key in SPRITESHEETS.SPRITEMAPS:
        spritemap = SPRITESHEETS.SPRITEMAPS[key]
        if "\\" in key: continue
        if key not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(key)
        for index in spritemap:
            spritekey = spritemap[index]
            if "\\" in spritekey: continue
            state, frame = index.split(":") if ":" in index else (index, "0")
            if spritekey not in UNIQUE_STRINGS:
                UNIQUE_STRINGS.append(spritekey)
            if state not in UNIQUE_STRINGS:
                UNIQUE_STRINGS.append(state)

    UNIQUE_STRINGS.sort()
    string_data_dot_c = "#include \"stringmachine.h\"\n#include <stddef.h>\n"
    string_data_dot_c += "const char** D_STRINGS = NULL;\n"
    string_data_dot_c += f"int NUM_STRINGS = {len(UNIQUE_STRINGS)};\n"
    string_data_dot_c += "int DYNAMIC_STRINGS = 0;\n"
    string_data_dot_c += "const char* STRINGS[] = {\n  \"" + "\",\n  \"".join(UNIQUE_STRINGS) + "\"\n};\n"
    string_data_dot_c += "const int STRING_LENS[] = {\n  " + ",\n  ".join(str(len(v)) for v in UNIQUE_STRINGS) + "\n};\n"
    
    string_data_dot_c += "void load_string_indexers() {\n"
    for idx, token in enumerate(UNIQUE_STRINGS):
        if token[0:2] not in STRING_INDEXERS:
            STRING_INDEXERS[token[0:2]] = idx
            string_data_dot_c += f"  add_indexer(\"{token[0:2]}\", {idx});\n"
    string_data_dot_c += "}\n"
    
    string_data_dot_h = f"""
#include <stddef.h>
#ifndef STRING_DATA_LOAD
#define STRING_DATA_LOAD 1

#define SELF {UNIQUE_STRINGS.index("self")}
#define RELATED {UNIQUE_STRINGS.index("related")}
#define XCOLLISION {UNIQUE_STRINGS.index("XCOLLISION")}
#define YCOLLISION {UNIQUE_STRINGS.index("YCOLLISION")}
#define X {UNIQUE_STRINGS.index("x")}
#define Y {UNIQUE_STRINGS.index("y")}
#define W {UNIQUE_STRINGS.index("w")}
#define H {UNIQUE_STRINGS.index("h")}
#define TOP {UNIQUE_STRINGS.index("top")}
#define LEFT {UNIQUE_STRINGS.index("left")}
#define BOTTOM {UNIQUE_STRINGS.index("bottom")}
#define RIGHT {UNIQUE_STRINGS.index("right")}
#define NAME {UNIQUE_STRINGS.index("name")}
#define STATE {UNIQUE_STRINGS.index("state")}
#define FRAME {UNIQUE_STRINGS.index("frame")}
#define X_VEL {UNIQUE_STRINGS.index("x_vel")}
#define Y_VEL {UNIQUE_STRINGS.index("y_vel")}
#define DIRECTION {UNIQUE_STRINGS.index("direction")}
#define ROTATION {UNIQUE_STRINGS.index("rotation")}
#define TANGIBLE {UNIQUE_STRINGS.index("tangible")}
#define PHYSICS {UNIQUE_STRINGS.index("physics")}
#define PLATFORM {UNIQUE_STRINGS.index("platform")}

extern const char* STRINGS[{len(UNIQUE_STRINGS)}];
extern const char** D_STRINGS;
extern int STRING_LENS[{len(UNIQUE_STRINGS)}];
extern int NUM_STRINGS;
extern int DYNAMIC_STRINGS;
#endif
"""
    return string_data_dot_c, string_data_dot_h

def _intern_floats(SCRIPTS): 
    for scriptMap in SCRIPTS:
        for key in SCRIPTS[scriptMap]:
            for statement in SCRIPTS[scriptMap][key]:
                for token in statement:
                    try:
                        int(token)
                        continue
                    except ValueError:
                        try:
                            f = float(token)
                            if f not in UNIQUE_FLOATS:
                                UNIQUE_FLOATS.append(f)
                        except ValueError:
                            continue
    UNIQUE_FLOATS.sort()

    float_data_dot_c = "const float FLOATS[] = {\n  " + ",\n  ".join(str(v) for v in UNIQUE_FLOATS) + "\n};"
    float_data_dot_h = f"""
#ifndef FLOAT_DATA_LOAD
#define FLOAT_DATA_LOAD 1

const float FLOATS[{len(UNIQUE_FLOATS)}];
int NUM_FLOATS = {len(UNIQUE_FLOATS)};
#endif
"""
    return float_data_dot_c, float_data_dot_h


def _convert_scripts(SCRIPTS):
    current_key = 0
    scripts = []
    script_data_dot_c = """// Generated with python from the red pants engine
#include "scripts.h"

void scripts_load() {
"""
    
    scriptMap_idx = -1
    for scriptMap in SCRIPTS:
        scriptMap_idx += 1
        script_data_dot_c += f"    add_script_map({scriptMap_idx});\n"
        SCRIPT_MAP_MAP[scriptMap] = scriptMap_idx
        for key in SCRIPTS[scriptMap]:
            state, frame = key.split(":") if ":" in key else (key, "0")
            state_idx = UNIQUE_STRINGS.index(state)
            script_data_dot_c += f"    add_script_to_script_map({scriptMap_idx}, {state_idx}, {frame}, {current_key});\n"
            for statement in SCRIPTS[scriptMap][key]:
                if not statement: continue
                verb = statement[0]
                if verb.startswith("#"): continue
                if verb not in VERBS:
                    print("Lost verb: ", verb)
                    print(scriptMap, key, statement)
                    continue
                scripts.append(VERBS[verb])
                for token in statement[1:]:
                    if token in OPERATORS:
                        scripts.append(LITERAL_TYPES["operator"])
                        scripts.append(OPERATORS[token])
                    elif token in LITERAL_KEYWORDS:
                        scripts.append(LITERAL_KEYWORDS[token])
                    else:
                        try:
                            ti = int(token)
                            scripts.append(LITERAL_TYPES["int"])
                            scripts.append(ti)
                        except ValueError:
                            try:
                                tf = float(token)
                                if tf not in UNIQUE_FLOATS:
                                    raise TypeError(f"Float not found {tf}")
                                scripts.append(LITERAL_TYPES["float"])
                                scripts.append(UNIQUE_FLOATS.index(tf))
                            except ValueError:
                                if not token or token.isspace(): continue
                                if "." in token:
                                    dot_seperated = token.split(".")
                                    while dot_seperated:
                                        t = dot_seperated.pop(0)
                                        if t not in UNIQUE_STRINGS:
                                            raise TypeError(f"String not found {t}")
                                        scripts.append(LITERAL_TYPES["string"])
                                        scripts.append(UNIQUE_STRINGS.index(t))
                                        if dot_seperated:
                                            scripts.append(LITERAL_TYPES["dot"])
                                else:
                                    if token not in UNIQUE_STRINGS:
                                        raise TypeError(f"String not found {token}")
                                    scripts.append(LITERAL_TYPES["string"])
                                    scripts.append(UNIQUE_STRINGS.index(token))
                scripts.append(-1000)
            scripts.append(-2000)
            current_key = len(scripts)
            
    script_data_dot_c += "}\nint SCRIPTS[] = {" + ", ".join(str(v) for v in scripts) + "};"
    script_data_dot_h = f"""// generated by red pants engine

#ifndef SCRIPT_DATA_LOAD
#define SCPRIT_DATA_LOAD 1

#define SCRIPT_MAP_SIZE {len(SCRIPT_MAP_MAP)}
#define SCRIPTS_SIZE {len(scripts)}
int SCRIPTS[SCRIPTS_SIZE];

#endif
"""
    return script_data_dot_c, script_data_dot_h
        
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
        actordata_dot_c += f"    add_actor({UNIQUE_STRINGS.index(template['name'])}, {a.x}, {a.y}, {a.w}, {a.h}, {a.x_vel}, {a.y_vel}, -1, -1, {SCRIPT_MAP_MAP[template['scripts']]}, {UNIQUE_STRINGS.index(template['sprites'])}, -1, {UNIQUE_STRINGS.index(a._input_name) if a._input_name else -1}, {UNIQUE_STRINGS.index(a.state)}, {a.frame}, {a.direction}, {a.rotation}, {int(a.platform)}, {int(a.tangible)}, {int(a.physics)}, {int(a.updated)});\n"

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
        names = list(UNIQUE_STRINGS.index(v) for v in SPRITESHEETS.SPRITESHEETS[filename].keys())
        xs    = [SPRITESHEETS.SPRITESHEETS[filename][name][0][0] for name in SPRITESHEETS.SPRITESHEETS[filename].keys()]
        ys    = [SPRITESHEETS.SPRITESHEETS[filename][name][0][1] for name in SPRITESHEETS.SPRITESHEETS[filename].keys()]
        ws    = [SPRITESHEETS.SPRITESHEETS[filename][name][1][0] for name in SPRITESHEETS.SPRITESHEETS[filename].keys()]
        hs    = [SPRITESHEETS.SPRITESHEETS[filename][name][1][1] for name in SPRITESHEETS.SPRITESHEETS[filename].keys()]
        length = len(names)
        name = filename.split(".")[0]

        spritesheets_dot_c += f"    int {name}names[{length}] = " + "{" + ", ".join(str(v) for v in names) + "};\n"
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
                spritesheets_dot_c += f"    add_offset({UNIQUE_STRINGS.index(name)}, {x}, {y});\n"
                keys.add(name)
        else:
            if key in keys: continue
            x, y = SPRITESHEETS.OFFSETS[key]
            spritesheets_dot_c += f"    add_offset({UNIQUE_STRINGS.index(key)}, {x}, {y});\n"
            keys.add(key)

    for key in SPRITESHEETS.SPRITEMAPS:
        if "\\" in key: continue
        spritemap = SPRITESHEETS.SPRITEMAPS[key]
        spritesheets_dot_c += f"    add_sprite_map({UNIQUE_STRINGS.index(key)});\n"
        for index in spritemap:
            spritekey = spritemap[index]
            if "\\" in spritekey: continue
            state, frame = index.split(":") if ":" in index else (index, "0")
            spritesheets_dot_c += f"    add_to_sprite_map({UNIQUE_STRINGS.index(key)}, {UNIQUE_STRINGS.index(state)}, {frame}, {UNIQUE_STRINGS.index(spritekey)});\n"

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
        worlddata_dot_c += f"    add_world({UNIQUE_STRINGS.index(name)}, {UNIQUE_STRINGS.index(background)}, {x_lock if x_lock is not None else 0}, {y_lock if y_lock is not None else 0});\n"
        for actor in WORLDS[name]["actors"]:
            worlddata_dot_c += f"    add_actor_to_world({UNIQUE_STRINGS.index(name)}, {UNIQUE_STRINGS.index(actor)});\n"

    worlddata_dot_c += "}\n"
    return worlddata_dot_c
