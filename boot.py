from src import game
from src import editor
import sys

if __name__ == "__main__":
    if "-e" in sys.argv:
        G = editor.set_up()
        editor.run(G)
    else:
        G = game.set_up()
        game.run(G)

