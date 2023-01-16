"""
export frames as images
convert them to gif
"""
import os
import sys
import imageio
import pygame
from pygame.locals import *
from pathlib import Path
from datetime import datetime

ROOT_PATH = Path('.')
PATH_TO_REPLAY = ROOT_PATH / ("replays/" if "-o" not in sys.argv else "replays/" + sys.argv[sys.argv.index("-o") + 1])
PATH_TO_DUMP = PATH_TO_REPLAY / "dump"

if not os.path.isdir(PATH_TO_REPLAY): os.mkdir(PATH_TO_REPLAY)
if not os.path.isdir(PATH_TO_DUMP): os.mkdir(PATH_TO_DUMP)

FRAME = 0
SAVED = []

START = None

FPS = 30
GIF_SIZE = FPS * 8

def save_surface(surf):
    w, h = (surf.get_width(), surf.get_height())
    save = pygame.Surface((w, h))
    save.blit(surf, (0, 0))
    SAVED.append(save)
    if len(SAVED) >= GIF_SIZE:
        SAVED.pop(0)

def save_em(frame=0):
    global SAVED
    for i, surf in enumerate(SAVED):
        pygame.image.save(surf, str(PATH_TO_DUMP/"{}.png".format(i+frame)))
    SAVED = []

def make_gif(filename=None, fps=FPS):
    if filename is None:
        filename = "{}.gif".format(datetime.now())
    images = []
    num_imgs = len(os.listdir(ROOT_PATH / PATH_TO_DUMP))
    for i in range(num_imgs):
        file_name = "{}.png".format(i)
        file_path = os.path.join(ROOT_PATH / PATH_TO_DUMP, file_name)
        images.append(imageio.imread(file_path))
    imageio.mimsave(os.path.join(ROOT_PATH / PATH_TO_REPLAY, filename), images, fps=fps)
    return str(os.path.join(ROOT_PATH / PATH_TO_REPLAY, filename))

def clear_em():
    num_imgs = len(os.listdir(ROOT_PATH / PATH_TO_DUMP))
    for i in range(num_imgs):
        file_name = "{}.png".format(i)
        file_path = os.path.join(ROOT_PATH / PATH_TO_DUMP, file_name)
        os.remove(file_path)

if __name__ == "__main__":
    make_gif(input("filename: "))
    print("Done.")
