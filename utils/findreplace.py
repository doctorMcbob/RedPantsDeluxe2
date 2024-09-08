import os
import sys

SCRIPTS_LOCATION = "scripts/"

#if "-f" not in sys.argv or "-r" not in sys.argv:
#    print("Use -f {} for find text and -r {} for replace text")
#    quit()


#find = sys.argv[sys.argv.index("-f") + 1]
#replace = sys.argv[sys.argv.index("-r") + 1]
find = "set related state LAUNCHED"
replace = """set related state LAUNCHED
set related frame 0"""

if input(f"Find \"{find}\" and replace with \"{replace}\" ? (y/n): ") != "y": quit()

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

    if find in code:
        if input(f"replace in {f}? (y/n): ") == "y":
            changed = replace.join(code.split(find)) 

    if changed:
        with open(f, "w") as script:
            script.write(changed)
        changed = False    
