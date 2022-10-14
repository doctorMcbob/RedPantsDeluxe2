from src import game
from src import editor
import sys

if __name__ == "__main__":
    if "-e" in sys.argv or "--editor" in sys.argv:
        G = editor.set_up()
        editor.run(G)
    else:
        if "-l" in sys.argv or "--load" in sys.argv:
            editor.load()
            editor.load_all_templates_button(0)
            editor.save(True)
        G = game.set_up()
        game.run(G)
