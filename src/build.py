from src.lib.SCRIPTS import SCRIPTS
from src.lib.WORLDS import WORLDS
from src.lib import ACTORS
from src import actor
from src.lib import SPRITESHEETS
from src.lib import BOXES
from src import sprites
from src import sounds

from pathlib import Path
import subprocess
from pydub import AudioSegment

import os
import sys
import pygame
os.environ["SDL_VIDEODRIVER"] = "dummy"
pygame.init()

C_PATH = Path('./c_src')
BUILD_TO = Path('./build')
MAKEFILE_PATH = Path('./c_src/Makefile')

ACTOR_BUFFER_SIZE = 1000
WORLD_ACTOR_SIZE = 1000

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
    "None": 21,
    "RAND?": 6,
    "WORLD?": 7,
    "STICKS?": 22,
    "song?": 8,
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
    "/": 3,
    "//": 4,
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
    "sin": 33,
    "cos": 34,
    "tan": 35,
    "asin": 36,
    "acos": 37,
    "atan": 38,
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
    "setjoy": 40,
    "add_input_state": 41,
}
UNIQUE_FLOATS = []
UNIQUE_STRINGS = []
STRING_INDEXERS = {}

def build():
    global ACTOR_BUFFER_SIZE, WORLD_ACTOR_SIZE
    if "--buffer" in sys.argv: ACTOR_BUFFER_SIZE = int(sys.argv[sys.argv.index("--buffer") + 1])
    if "--world" in sys.argv: WORLD_ACTOR_SIZE = int(sys.argv[sys.argv.index("--world") + 1])
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
    print("Building Image data...")
    make_image_data()
    print("    ...Done")
    print("Building Audio data...")
    make_audio_data()
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
        if scriptMap not in UNIQUE_STRINGS: UNIQUE_STRINGS.append(scriptMap)
        for key in SCRIPTS[scriptMap]:
            state, frame = key.split(":") if ":" in key else (key, "0")
            if state not in UNIQUE_STRINGS: UNIQUE_STRINGS.append(state)
            for statement in SCRIPTS[scriptMap][key]:
                for token in statement[1:]:
                    if token.startswith("#"): continue
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

    for name in WORLDS.keys():
        if name not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(name)

    for name in ACTORS.ACTORS.keys():
        if name not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(name)

    sounds.load()
    soundfx = sounds.get_sounds()
    for name in soundfx.keys():
        if name not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(name)

    songs = sounds.get_songs()
    for name in songs.keys():
        song_name = songs[name].split("/")[-1].split(".")[0]
        if song_name not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(song_name)

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

    for mapkey in BOXES.HITBOXES.keys():
        if mapkey not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(mapkey)
        for key in BOXES.HITBOXES[mapkey]:
            state, frame = key.split(":") if ":" in key else (key, "0")
            if state not in UNIQUE_STRINGS:
                UNIQUE_STRINGS.append(state)

    for mapkey in BOXES.HURTBOXES.keys():
        if mapkey not in UNIQUE_STRINGS:
            UNIQUE_STRINGS.append(mapkey)
        for key in BOXES.HURTBOXES[mapkey]:
            state, frame = key.split(":") if ":" in key else (key, "0")
            if state not in UNIQUE_STRINGS:
                UNIQUE_STRINGS.append(state)

    if "A_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("A_DOWN")
    if "A_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("A_UP")
    if "B_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("B_DOWN")
    if "B_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("B_UP")
    if "X_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("X_DOWN")
    if "X_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("X_UP")
    if "Y_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("Y_DOWN")
    if "Y_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("Y_UP")
    if "LEFT_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("LEFT_DOWN")
    if "LEFT_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("LEFT_UP")
    if "UP_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("UP_DOWN")
    if "UP_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("UP_UP")
    if "RIGHT_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("RIGHT_DOWN")
    if "RIGHT_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("RIGHT_UP")
    if "DOWN_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("DOWN_DOWN")
    if "DOWN_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("DOWN_UP")
    if "START_DOWN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("START_DOWN")
    if "START_UP" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("START_UP")
    if "_input_name" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("_input_name")
    if "" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("")
    if "COLLIDE" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("COLLIDE")
    if "START" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("START")
    if "width" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("width")
    if "height" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("height")
    if "ROOT" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("ROOT")
    if "root" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("root")
    if "MAIN" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("MAIN")
    if "HIT" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("HIT")
    if "background" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("background")
    if "XCOLLISION" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("XCOLLISION")
    if "YCOLLISION" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("YCOLLISION")
    if "top" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("top")
    if "left" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("left")
    if "bottom" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("bottom")
    if "right" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("right")
    if "name" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("name")
    if "state" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("state")
    if "frame" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("frame")
    if "x_vel" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("x_vel")
    if "y_vel" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("y_vel")
    if "direction" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("direction")
    if "rotation" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("rotation")
    if "tangible" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("tangible")
    if "physics" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("physics")
    if "platform" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("platform")
    if "self" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("self")
    if "related" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("related")
    if "x" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("x")
    if "y" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("y")
    if "w" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("w")
    if "h" not in UNIQUE_STRINGS:
        UNIQUE_STRINGS.append("h")
        
    UNIQUE_STRINGS.sort()
    string_data_dot_c = "#include \"stringmachine.h\"\n#include <stddef.h>\n"
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
#define _X {UNIQUE_STRINGS.index("x")}
#define _Y {UNIQUE_STRINGS.index("y")}
#define W {UNIQUE_STRINGS.index("w")}
#define H {UNIQUE_STRINGS.index("h")}
#define TOP {UNIQUE_STRINGS.index("top")}
#define _LEFT {UNIQUE_STRINGS.index("left")}
#define BOTTOM {UNIQUE_STRINGS.index("bottom")}
#define _RIGHT {UNIQUE_STRINGS.index("right")}
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
#define  _A_UP {UNIQUE_STRINGS.index("A_UP")}
#define  _A_DOWN {UNIQUE_STRINGS.index("A_DOWN")}
#define  _B_UP {UNIQUE_STRINGS.index("B_UP")}
#define  _B_DOWN {UNIQUE_STRINGS.index("B_DOWN")}
#define  _X_UP {UNIQUE_STRINGS.index("X_UP")}
#define  _X_DOWN {UNIQUE_STRINGS.index("X_DOWN")}
#define  _Y_UP {UNIQUE_STRINGS.index("Y_UP")}
#define  _Y_DOWN {UNIQUE_STRINGS.index("Y_DOWN")}
#define  _LEFT_UP {UNIQUE_STRINGS.index("LEFT_UP")}
#define  _LEFT_DOWN {UNIQUE_STRINGS.index("LEFT_DOWN")}
#define  _UP_UP {UNIQUE_STRINGS.index("UP_UP")}
#define  _UP_DOWN {UNIQUE_STRINGS.index("UP_DOWN")}
#define  _RIGHT_UP {UNIQUE_STRINGS.index("RIGHT_UP")}
#define  _RIGHT_DOWN {UNIQUE_STRINGS.index("RIGHT_DOWN")}
#define  _DOWN_UP {UNIQUE_STRINGS.index("DOWN_UP")}
#define  _DOWN_DOWN {UNIQUE_STRINGS.index("DOWN_DOWN")}
#define  _START_UP {UNIQUE_STRINGS.index("START_UP")}
#define  _START_DOWN {UNIQUE_STRINGS.index("START_DOWN")}
#define _INPUT_NAME {UNIQUE_STRINGS.index("_input_name")}
#define EMPTY {UNIQUE_STRINGS.index("")}
#define COLLIDE {UNIQUE_STRINGS.index("COLLIDE")}
#define _START {UNIQUE_STRINGS.index("START")}
#define _WIDTH {UNIQUE_STRINGS.index("width")}
#define _HEIGHT {UNIQUE_STRINGS.index("height")}
#define ROOT {UNIQUE_STRINGS.index("ROOT")}
#define _ROOT {UNIQUE_STRINGS.index("root")}
#define MAIN {UNIQUE_STRINGS.index("MAIN")}
#define HIT {UNIQUE_STRINGS.index("HIT")}
#define BACKGROUND {UNIQUE_STRINGS.index("background")}

