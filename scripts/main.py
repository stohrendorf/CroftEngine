# language_override:  "en_US.utf8"
from engine import TR1ItemId

from scripts.tr1.object_infos import object_infos
from scripts.tr1.audio import tracks
from scripts.tr1.level_sequence import level_sequence, title_menu, lara_home

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
        }
    }
else:
    cheats = {}


def getGlidosPack():
    return None
    # return "assets/trx/1SilverlokAllVers/silverlok/"
    # return "assets/trx/JC levels 1-12/Textures/JC/"
    # return "assets/trx/JC levels 13-15/Textures/JC/"


def getObjectInfo(id):
    return object_infos[TR1ItemId(id)]


def getTrackInfo(id):
    return tracks[id]


print("Yay! Main script loaded.")
