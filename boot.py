import sys

if __name__ == "__main__":
    if "-l" in sys.argv or "--load" in sys.argv:
        from src import editor_v2
        editor_v2.load()
        editor_v2.load_all_templates_button(0)
        editor_v2.save(True)

    if "-e" in sys.argv or "--editor" in sys.argv:
        if "-v2" in sys.argv:
            from src import editor_v2
            G = editor_v2.set_up()
            editor_v2.run(G)
        else:
            from src import editor
            G = editor.set_up()
            editor.run(G)
    elif "-t" in sys.argv:
        from src import tas
        G = tas.set_up()
        tas.run(G)
    elif "-b" in sys.argv or "--build" in sys.argv:
        from src import build
        build.build()
    else:
        from src import game
        G = game.set_up()
        game.run(G)