extern const char* STRINGS[{len(UNIQUE_STRINGS)}];
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
    scriptmaps = []
    largest_script_map = 0
    script_data_dot_c = """// Generated with python from the red pants engine
#include "scripts.h"

void scripts_load() {
"""
    
    for scriptMap in SCRIPTS:
        script_data_dot_c += f"    add_script_map({UNIQUE_STRINGS.index(scriptMap)}, {len(scriptmaps)});\n"
        length_of_scriptmap = len(SCRIPTS[scriptMap])
        largest_script_map = max(largest_script_map, length_of_scriptmap)

        for key in SCRIPTS[scriptMap]:
            state, frame = key.split(":") if ":" in key else (key, "-1")
            state_idx = UNIQUE_STRINGS.index(state)
            scriptmaps.append(state_idx)
            scriptmaps.append(frame)
            scriptmaps.append(current_key)

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
            
        scriptmaps.append(-1000)
    script_data_dot_c += "}\nint SCRIPTS[] = {" + ", ".join(str(v) for v in scripts) + "};"
    script_data_dot_c += "\nint SCRIPT_MAPS[] = {" + ", ".join(str(v) for v in scriptmaps) + "};"
    script_data_dot_h = f"""// generated by red pants engine

#ifndef SCRIPT_DATA_LOAD
#define SCPRIT_DATA_LOAD 1
#define LARGEST_SCRIPT_MAP {largest_script_map * 3}
#define SCRIPTS_SIZE {len(scripts)}
int SCRIPTS[SCRIPTS_SIZE];
int SCRIPT_MAPS[{len(scriptmaps)}];

#endif
"""
    return script_data_dot_c, script_data_dot_h
        
