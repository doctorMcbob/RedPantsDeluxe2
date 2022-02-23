from src import game
from src import editor
import sys

if __name__ == "__main__":
    if "-e" in sys.argv:
        G = editor.set_up()
        editor.run(G)
    else:
        G = game.set_up(loadscripts="-R" in sys.argv)
        game.run(G)
