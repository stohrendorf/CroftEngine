from ce.engine import TR1TrackId, TR1ItemId, Video, Level, TitleMenu, WeaponType, ModifyInventory, ResetSoundEngine, \
    SplashScreen

early_boot = [
    SplashScreen(
        path=f"data/EidosPC.png",
        duration_seconds=3,
        fade_in_duration_seconds=1,
        fade_out_duration_seconds=1,
    )
]

title_menu = TitleMenu(
    name="data/TITLE.PHD",
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
        name="data/LEVEL1.PHD",
        titles={
            "en_GB": "Night in Venice",
            "de_DE": "Nacht in Venedig",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Bar",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldbarren",
            },
        },
    ),
    Level(
        name="data/LEVEL2.PHD",
        titles={
            "en_GB": "The Lost Shipwreck",
            "de_DE": "Das verlorene Schiffswrack",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Idol",
                TR1ItemId.Key3: "Old Key",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldener G~otze",
                TR1ItemId.Key3: "Alter Schl~ussel",
            },
        },
    ),
    Level(
        name="data/LEVEL3A.PHD",
        titles={
            "en_GB": "Lara's Delirium",
            "de_DE": "Laras Delirium",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Sapphire Key",
                TR1ItemId.Puzzle1: "Ancient Amulet",
            },
            "de_DE": {
                TR1ItemId.Key1: "Saphirschl~ussel",
                TR1ItemId.Puzzle1: "Antikes Amulett",
            },
        },
    ),
    Level(
        name="data/LEVEL3B.PHD",
        titles={
            "en_GB": "Out of Tune",
            "de_DE": "Verstimmt",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Turbo Pascal Key",
                TR1ItemId.Puzzle1: "Machine Cog",
            },
            "de_DE": {
                TR1ItemId.Key1: "Turbo Pascal Schl~ussel",
                TR1ItemId.Puzzle1: "Zahnrad",
            },
        },
    ),
    Level(
        name="data/LEVEL4.PHD",
        titles={
            "en_GB": "Resonance",
            "de_DE": "Resonanz",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Neptune Key",
                TR1ItemId.Key2: "Atlas Key",
                TR1ItemId.Key3: "Damocles Key",
                TR1ItemId.Key4: "Thor Key",
            },
            "de_DE": {
                TR1ItemId.Key1: "Schl~ussel des Neptun",
                TR1ItemId.Key2: "Schl~ussel des Atlas",
                TR1ItemId.Key3: "Schl~ussel des Damokles",
                TR1ItemId.Key4: "Schl~ussel des Thor",
            },
        },
    ),
    Level(
        name="data/LEVEL5.PHD",
        titles={
            "en_GB": "The Forgotten Treasure",
            "de_DE": "Der vergessene Schatz",
        },
        ambient=TR1TrackId.Ambience4,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Dxtre3d Gem",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Dxtre3d Edelstein",
            },
        },
    ),
    Level(
        name="data/LEVEL6.PHD",
        titles={
            "en_GB": "Epilogue",
            "de_DE": "Epilog",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Bar",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldbarren",
            },
        },
    ),
    Video(paths=["FMV/Credits.mp4"]),
    ResetSoundEngine(),
]
