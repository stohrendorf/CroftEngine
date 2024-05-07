from ce.engine import TR1TrackId, TR1ItemId, Video, Cutscene, Level, TitleMenu, SplashScreen, WeaponType, \
    ModifyInventory, ResetSoundEngine, PlayAudioSlot


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
        "fr_FR": "Tomb Raider",
    },
    ambient=TR1TrackId.MainTheme,
)

lara_home = []
# lara_home = [
#     Video(paths=_fmv("MANSION")),
#     Level(
#         name="DATA/Manor.phd",
#         titles={
#             "en_GB": "Lara's home",
#             "de_DE": "Laras Haus",
#             "fr_FR": "La Demeure de Lara",
#         },
#         use_alternative_lara=True,
#         allow_save=False,
#         default_weapon=WeaponType.None_,
#         alternative_splashscreen="DATA/GYMLOAD.jpg",
#     ),
# ]

level_sequence = [
    Video(paths=_fmv("SNOW")),
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="data/Caves.phd",
        titles={
            "en_GB": "Caves",
            "de_DE": "Die Kavernen",
            "fr_FR": "Les Cavernes",
        },
        ambient=TR1TrackId.Ambience1,
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Level(
        name="data/The_City_of_Vilcabamba.phd",
        titles={
            "en_GB": "City of Vilcabamba",
            "de_DE": "Die Stadt Vilcabamba",
            "fr_FR": "La Cit)e de Vilcabamba",
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
            "fr_FR": {
                TR1ItemId.Puzzle1: "Id(ole d'or",
                TR1ItemId.Key1: "Cl)e d'argent",
            },
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Level(
        name="data/Lost_Valley.phd",
        titles={
            "en_GB": "Lost Valley",
            "de_DE": "Das Verlorene Tal",
            "fr_FR": "La Vall)ee Perdue",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Machine Cog",
                TR1ItemId.Key1: "Silver Key",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Zahnrad",
                TR1ItemId.Key1: "Silberner Schl~ussel",
            },
            "fr_FR": {
                TR1ItemId.Puzzle1: "Pi$ece de machine",
                TR1ItemId.Key1: "Cl)e d'argent",
            },
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Level(
        name="data/Tomb_of_Qualopec.phd",
        titles={
            "en_GB": "Tomb of Qualopec",
            "de_DE": "Das Grab von Qualopec",
            "fr_FR": "La Tombe de Qualopec",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Cutscene(
        name="DATA/CUT1.PHD",
        track=TR1TrackId.Cutscene2,
        camera_pos_x=36668,
        camera_pos_z=63180,
        camera_rot=-128.0,
    ),
    Level(
        name="data/Natla_Technologies.phd",
        titles={
            "en_GB": "Natla Technologies",
            "de_DE": "Natla Technologien",
            "fr_FR": "Natla Technologies",
        },
        ambient=TR1TrackId.Ambience3,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Elevator Key",
                TR1ItemId.Puzzle2: "Access Pass"
            },
            "de_DE": {
                TR1ItemId.Key1: "Aufzugsschlüssel",
                TR1ItemId.Puzzle2: "Zugangspass"
            },
            "fr_FR": {
                TR1ItemId.Key1: "Clé dascenseur",
                TR1ItemId.Puzzle2: "Laissez-passer daccès"
            }
        },
        alternative_splashscreen="DATA/AZTECLOA.jpg",
    ),
    Video(paths=_fmv("LIFT")),
    Level(
        name="data/St_Francis_Folly.phd",
        titles={
            "en_GB": "St. Francis' Folly",
            "de_DE": "St. Francis' Folly",
            "fr_FR": "Le Monument St Francis",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Hades Key",
                TR1ItemId.Key2: "Poseidon Key",
                TR1ItemId.Key3: "Athena Key",
                TR1ItemId.Key4: "Ares Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Hades-Schlüssel",
                TR1ItemId.Key2: "Poseidon-Schlüssel",
                TR1ItemId.Key3: "Athena-Schlüssel",
                TR1ItemId.Key4: "Ares-Schlüssel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Key1: "Clé d’Hadès",
                TR1ItemId.Key2: "Clé de Poséidon",
                TR1ItemId.Key3: "Clé d’Athéna",
                TR1ItemId.Key4: "Clé d’Arès",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/GREEKLOA.jpg",
    ),
    Level(
        name="data/Colosseum.phd",
        titles={
            "en_GB": "Colosseum",
            "de_DE": "Das Kolosseum",
            "fr_FR": "Le Colosseum",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Tartarus Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Tartarus-Schlüssel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Key1: "Clé du Tartare",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/GREEKLOA.jpg",
    ),
    Level(
        name="data\\The_Palace_of_Kings.phd",
        titles={
            "en_GB": "The Palace of the Kings",
            "de_DE": "Palce der Könige",
            "fr_FR": "Palais des Rois",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Gold Bar",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Goldbarren",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Puzzle1: "Lingot d'or",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/GREEKLOA.jpg",
    ),
    Level(
        name="data\\Tomb_of_Tihocan.phd",
        titles={
            "en_GB": "Tomb of Tihocan",
            "de_DE": "Grab von Tihocan",
            "fr_FR": "Tombeau de Tihocan",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Silver Key",
                TR1ItemId.Key3: "Rusty Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.Key2: "Silberner Schl~ussel",
                TR1ItemId.Key3: "Rostiger Schl~ussel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Key1: "Cl)e en or",
                TR1ItemId.Key2: "Cl)e d'argent",
                TR1ItemId.Key3: "Cl)e rouill)ee",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/GREEKLOA.jpg",
    ),
    Cutscene(
        name="DATA/CUT2.PHD",
        track=TR1TrackId.Cutscene4,
        camera_pos_x=51962,
        camera_pos_z=53760,
        camera_rot=90.0,
        weapon_swap=True,
    ),
    Video(paths=_fmv("VISION")),
    Level(
        name="data\\The_Lost_City.phd",
        titles={
            "en_GB": "The Lost City",
            "de_DE": "Die verlorene Stadt",
            "fr_FR": "La Cité Perdue",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Sapphire Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Saphir-Schl~ussel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Key1: "Cl)e de saphir",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/GREEKLOA.jpg",
    ),
    Level(
        name="data\\Obelisk_of_Khamoon.phd",
        titles={
            "en_GB": "Obelisk of Khamoon",
            "de_DE": "Der Obelisk von Khamoon",
            "fr_FR": "L'Ob)elisque de Khamoonn",
        },
        ambient=TR1TrackId.Ambience1,
        alternative_splashscreen="DATA/EGYPTLOA.jpg",
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Eye of Horus",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Puzzle3: "Seal of Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Sapphire Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Auge des Horus",
                TR1ItemId.Puzzle2: "Scarab~aus",
                TR1ItemId.Puzzle3: "Siegel des Anubis",
                TR1ItemId.Puzzle4: "Ankh-Kreuz",
                TR1ItemId.Key1: "Saphir-Schl~ussel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Puzzle1: "Oeil d'Horus",
                TR1ItemId.Puzzle2: "Scarab)ee",
                TR1ItemId.Puzzle3: "Sceau d'Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Cl)e de saphir",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
    ),
    Level(
        name="data\\Sanctuary_of_the_Scion.phd",
        titles={
            "en_GB": "Sanctuary of the Scion",
            "de_DE": "Das Heiligtum des Scion",
            "fr_FR": "Le Sanctuaire du Scion",
        },
        ambient=TR1TrackId.Ambience1,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Eye of Horus",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Puzzle3: "Seal of Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Saphire Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Auge des Horus",
                TR1ItemId.Puzzle2: "Scarab~aus",
                TR1ItemId.Puzzle3: "Siegel des Anubis",
                TR1ItemId.Puzzle4: "Ankh-Kreuz",
                TR1ItemId.Key1: "Saphir-Schl~ussel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Puzzle1: "Oeil d'Horus",
                TR1ItemId.Puzzle2: "Scarab)ee",
                TR1ItemId.Puzzle3: "Sceau d'Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Cl)e de saphir",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/EGYPTLOA.jpg",
        water_color=(0.7, 0.1, 0.7),
        water_density=0.3
    ),
    Video(paths=_fmv("CANYON")),
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
            TR1ItemId.ScionPieceCounter,
        },
    ),
    Level(
        name="data\\Natla's_Mines.phd",
        titles={
            "en_GB": "Natla's Mines",
            "de_DE": "Natlas Katakomben",
            "fr_FR": "Les Mines de Natla",
        },
        ambient=TR1TrackId.Ambience2,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Fuse",
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Sicherung",
                TR1ItemId.Puzzle2: "Schl~ussel der Pyramide",
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Puzzle1: "Fusible",
                TR1ItemId.Puzzle2: "Cl)e de la pyramide",
                TR1ItemId.Key1: "Cl)e en or",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        default_weapon=WeaponType.None_,
        alternative_splashscreen="DATA/EGYPTLOA.jpg",
    ),
    Cutscene(
        name="DATA/CUT3.PHD",
        track=TR1TrackId.Cutscene3,
        flip_rooms=True,
        camera_rot=90.0,
    ),
    Video(paths=_fmv("PYRAMID")),
    ModifyInventory(
        add_inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="data\\Atlantis.phd",
        titles={
            "en_GB": "Atlantis",
            "de_DE": "Atlantis",
            "fr_FR": "L'Atlantide",
        },
        ambient=TR1TrackId.Ambience4,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle2: "Schl~ussel der Pyramide",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Puzzle2: "Cl)e de la pyramide",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/ATLANLOA.jpg",
        water_color=(0.9, 0.1, 0.1),
        water_density=0.3,
    ),
    Video(paths=_fmv("PRISON")),
    Cutscene(
        name="DATA/CUT4.PHD",
        track=TR1TrackId.Cutscene1,
        camera_rot=90.0,
        weapon_swap=True,
    ),
    Level(
        name="data\\The_Great_Pyramid.phd",
        titles={
            "en_GB": "The Great Pyramid",
            "de_DE": "Die Große Pyramide",
            "fr_FR": "La Grande Pyramide",
        },
        ambient=TR1TrackId.Ambience4,
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle2: "Self-destruct Key",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle2: "Selbstzerstörungsschlüssel",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
            "fr_FR": {
                TR1ItemId.Puzzle2: "Clé d’autodestruction",
                TR1ItemId.ScionPieceCounter: "Scion",
            },
        },
        alternative_splashscreen="DATA/ATLANLOA.jpg",
        water_color=(0.9, 0.1, 0.1),
        water_density=0.3,
    ),
    Video(paths=_fmv("END")),
    ResetSoundEngine(),
    PlayAudioSlot(slot=0, track=TR1TrackId.MidasReprise),
    *(
        SplashScreen(
            path=f"DATA/{name}.png",
            duration_seconds=40 // 4 - 4,
            fade_in_duration_seconds=2,
            fade_out_duration_seconds=2,
        )
        for name in ("END", "CRED1", "CRED2", "CRED3")
    ),
    ResetSoundEngine(),
]
