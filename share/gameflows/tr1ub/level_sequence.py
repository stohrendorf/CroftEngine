from engine import TR1TrackId, TR1ItemId, Video, Level, TitleMenu, SplashScreen, WeaponType, ModifyInventory, \
    ResetSoundEngine


def _fmv(basename: str):
    return [
        f"FMV/{basename}.AVI",
        f"FMV/{basename}.RPL",
    ]


early_boot = [
    Video(_fmv("CORE")),
    Video(_fmv("ESCAPE")),
]

title_menu = TitleMenu(
    name="DATA/TITLE.PHD",
    titles={
        "en_GB": "Tomb Raider - Unfinished Business",
        "de_DE": "Tomb Raider - Unfinished Business",
    },
    track=TR1TrackId.MainTheme,
)

lara_home = [
    Video(_fmv("MANSION")),
    Level(
        name="DATA/GYM.PHD",
        titles={
            "en_GB": "Lara's home",
            "de_DE": "Laras Haus",
        },
        use_alternative_lara=True,
        allow_save=False,
        default_weapon=WeaponType.None_,
    ),
]

level_sequence = [
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="DATA/EGYPT.PHD",
        titles={
            "en_GB": "Return to Egypt",
            "de_DE": "R~uckkehr nach ~Agypten",
        },
        track=TR1TrackId.Ambience0,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Saphire Key",
            },
            "de_DE": {
                TR1ItemId.Key1: "Saphir-Schl~ussel",
            },
        },
    ),
    Level(
        name="DATA/CAT.PHD",
        titles={
            "en_GB": "Temple of the Cat",
            "de_DE": "Tempel der Katze",
        },
        track=TR1TrackId.Ambience0,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Ornate Key",
            },
            "de_DE": {
                TR1ItemId.Key1: "Verzierter Schl~ussel",
            },
        },
    ),
    Level(
        name="DATA/END.PHD",
        titles={
            "en_GB": "Atlantean Stronghold",
            "de_DE": "Atlantische Festung",
        },
        track=TR1TrackId.Ambience2,
    ),
    Level(
        name="DATA/END2.PHD",
        titles={
            "en_GB": "The Hive",
            "de_DE": "Der Bienenstock",
        },
        track=TR1TrackId.Ambience2,
    ),
    ResetSoundEngine(),
    *(
        SplashScreen(
            path=f"DATA/{name}.PCX",
            duration_seconds=11,
            fade_in_duration_seconds=2,
            fade_out_duration_seconds=2,
        )
        for name in ("END",)
    ),
]