def make_actor_data():
    actordata_dot_c, actor_data_dot_h = _convert_actors(ACTORS)
    with open(C_PATH / "actordata.c", "w+") as f:
        f.write(actordata_dot_c)
    with open(C_PATH / "actordata.h", "w+") as f:
        f.write(actor_data_dot_h)

def _convert_actors(ACTORS):   
    actordata_dot_c = f"""
// The following code was generated by the Red Pants Engine
// lets go baby
    
# include "actors.h"
# include <SDL2/SDL.h>
# include <SDL2/SDL_image.h>
Actor ACTORS[{len(ACTORS.ACTORS) + ACTOR_BUFFER_SIZE}];
Actor TEMPLATES[{len(ACTORS.ACTORS)}];
void actor_load() {{
"""

    for i, name in enumerate(ACTORS.ACTORS.keys()):
        template = ACTORS.ACTORS[name]
        a = actor.Actor(template)
        actordata_dot_c += f"    add_actor({UNIQUE_STRINGS.index(template['name'])}, {a.x}, {a.y}, {a.w}, {a.h}, {a.x_vel}, {a.y_vel}, -1, -1, {UNIQUE_STRINGS.index(template['scripts'])}, {UNIQUE_STRINGS.index(template['sprites'])}, -1, {UNIQUE_STRINGS.index(a._input_name) if a._input_name else -1}, {UNIQUE_STRINGS.index(a.state)}, {a.frame}, {a.direction}, {a.rotation}, {int(a.platform)}, {int(a.tangible)}, {int(a.physics)}, {int(a.updated)});\n"
        actordata_dot_c += f"    add_template_from_actorkey({i}, {UNIQUE_STRINGS.index(template['name'])});\n"

    actordata_dot_c += "}\n"

    actor_data_dot_h = f"""
    // generated by red pants engine
    // lets go baby
    #include "actors.h"
    #ifndef ACTOR_DATA_LOAD
    #define ACTOR_DATA_LOAD 1

    extern Actor ACTORS[];
    extern Actor TEMPLATES[];
    #define NUM_ACTORS {len(ACTORS.ACTORS) + ACTOR_BUFFER_SIZE}
    #define NUM_TEMPLATES {len(ACTORS.ACTORS)}
    
    #endif
    """
    return actordata_dot_c, actor_data_dot_h

def make_spritesheet_data():
    spritesheets_dot_c = _convert_spritesheets(SPRITESHEETS)
    with open(C_PATH / "spritesheets.c", "w+") as f:
        f.write(spritesheets_dot_c)

