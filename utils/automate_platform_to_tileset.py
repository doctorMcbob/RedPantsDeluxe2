import os
import sys

SCRIPTS_LOCATION = "scripts/"

filenames = []
for _, _, files in os.walk(SCRIPTS_LOCATION):
    for f in files:
        if f[-3:] == ".rp":
            filenames.append(SCRIPTS_LOCATION + f)

print("found {} redpantsscript files".format(len(filenames)))
for f in filenames:
    changed = False
    with open(f, "r") as script:
        code = script.read()

    if "set self platform 1" in code:
        print()
        print("{} has platform 1".format(f))

        inserts = {}
        names = {}
        code = code.splitlines()
        for i, line in enumerate(code):
            if any(token.endswith("00") for token in line.split()):
               state, img, offs = line.split()
               names[state.split("00")[0]] = img.split("00")[0]
               
            if "set self state" in line:
                state = line.split("set self state ")[1]
                if state in names:
                    inserts[i] = "set self tileset {}".format(names[state])

        idxs = list(inserts.keys())
        idxs.sort()
        for idx in idxs[::-1]:
            code.insert(idx+1, inserts[idx])

        #changed = "\n".join(code)

    if changed:
        with open(f, "w") as script:
            script.write(changed)
        changed = False    
