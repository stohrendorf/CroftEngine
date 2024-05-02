from ce.engine import TR1TrackId, Level, TitleMenu, WeaponType, TR1ItemId, SplashScreen


def _fmv(basename: str):
    return [
        f"FMV/{basename}.AVI",
        f"FMV/{basename}.RPL",
    ]


early_boot = []

title_menu = TitleMenu(
    name="Data/TITLE.PHD",
    titles={
        "en_GB": "Croft Arena",
        "de_DE": "Croft Arena",
    },
    ambient=TR1TrackId.MainTheme,
)

lara_home = []

level_sequence = [
    Level(
        name="Data/Fall_Raider_Easy.phd",
        titles={
            "en_GB": "Fall Raider Easy",
            "de_DE": "Fall Raider einfach",
        },
        ambient=TR1TrackId.Ambience4,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Fall_Raider_Medium.phd",
        titles={
            "en_GB": "Fall Raider Medium",
            "de_DE": "Fall Raider mittel",
        },
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Silver Key",
                TR1ItemId.Puzzle1: "Gold Idol",
            },
            "de_DE": {
                TR1ItemId.Key1: "Silberner Schl~ussel",
                TR1ItemId.Puzzle1: "Goldener G~otze",
            },
        },
        ambient=TR1TrackId.Ambience4,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Fall_Lara_H.phd",
        titles={
            "en_GB": "Fall Raider Hard",
            "de_DE": "Fall Raider schwer",
        },
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Machine Cog",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Zahnrad",
            },
        },
        ambient=TR1TrackId.Ambience4,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Hide_and_Seek_Manor.phd",
        titles={
            "en_GB": "Hide and Seek Manor",
            "de_DE": "Versteckspiel Herrenhaus",
        },
        ambient=TR1TrackId.LaraTalk28,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Hide_and_Seek_Peru.phd",
        titles={
            "en_GB": "Hide and Seek Peru",
            "de_DE": "Versteckspiel Peru",
        },
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
        ambient=TR1TrackId.Ambience3,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Hide_and_Seek_Egypt.phd",
        titles={
            "en_GB": "Hide and Seek Egypt",
            "de_DE": "Versteckspiel ~Agypten",
        },
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Rusty Key",
            },
            "de_DE": {
                TR1ItemId.Key1: "Rostiger Schl~ussel",
            },
        },
        ambient=TR1TrackId.Ambience3,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Volcanic_Race.phd",
        titles={
            "en_GB": "Volcanic Race",
            "de_DE": "Vulkanisches Rennen",
        },
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Bar",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldbarren",
            },
        },
        ambient=TR1TrackId.Ambience3,
        default_weapon=WeaponType.None_,
    ),
    Level(
        name="Data/Tour_Race_1.phd",
        titles={
            "en_GB": "Classic Tour Race",
            "de_DE": "Klassisches Tour-Rennen",
        },
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Silver Key",
                TR1ItemId.Key3: "Rusty Key",
            },
            "de_DE": {
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.Key2: "Silberner Schl~ussel",
                TR1ItemId.Key3: "Rostiger Schl~ussel",
            },
        },
        ambient=TR1TrackId.Ambience2,
        default_weapon=WeaponType.None_,
    ),
    SplashScreen(
        path=f"data/CRED0.PNG",
        duration_seconds=5,
        fade_in_duration_seconds=2,
        fade_out_duration_seconds=2,
    )
]
