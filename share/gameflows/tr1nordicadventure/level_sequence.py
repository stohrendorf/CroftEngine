from ce.engine import TR1TrackId, TR1ItemId, Video, Level, TitleMenu, ModifyInventory


def _fmv(basename: str):
    return [
        f"FMV/{basename}.AVI",
        f"FMV/{basename}.RPL",
    ]


early_boot = [
    Video(paths=_fmv("CORE")),
    Video(paths=_fmv("ESCAPE"), optional=True),
    Video(paths=_fmv("CAFE")),
]

title_menu = TitleMenu(
    name="DATA/TITLE.PHD",
    titles={
        "en_GB": "Tomb Raider",
        "de_DE": "Tomb Raider",
    },
    ambient=TR1TrackId.MainTheme,
)

lara_home = []

level_sequence = [
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="DATA/LEVEL1.PHD",
        titles={
            "en_GB": "Ruins",
            "de_DE": "Ruinen",
        },
        ambient=TR1TrackId.Ambience1,
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Level(
        name="DATA/LEVEL2.PHD",
        titles={
            "en_GB": "The Lost Ship",
            "de_DE": "Das verlorene Schiff",
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
    ),
    Level(
        name="DATA/LEVEL3A.PHD",
        titles={
            "en_GB": "Thor's Hall",
            "de_DE": "Thor's Saal",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Machine Cog",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Zahnrad",
            },
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Level(
        name="DATA/LEVEL3B.PHD",
        titles={
            "en_GB": "Way to Valhalla",
            "de_DE": "Weg nach Valhalla",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
]
