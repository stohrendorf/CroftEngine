from ce.engine import TR1TrackId, TR1ItemId, Level, TitleMenu, ResetSoundEngine, ModifyInventory
from ce.loader.file.level import Game

early_boot = []

title_menu = TitleMenu(
    name="DATA/TITLE.PHD",
    titles={
        "en_GB": "Tomb Raider - Demo",
        "de_DE": "Tomb Raider - Demo",
    },
    ambient=TR1TrackId.MainTheme,
    game=Game.TR1,
)

lara_home = []

level_sequence = [
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="DATA/LEVEL2.PHD",
        titles={
            "en_GB": "City of Vilcabamba - Demo",
            "de_DE": "Die Stadt Vilcabamba - Demo",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Idol",
                TR1ItemId.Key1: "Silver Key",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldener G~otze",
                TR1ItemId.Key1: "Silberner Schl~ussel",
            },
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
        game=Game.TR1,
    ),
    ResetSoundEngine(),
]
