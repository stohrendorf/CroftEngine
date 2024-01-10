from ce.engine import Gameflow

from audio import tracks
from level_sequence import level_sequence, title_menu, lara_home, early_boot
from object_infos import object_infos

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
    } if False else {},  # lgtm [py/constant-conditional-expression]
    asset_root="tr1nordicadventure",
)
