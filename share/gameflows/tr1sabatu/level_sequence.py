from engine import TR1TrackId, TR1ItemId, Video, Level, TitleMenu, SplashScreen, WeaponType, ModifyInventory, \
    PlayAudioSlot, ResetSoundEngine


def _fmv(basename: str):
    return [
        f"FMV/{basename}.AVI",
        f"FMV/{basename}.RPL",
    ]


early_boot = [
    Video(_fmv("CAFE")),
    SplashScreen(
        path=f"DATA/eidospc.png",
        duration_seconds=3,
        fade_in_duration_seconds=1,
        fade_out_duration_seconds=1,
    )
]

title_menu = TitleMenu(
    name="DATA/TITLE.PHD",
    titles={
        "en_GB": "Sabatu's Tomb Raider",
        "de_DE": "Sabatu's Tomb Raider",
    },
    ambient=TR1TrackId.MainTheme,
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
        item_titles={
            "en_GB": {
                TR1ItemId.Key3: "Manor Key",
            },
            "de_DE": {
                TR1ItemId.Key3: "Landsitz Schl~ussel",
            },
        },
    ),
]

level_sequence = [
    Video(_fmv("SNOW")),
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="DATA/LEVEL1.PHD",
        titles={
            "en_GB": "Caves",
            "de_DE": "Die Kavernen",
        },
        ambient=TR1TrackId.Ambience1,
    ),
    Level(
        name="DATA/LEVEL2.PHD",
        titles={
            "en_GB": "City of Vilcabamba",
            "de_DE": "Die Stadt Vilcabamba",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Idol",
                TR1ItemId.Key1: "Skull Key",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldener G~otze",
                TR1ItemId.Key1: "Sch~adelschl~ussel",
            },
        },
    ),
    Level(
        name="DATA/LEVEL3A.PHD",
        titles={
            "en_GB": "Lost Valley",
            "de_DE": "Das Verlorene Tal",
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
    ),
    Level(
        name="DATA/LEVEL3B.PHD",
        titles={
            "en_GB": "Tomb of Qualopec",
            "de_DE": "Das Grab von Qualopec",
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
    ),
    Video(_fmv("LIFT")),
    Level(
        name="DATA/LEVEL4.PHD",
        titles={
            "en_GB": "St. Francis' Folly",
            "de_DE": "St. Francis' Folly",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Neptune Key",
                TR1ItemId.Key2: "Atlas Key",
                TR1ItemId.Key3: "Damocles Key",
                TR1ItemId.Key4: "Thor Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Schl~ussel des Neptun",
                TR1ItemId.Key2: "Schl~ussel des Atlas",
                TR1ItemId.Key3: "Schl~ussel des Damokles",
                TR1ItemId.Key4: "Schl~ussel des Thor",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="DATA/LEVEL5.PHD",
        titles={
            "en_GB": "Colosseum",
            "de_DE": "Das Kolosseum",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Flow Machine Key",
                TR1ItemId.Puzzle1: "Machine Cog",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Str~omungsmaschine Schl~ussel",
                TR1ItemId.Puzzle1: "Zahnrad",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="DATA/LEVEL6.PHD",
        titles={
            "en_GB": "Palace Midas",
            "de_DE": "Der Palast des Midas",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Bar",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldbarren",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="DATA/LEVEL7A.PHD",
        titles={
            "en_GB": "The Cistern",
            "de_DE": "Die Zisterne",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Silver Key",
                TR1ItemId.Key3: "Rusty Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.Key2: "Silberner Schl~ussel",
                TR1ItemId.Key3: "Rostiger Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="DATA/LEVEL7B.PHD",
        titles={
            "en_GB": "Tomb of Tihocan",
            "de_DE": "Das Grab des Tihocan",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Rusty Key",
                TR1ItemId.Key3: "Rusty Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.Key2: "Rostiger Schl~ussel",
                TR1ItemId.Key3: "Rostiger Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Video(_fmv("VISION")),
    Level(
        name="DATA/LEVEL8A.PHD",
        titles={
            "en_GB": "City of Khamoon",
            "de_DE": "Die Stadt Khamoon",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Sapphire Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Saphir-Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="DATA/LEVEL8B.PHD",
        titles={
            "en_GB": "Obelisk of Khamoon",
            "de_DE": "Der Obelisk von Khamoon",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Eye of Horus",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Puzzle3: "Seal of Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Sapphire Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Auge des Horus",
                TR1ItemId.Puzzle2: "Scarab~aus",
                TR1ItemId.Puzzle3: "Siegel des Anubis",
                TR1ItemId.Puzzle4: "Ankh-Kreuz",
                TR1ItemId.Key1: "Saphir-Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="DATA/LEVEL8C.PHD",
        titles={
            "en_GB": "Sanctuary of the Scion",
            "de_DE": "Das Heiligtum des Scion",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Ankh",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Ankh-Kreuz",
                TR1ItemId.Puzzle2: "Scarab~aus",
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Video(_fmv("CANYON")),
    ModifyInventory(
        drop_inventory={
            TR1ItemId.Pistols,
            TR1ItemId.Magnums,
            TR1ItemId.Uzis,
            TR1ItemId.Shotgun,
            TR1ItemId.ScionPiece1,
            TR1ItemId.ScionPiece2,
            TR1ItemId.ScionPiece3,
            TR1ItemId.ScionPiece4,
            TR1ItemId.ScionPiece5,
        },
    ),
    Level(
        name="DATA/LEVEL10A.PHD",
        titles={
            "en_GB": "Natla's Mines",
            "de_DE": "Natlas Katakomben",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Fuse",
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.Key1: "Rusty Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Sicherung",
                TR1ItemId.Puzzle2: "Schl~ussel der Pyramide",
                TR1ItemId.Key1: "Rostiger Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
        default_weapon=WeaponType.None_,
    ),
    Video(_fmv("PYRAMID")),
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="DATA/LEVEL10B.PHD",
        titles={
            "en_GB": "Atlantis",
            "de_DE": "Atlantis",
        },
        ambient=TR1TrackId.Ambience4,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle2: "Schl~ussel der Pyramide",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
        water_color=(0.9, 0.1, 0.1),
        water_density=0.3,
    ),
    Video(_fmv("PRISON")),
    Level(
        name="DATA/LEVEL10C.PHD",
        titles={
            "en_GB": "The Great Pyramid",
            "de_DE": "Die Gro=e Pyramide",
        },
        ambient=TR1TrackId.Ambience4,
        item_titles={
            "en_GB": {
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
        water_color=(0.9, 0.1, 0.1),
        water_density=0.3,
    ),
    Video(_fmv("END")),
    ResetSoundEngine(),
    PlayAudioSlot(slot=0, track=TR1TrackId.LaraTalk24),
    *(
        SplashScreen(
            path=f"DATA/{name}.PCX",
            duration_seconds=50 // 4 - 4,
            fade_in_duration_seconds=2,
            fade_out_duration_seconds=2,
        )
        for name in ("END", "CRED1", "CRED2", "CRED3")
    ),
    ResetSoundEngine(),
]
