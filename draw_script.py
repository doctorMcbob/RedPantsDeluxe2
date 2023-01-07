import os, sys
from pathlib import Path
import pygame

ROOT_PATH = Path('.')
PATH_TO_IMGS = ROOT_PATH / 'world_images'

if not os.path.isdir(PATH_TO_IMGS): os.mkdir(PATH_TO_IMGS)

pygame.init()

HEL16 = pygame.font.SysFont("Helvetica", 16)
screen = pygame.display.set_mode((640, 480))
LOG = []
def showlog(surf, pos):
    screen.fill((255, 255, 255))
    x, y = pos
    for i, log in enumerate(LOG):
        surf.blit(
            HEL16.render(log, 0, (0, 0, 0)),
            (x, y - i*16)
        )
    pygame.display.update()

def log(message):
    LOG.append(message)
    showlog(screen, (0, 16*len(LOG)))

try:
    log("Loading")

    from src import sprites
    from src import worlds
    from src import actor
    from src import frames
    from src import scripts
    from src import boxes
    
    scripts.load()
    sprites.load()
    boxes.load()
    worlds.load()
    actor.load()
    

    for world in worlds.get_worlds():
        log(f"Beginning room {world.name}")
        world.update()
        TOP = None
        BOT = None
        LEF = None
        RIG = None
        for name in world.actors:
            a = actor.get_actor(name)
            TOP = min( a.top, TOP ) if TOP is not None else a.top
            BOT = max( a.bottom, BOT ) if BOT is not None else a.bottom
            LEF = min( a.left, LEF ) if LEF is not None else a.left
            RIG = max( a.right, RIG) if RIG is not None else a.right

        W, H = abs(LEF - RIG), abs(TOP - BOT)

        log(f"Making frame W,H {W},{H} at {TOP},{LEF}")
        
        frame = frames.add_frame(world.name, world.name, (W, H), position=(LEF,TOP))

        log(f"Drawing...")

        image = frame.drawn()

        log(f"Writing to {PATH_TO_IMGS}/{world.name}.png")

        frames.delete_frame(world.name)
        
        pygame.image.save(image, str(PATH_TO_IMGS / f"{world.name}.png"))
        
except Exception as e:
    print(e)

for log in LOG:
    print(log)
    
sys.exit()

