# locale_override = "en_GB.utf8"
from engine import TR1ItemId

from scripts.tr1.object_infos import object_infos
from scripts.tr1.audio import tracks
from scripts.tr1.level_sequence import level_sequence, title_menu, lara_home, early_boot

if False:
    cheats = {
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
    }
else:
    cheats = {}

print("Yay! Main script loaded.")
