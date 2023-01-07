import sys

if __name__ == "__main__":
    if "-e" in sys.argv or "--editor" in sys.argv:
        from src import editor
        G = editor.set_up()
        editor.run(G)
    elif "-t" in sys.argv:
        from src import tas
        G = tas.set_up()
        tas.run(G)
    else:
        from src import game
        if "-l" in sys.argv or "--load" in sys.argv:
            from src import editor
            editor.load()
            editor.load_all_templates_button(0)
            editor.save(True)
        G = game.set_up()
        game.run(G)
