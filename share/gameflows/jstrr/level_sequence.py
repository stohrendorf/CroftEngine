from ce.engine import TR1TrackId, TR1ItemId, Video, Cutscene, Level, TitleMenu, SplashScreen, WeaponType, \
    ModifyInventory, ResetSoundEngine, PlayAudioSlot


def _fmv(basename: str):
    return [
        f"FMV/{basename}.AVI",
        f"FMV/{basename}.RPL",
    ]


early_boot = [
]

title_menu = TitleMenu(
    name="DATA/TITLE.PHD",
    titles={
        "en_GB": "Tomb Raider",
        "de_DE": "Tomb Raider",
    },
    ambient=TR1TrackId.MainTheme,
)

lara_home = [
]

level_sequence = [
    Video(paths=_fmv("SNOW")),
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="DATA/Caves.PHD",
        titles={
            "en_GB": "Caves",
            "de_DE": "Die Kavernen",
        },
        ambient=TR1TrackId.Ambience1,
    ),
    Level(
        name="DATA/The_City_of_Vilcabamba.phd",
        titles={
            "en_GB": "City of Vilcabamba",
            "de_DE": "Die Stadt Vilcabamba",
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
    ResetSoundEngine(),
    PlayAudioSlot(slot=0, track=TR1TrackId.MidasReprise),
    *(
        SplashScreen(
            path=f"DATA/{name}.PCX",
            duration_seconds=40 // 4 - 4,
            fade_in_duration_seconds=2,
            fade_out_duration_seconds=2,
        )
        for name in ("CRED0", "CRED1", "CRED2", "CRED3")
    ),
    ResetSoundEngine(),
]
