from engine import TR1TrackId, TR1ItemId, Video, Cutscene, Level, TitleMenu

title_menu = TitleMenu(
    name="TITLE",
    titles={
        "en_GB": "Tomb Raider",
        "de_DE": "Tomb Raider",
    },
    track=TR1TrackId.MainTheme,
)

lara_home = Level(
    name="GYM",
    titles={
        "en_GB": "Lara's home",
        "de_DE": "Laras Haus",
    },
    secrets=0,
    use_alternative_lara=True,
    allow_save=False,
)

level_sequence = [
    Video("CORE.RPL"),
    Video("ESCAPE.RPL"),
    Video("CAFE.RPL"),
    Video("SNOW.RPL"),
    Level(
        name="LEVEL1",
        titles={
            "en_GB": "Caves",
            "de_DE": "Die Kavernen",
        },
        track=TR1TrackId.Ambience1,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="LEVEL2",
        titles={
            "en_GB": "City of Vilcabamba",
            "de_DE": "Die Stadt Vilcabamba",
        },
        track=TR1TrackId.Ambience1,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
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
    ),
    Level(
        name="LEVEL3A",
        titles={
            "en_GB": "Lost Valley",
            "de_DE": "Das Verlorene Tal",
        },
        track=TR1TrackId.Ambience1,
        secrets=5,
        inventory={TR1ItemId.Pistols: 1},
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
        name="LEVEL3B",
        titles={
            "en_GB": "Tomb of Qualopec",
            "de_DE": "Das Grab von Qualopec",
        },
        track=TR1TrackId.Ambience1,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Cutscene(
        name="CUT1",
        track=TR1TrackId.Cutscene2,
        camera_pos_x=36668,
        camera_pos_z=63180,
        camera_rot=-128.0,
    ),
    Video("LIFT.RPL"),
    Level(
        name="LEVEL4",
        titles={
            "en_GB": "St. Francis' Folly",
            "de_DE": "St. Francis' Folly",
        },
        track=TR1TrackId.Ambience3,
        secrets=4,
        inventory={TR1ItemId.Pistols: 1},
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
        name="LEVEL5",
        titles={
            "en_GB": "Colosseum",
            "de_DE": "Das Kolosseum",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Rusty Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Rostiger Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="LEVEL6",
        titles={
            "en_GB": "Palace Midas",
            "de_DE": "Der Palast des Midas",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
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
        name="LEVEL7A",
        titles={
            "en_GB": "The Cistern",
            "de_DE": "Die Zisterne",
        },
        track=TR1TrackId.Ambience2,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
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
    Cutscene(
        name="CUT2",
        track=TR1TrackId.Cutscene4,
        camera_pos_x=51962,
        camera_pos_z=53760,
        camera_rot=90.0,
        weapon_swap=True,
    ),
    Level(
        name="LEVEL7B",
        titles={
            "en_GB": "Tomb of Tihocan",
            "de_DE": "Das Grab des Tihocan",
        },
        track=TR1TrackId.Ambience2,
        secrets=2,
        inventory={TR1ItemId.Pistols: 1},
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
    Video("VISION.RPL"),
    Level(
        name="LEVEL8A",
        titles={
            "en_GB": "City of Khamoon",
            "de_DE": "Die Stadt Khamoon",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.Key1: "Saphire Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Key1: "Saphir-Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Level(
        name="LEVEL8B",
        titles={
            "en_GB": "Obelisk of Khamoon",
            "de_DE": "Der Obelisk von Khamoon",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Eye of Horus",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Puzzle3: "Seal of Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Saphire Key",
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
        name="LEVEL8C",
        titles={
            "en_GB": "Sanctuary of the Scion",
            "de_DE": "Das Heiligtum des Scion",
        },
        track=TR1TrackId.Ambience3,
        secrets=1,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Ankh",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Ankh-kreuz",
                TR1ItemId.Puzzle2: "Scarab~aus",
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Cutscene(
        name="CUT3",
        track=TR1TrackId.Cutscene3,
        flip_rooms=True,
        camera_rot=90.0,
    ),
    Video("CANYON.RPL"),
    Level(
        name="LEVEL10A",
        titles={
            "en_GB": "Natla's Mines",
            "de_DE": "Natlas Katakomben",
        },
        track=TR1TrackId.Ambience2,
        secrets=3,
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
        item_titles={
            "en_GB": {
                TR1ItemId.Puzzle1: "Fuse",
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.ScionPiece5: "Scion",
            },
            "de_DE": {
                TR1ItemId.Puzzle1: "Sicherung",
                TR1ItemId.Puzzle2: "Schl~ussel der Pyramide",
                TR1ItemId.Key1: "Goldener Schl~ussel",
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Cutscene(
        name="CUT4",
        track=TR1TrackId.Cutscene1,
        camera_rot=90.0,
        weapon_swap=True,
    ),
    Video("PYRAMID.RPL"),
    Level(
        name="LEVEL10B",
        titles={
            "en_GB": "Atlantis",
            "de_DE": "Atlantis",
        },
        track=TR1TrackId.Ambience4,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Video("PRISON.RPL"),
    Level(
        name="LEVEL10C",
        titles={
            "en_GB": "The Great Pyramid",
            "de_DE": "Die Gro=e Pyramide",
        },
        track=TR1TrackId.Ambience4,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en_GB": {
                TR1ItemId.ScionPiece5: "Scion",
            },
        },
    ),
    Video("END.RPL"),
]