def _convert_spritesheets(SPRITESHEETS):
    spritesheets_dot_c = """
// The following code was generated by the Red Pants Engine
// lets go baby

#include "imagedata.h"
#include "sprites.h"
#include <SDL2/SDL.h>

void spritesheet_load(SDL_Renderer* rend) {
"""

    for filename in SPRITESHEETS.SPRITESHEETS.keys():
        for name in SPRITESHEETS.SPRITESHEETS[filename].keys():
            w, h = SPRITESHEETS.SPRITESHEETS[filename][name][1]
            key = name.replace(" ", "_SPACE_")
            spritesheets_dot_c += f"    load_sprite({UNIQUE_STRINGS.index(name)}, _{key}, {w}, {h}, rend);\n"  

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
        boxdata_dot_c += f"    add_hitbox_map({UNIQUE_STRINGS.index(mapkey)});\n"
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
            boxdata_dot_c += f"    add_to_hitbox_map({UNIQUE_STRINGS.index(mapkey)}, {UNIQUE_STRINGS.index(state)}, {frame}, i{mapkey}{state}{frame}xs, i{mapkey}{state}{frame}ys, i{mapkey}{state}{frame}ws, i{mapkey}{state}{frame}hs, {count});\n"


    for mapkey in BOXES.HURTBOXES.keys():
        boxdata_dot_c += f"    add_hurtbox_map({UNIQUE_STRINGS.index(mapkey)});\n"
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
            boxdata_dot_c += f"    add_to_hurtbox_map({UNIQUE_STRINGS.index(mapkey)}, {UNIQUE_STRINGS.index(state)}, {frame}, u{mapkey}{state}{frame}xs, u{mapkey}{state}{frame}ys, u{mapkey}{state}{frame}ws, u{mapkey}{state}{frame}hs, {count});\n"


    boxdata_dot_c += "}\n"
    return boxdata_dot_c

def make_world_data():
    worlddata_dot_c, worlddata_dot_h = _convert_worlds()
    with open(C_PATH / "worlddata.c", "w+") as f:
        f.write(worlddata_dot_c)
    with open(C_PATH / "worlddata.h", "w+") as f:
        f.write(worlddata_dot_h)

def _convert_worlds():
    worlddata_dot_c = f"""
    // The following code was generated by the Red Pants Engine
    // lets go baby

    # include "worlds.h"
    # include <SDL2/SDL.h>
    # include <SDL2/SDL_image.h>

    World WORLDS[{len(WORLDS)}];
    void world_load() """ + "{\n"

    for i, name in enumerate(WORLDS.keys()):
        background = WORLDS[name]["background"]
        x_lock = WORLDS[name]["x_lock"] if "x_lock" in WORLDS[name] else None
        y_lock = WORLDS[name]["y_lock"] if "y_lock" in WORLDS[name] else None
        worlddata_dot_c += f"        add_world({i}, {UNIQUE_STRINGS.index(name)}, {UNIQUE_STRINGS.index(background)}, {x_lock if x_lock is not None else 0}, {y_lock if y_lock is not None else 0});\n"
        for actor in WORLDS[name]["actors"]:
            worlddata_dot_c += f"        add_actor_to_world({UNIQUE_STRINGS.index(name)}, {UNIQUE_STRINGS.index(actor)});\n"

    worlddata_dot_c += "}\n"
    worlddata_dot_h = f"""
    // The following code was generated by the Red Pants Engine
    // lets go baby
    
    #ifndef WORLD_DATA_LOAD
    #define WORLD_DATA_LOAD 1
    #define NUM_WORLDS {len(WORLDS)}
    #define WORLD_BUFFER_SIZE {WORLD_ACTOR_SIZE}
    #endif
    """
    return worlddata_dot_c, worlddata_dot_h

def make_image_data():
    imagedata_dot_h = _convert_images()
    with open(C_PATH / "imagedata.h", "w+") as f:
        f.write(imagedata_dot_h)
    
def _convert_images():
    imagedata_dot_h = """
// The following code was generated by the Red Pants Engine
// lets go baby

#ifndef IMAGEDATA_H
#define IMAGEDATA_H
"""
    pygame.display.set_mode((1, 1))
    sprites.load()
    for name in sprites.SPRITES.keys():
        sprite = sprites.SPRITES[name]
        width, height = sprite.get_size()
        key = name.replace(" ", "_SPACE_")
        code = f"static const unsigned char _{key}[{height*width}][{4}] = {{\n"

        for y in range(height):
            for x in range(width):
                r, g, b, a = sprite.get_at((x, y))
                a = 0 if (r, g, b) == (1, 255, 1) else 255
                code += "    { " + f"{r}, {g}, {b}, {a}" + " }"
                if x == width-1 and y == height-1:
                    continue
                code += ",\n"

        code += "};\n"
        
        imagedata_dot_h += code

    imagedata_dot_h += """
#endif
"""
    return imagedata_dot_h

