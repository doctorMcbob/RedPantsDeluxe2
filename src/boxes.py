from copy import deepcopy

HITBOXES = {}
HURTBOXES = {}

def swap_in(hitboxes=None, hurtboxes=None):
    global HITBOXES, HURTBOXES
    if hitboxes is not None:
        HITBOXES = deepcopy(hitboxes)
    if hurtboxes is not None:
        HURTBOXES = deepcopy(hurtboxes)

def load():
    from src.lib import BOXES as B

    for key in B.HITBOXES:
        HITBOXES[key] = B.HITBOXES[key]
        HURTBOXES[key] = B.HURTBOXES[key]
        
def get_hitbox_map(name):
    return HITBOXES[name] if name in HITBOXES else None

def get_hurtbox_map(name):
    return HURTBOXES[name] if name in HURTBOXES else None

