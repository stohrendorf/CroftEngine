# locale_override = "en_GB.utf8"
from engine import TR1ItemId, Gameflow

from object_infos import object_infos
from audio import tracks
from level_sequence import level_sequence, title_menu, lara_home, early_boot

gameflow = Gameflow(
    object_infos=object_infos,
    tracks=tracks,
    level_sequence=level_sequence,
    title_menu=title_menu,
    title_menu_backdrop="DATA/TITLEH.PCX",
    lara_home=lara_home,
    early_boot=early_boot,
    cheats={
        "godMode": True,
        "allAmmoCheat": True,
        "inventory": {
            TR1ItemId.Key1: 10,
            TR1ItemId.Key2: 10,
            TR1ItemId.Key3: 10,
            TR1ItemId.Key4: 10,
            TR1ItemId.Puzzle1: 10,
            TR1ItemId.Puzzle2: 10,
            TR1ItemId.Puzzle3: 10,
            TR1ItemId.Puzzle4: 10,
            TR1ItemId.UzisSprite: 1,
            TR1ItemId.MagnumsSprite: 1,
            TR1ItemId.ShotgunSprite: 1,
        }
    } if False else {}  # lgtm [py/constant-conditional-expression]
)