def make_audio_data():
    if "--noaudio" in sys.argv:
        audiodata_dot_h, audiodata_dot_c = _make_dummy_audio()
    else:
        audiodata_dot_h, audiodata_dot_c = _convert_audio()

    with open(C_PATH / "audiodata.h", "w+") as f:
        f.write(audiodata_dot_h)
    with open(C_PATH / "audiodata.c", "w+") as f:
        f.write(audiodata_dot_c)

def _make_dummy_audio():
    return f"""
// The following code was generated by the Red Pants Engine
// lets go baby

#ifndef AUDIODATA_H
#define AUDIODATA_H
# include <SDL2/SDL.h>
# include <SDL2/SDL_mixer.h>

#define NUM_SONGS 0
#define NUM_SOUNDS 0
Mix_Chunk* SOUNDS[0];
Mix_Music* SONGS[0];

#endif
// songs disabled from build script

""", """
// The following code was generated by the Red Pants Engine
// lets go baby

# include "audiodata.h"
# include "sounds.h"
# include <SDL2/SDL.h>
# include <SDL2/SDL_mixer.h>

void audio_load() {
}

// sounds disabled in build script

"""


def _convert_audio():
    sounds.load()

    song_filenames = convert_songs_to_wav()
    SOUNDS = sounds.get_sounds()
    audiodata_dot_h = f"""
// The following code was generated by the Red Pants Engine
// lets go baby

#ifndef AUDIODATA_H
#define AUDIODATA_H
# include <SDL2/SDL.h>
# include <SDL2/SDL_mixer.h>

#define NUM_SONGS {len(song_filenames)}
#define NUM_SOUNDS {len(SOUNDS)}
Mix_Chunk* SOUNDS[{len(SOUNDS)}];
Mix_Music* SONGS[{len(song_filenames)}];
"""
    audio_data_doc_c = """
// The following code was generated by the Red Pants Engine
// lets go baby

# include "audiodata.h"
# include "sounds.h"
# include <SDL2/SDL.h>
# include <SDL2/SDL_mixer.h>

void audio_load() {
"""
    idx = 0
    for name in SOUNDS.keys():
        sound = SOUNDS[name]

        raw_data = sound.get_raw()

        code = f"static const unsigned char _{name.replace(' ', '_SPACE_')}_sound[{len(raw_data)}] = {{"
        for i in range(len(raw_data)):
            if i % 16 == 0:
                code += "\n"

            code += f"    {raw_data[i]}"
            if i != len(raw_data)-1:
                code += ", "
        code += "};\n"
        audiodata_dot_h += code

        code =  f"    SOUNDS[{idx}] = Mix_QuickLoad_RAW(_{name.replace(' ', '_SPACE_')}_sound, {len(raw_data)});\n"
        code += f"    if (SOUNDS[{idx}] == NULL) {{\n"
        code += f"        printf(\"Failed to load sound {name} : %s\\n\", Mix_GetError());\n"
        code += f"    }}\n"
        code += f"    add_sound({UNIQUE_STRINGS.index(name)}, {idx});\n"
        idx += 1
        audio_data_doc_c += code
    
    for idx, song_path in enumerate(song_filenames):
        with open(song_path, "rb") as file:
            wav_data = file.read()

        song_name = song_path.split("/")[-1].split(".")[0]
        code = "static const unsigned char _" + song_name.replace(" ", "_SPACE_") + "_song[" + str(len(wav_data)) + "] = {"
        for i, byte in enumerate(wav_data):
            if i % 16 == 0:
                code += "\n"
            code += str(byte)
            if i != len(wav_data)-1:
                code += ", "
        code += "};\n"
        audiodata_dot_h += code

        code = f"   add_song_data({idx}, _{song_name.replace(' ', '_SPACE_')}_song, {len(wav_data)});\n"
        code += f"    add_song({UNIQUE_STRINGS.index(song_name)}, {idx});\n"
        code += f"    "
        audio_data_doc_c += code

    audio_data_doc_c += "}\n"
    audiodata_dot_h += "#endif\n"
    return audiodata_dot_h, audio_data_doc_c

def convert_songs_to_wav():
    print("  Converting songs to wav")
    SONGS = sounds.get_songs()
    filenames = []
    for name in SONGS.keys():
        path2mp3 = SONGS[name]
        path2wav = path2mp3.replace(".mp3", ".wav")
        print("    " + path2mp3)
        audio = AudioSegment.from_mp3(path2mp3)
        audio.export(path2wav, format="wav")
        filenames.append(path2wav)
    print("  Done...")
    return filenames
