from ce.engine import TR1TrackId, TR1ItemId, Video, Level, TitleMenu, SplashScreen, WeaponType, ModifyInventory, \
    ResetSoundEngine, PlayAudioSlot


def _fmv(basename: str):
    return [
        f"fmv/{basename}.mp4",
    ]


early_boot = [
    SplashScreen(
        path=f"pictures/intro.png",
        duration_seconds=1,
        fade_in_duration_seconds=1,
        fade_out_duration_seconds=1,
    )
]

title_menu = TitleMenu(
    name="data/Title.phd",
    titles={
        "en_GB": "Into the Realm of Eternal Darkness",
        "de_DE": "In das Reich der ewigen Finsternis",
    },
    ambient=TR1TrackId.MainTheme,
)

lara_home = []

level_sequence = [
    Video(paths=_fmv("FMV1")),
    Level(
        name="data/Manor.phd",
        titles={
            "en_GB": "Croft Manor",
            "de_DE": "Croft Herrenhaus",
        },
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Basement Key",
                TR1ItemId.Key2: "Closet Key",
                TR1ItemId.Key3: "Storage Room Key",
                TR1ItemId.Key4: "Tomb Key",
                TR1ItemId.Puzzle1: "Thistle Stone",
                TR1ItemId.Puzzle2: "Cogwheel",
                TR1ItemId.Puzzle3: "Gold Bar",
            },
            "de_DE": {
                TR1ItemId.Key1: "Kellerschl~ussel",
                TR1ItemId.Key2: "Schrankschl~ussel",
                TR1ItemId.Key3: "Lagerraumschl~ussel",
                TR1ItemId.Key4: "Grabschl~ussel",
                TR1ItemId.Puzzle1: "Distelstein",
                TR1ItemId.Puzzle2: "Zahnrad",
                TR1ItemId.Puzzle3: "Goldbarren",
            },
        },
        ambient=TR1TrackId.Ambience4,
        default_weapon=WeaponType.None_,
        water_color=(0.45, 1.0, 1.0),
    ),
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
        drop_inventory={
            TR1ItemId.Puzzle1,
            TR1ItemId.Puzzle2,
            TR1ItemId.Puzzle3,
            TR1ItemId.Puzzle4,
            TR1ItemId.Key1,
            TR1ItemId.Key2,
            TR1ItemId.Key3,
            TR1ItemId.Key4,
        },
    ),
    Level(
        name="data/Underground.phd",
        titles={
            "en_GB": "Manor Underground",
            "de_DE": "Herrenhaus Untergrund",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Fuse",
                TR1ItemId.Puzzle2: "Circuit Board",
                TR1ItemId.Key1: "Storeroom Key",
                TR1ItemId.Key2: "Rusty Key",
                TR1ItemId.Key4: "Detonator Key",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Sicherung",
                TR1ItemId.Puzzle2: "Platine",
                TR1ItemId.Key1: "Lagerraumschl~ussel",
                TR1ItemId.Key2: "Rostiger Schl~ussel",
                TR1ItemId.Key4: "Detonatorschl~ussel",
            },
        },
        water_color=(0.45, 1.0, 1.0),
    ),
    ModifyInventory(
        drop_inventory={
            TR1ItemId.Puzzle1,
            TR1ItemId.Puzzle2,
            TR1ItemId.Puzzle3,
            TR1ItemId.Puzzle4,
            TR1ItemId.Key1,
            TR1ItemId.Key2,
            TR1ItemId.Key3,
            TR1ItemId.Key4,
        },
    ),
    Video(paths=_fmv("FMV3")),
    Level(
        name="data/Fortress.phd",
        titles={
            "en_GB": "Dark Fortress",
            "de_DE": "Dunkle Festung",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Silver Key",
                TR1ItemId.Key3: "Dungeon Key",
                TR1ItemId.Key4: "Underground Key",
                TR1ItemId.Puzzle1: "Violet Gem",
                TR1ItemId.Puzzle2: "Bucket of Black Water",
                TR1ItemId.Puzzle3: "Knight's Diary",
            },
            "de_DE": {
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.Key2: "Silberner Schl~ussel",
                TR1ItemId.Key3: "Verliesschl~ussel",
                TR1ItemId.Key4: "Untergrundschl~ussel",
                TR1ItemId.Puzzle1: "Violettes Juwel",
                TR1ItemId.Puzzle2: "Eimer mit schwarzem Wasser",
                TR1ItemId.Puzzle3: "Tagebuch des Ritters",
            },
        },
        water_color=(0.4, 0.4, 0.4),
    ),
    ModifyInventory(
        drop_inventory={
            TR1ItemId.Puzzle1,
            TR1ItemId.Puzzle2,
            TR1ItemId.Puzzle3,
            TR1ItemId.Puzzle4,
            TR1ItemId.Key1,
            TR1ItemId.Key2,
            TR1ItemId.Key3,
            TR1ItemId.Key4,
        },
    ),
    Level(
        name="data/chambers.phd",
        titles={
            "en_GB": "Chambers of Torture",
            "de_DE": "Kammern der Qualen",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Bronze Key",
                TR1ItemId.Puzzle1: "Unholy Plaque",
                TR1ItemId.Puzzle2: "Unknown's Diary",
                TR1ItemId.Puzzle3: "North - 3, South - 1, West - 4, East - 2",
                TR1ItemId.Puzzle4: "Use your compass to reveal the plaque. Remember, it ALWAYS points towards North!",
            },
            "de_DE": {
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.Key2: "Bronzeschl~ussel",
                TR1ItemId.Puzzle1: "Unheilige Plakette",
                TR1ItemId.Puzzle2: "Tagebuch eines Unbekannten",
                TR1ItemId.Puzzle3: "Norden - 3, S~uden - 1, Westen - 4, Osten - 2",
                TR1ItemId.Puzzle4: "Kompass benutzen, um die Plakette aufzudecken. Er zeigt IMMER nach Norden!",
            },
        },
        water_color=(0.4, 0.4, 0.4),
    ),
    ModifyInventory(
        drop_inventory={
            TR1ItemId.Puzzle1,
            TR1ItemId.Puzzle2,
            TR1ItemId.Puzzle3,
            TR1ItemId.Puzzle4,
            TR1ItemId.Key1,
            TR1ItemId.Key2,
            TR1ItemId.Key3,
            TR1ItemId.Key4,
        },
    ),
    Level(
        name="data/Palace.phd",
        titles={
            "en_GB": "Black Palace",
            "de_DE": "Schwarzer Palast",
        },
        ambient=TR1TrackId.LaraTalk30,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Stone Key",
                TR1ItemId.Key2: "Labyrinth Key",
                TR1ItemId.Key3: "The Moon is born of black. Switch is to be used only once. Save the game.",
                TR1ItemId.Key4: "Rooftop key",
                TR1ItemId.Puzzle1: "Amethyst Bar",
                TR1ItemId.Puzzle2: "Survivor's Diary",
                TR1ItemId.Puzzle3: "Crowbar",
                TR1ItemId.Puzzle4: "Knife",
            },
            "de_DE": {
                TR1ItemId.Key1: "Steinschl~ussel",
                TR1ItemId.Key2: "Labyrinthschl~ussel",
                TR1ItemId.Key3: "Der Mond ist aus Schwarz geboren. Der Schalter darf nur einmal benutzt werden. Spiel sichern.",
                TR1ItemId.Key4: "Dachschl~ussel",
                TR1ItemId.Puzzle1: "Amethystbarren",
                TR1ItemId.Puzzle2: "Tagebuch des ~uberlebenden",
                TR1ItemId.Puzzle3: "Brechstange",
                TR1ItemId.Puzzle4: "Messer",
            },
        },
        water_color=(0.4, 0.4, 0.4),
    ),
    ModifyInventory(
        drop_inventory={
            TR1ItemId.Puzzle1,
            TR1ItemId.Puzzle2,
            TR1ItemId.Puzzle3,
            TR1ItemId.Puzzle4,
            TR1ItemId.Key1,
            TR1ItemId.Key2,
            TR1ItemId.Key3,
            TR1ItemId.Key4,
        },
    ),
    Level(
        name="data/Sanctuary.phd",
        titles={
            "en_GB": "Sanctuary of the Crystal",
            "de_DE": "Das Heiligtum des Kristalls",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Dark Crystal",
                TR1ItemId.Puzzle3: "Beware of him...",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Dunkler Kristall",
                TR1ItemId.Puzzle3: "H~ute dich vor ihm...",
            },
        },
        water_color=(0.4, 0.4, 0.4),
    ),
    Video(paths=_fmv("FMV2")),
    ResetSoundEngine(),
    PlayAudioSlot(slot=0, track=TR1TrackId.Cutscene2),
    *(
        SplashScreen(
            path=f"pictures/{name}.png",
            duration_seconds=time - 2,
            fade_in_duration_seconds=1,
            fade_out_duration_seconds=1,
        )
        for name, time in (
        ("end1", 15),
        ("end2", 8),
        ("end3", 8),
        ("end4", 10),
        ("end5", 10),
        ("end6", 10),
        ("end7", 10),
        ("end8", 10),
        ("end12", 10),
        ("end11", 10),
        ("end9", 15),
        ("end10", 10),
    )
    ),
    ResetSoundEngine(),
]
