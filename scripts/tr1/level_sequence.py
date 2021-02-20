from engine import TR1TrackId, TR1ItemId, Video, Cutscene, Level, TitleMenu

title_menu = TitleMenu(
    name="TITLE",
    titles={
        "en": "Tomb Raider",
        "de": "Tomb Raider",
    },
    track=TR1TrackId.MainTheme,
    secrets=0,
)

lara_home = Level(
    name="GYM",
    titles={
        "en": "Lara's home",
        "de": "Laras Haus",
    },
    track=TR1TrackId.Ambience1,
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
            "en": "Caves",
            "de": "Die Kavernen",
        },
        track=TR1TrackId.Ambience1,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
    ),
    Level(
        name="LEVEL2",
        titles={
            "en": "City of Vilcabamba",
            "de": "Die Stadt Vilcabamba",
        },
        track=TR1TrackId.Ambience1,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Puzzle1: "Gold Idol",
                TR1ItemId.Key1: "Silver Key",
            },
        },
    ),
    Level(
        name="LEVEL3A",
        titles={
            "en": "Lost Valley",
            "de": "Das Verlorene Tal",
        },
        track=TR1TrackId.Ambience1,
        secrets=5,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Puzzle1: "Machine Cog",
            },
        },
    ),
    Level(
        name="LEVEL3B",
        titles={
            "en": "Tomb of Qualopec",
            "de": "Das Grab von Qualopec",
        },
        track=TR1TrackId.Ambience1,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
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
            "en": "St. Francis' Folly",
            "de": "St. Francis' Folly",
        },
        track=TR1TrackId.Ambience3,
        secrets=4,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Key1: "Neptune Key",
                TR1ItemId.Key2: "Atlas Key",
                TR1ItemId.Key3: "Damocles Key",
                TR1ItemId.Key4: "Thor Key",
            },
        },
    ),
    Level(
        name="LEVEL5",
        titles={
            "en": "Colosseum",
            "de": "Das Kolosseum",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Key1: "Rusty Key",
            },
        },
    ),
    Level(
        name="LEVEL6",
        titles={
            "en": "Palace Midas",
            "de": "Der Palast des Midas",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Puzzle1: "Gold Bar",
            },
        },
    ),
    Level(
        name="LEVEL7A",
        titles={
            "en": "The Cistern",
            "de": "Die Zisterne",
        },
        track=TR1TrackId.Ambience2,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Silver Key",
                TR1ItemId.Key3: "Rusty Key",
            },
        },
    ),
    Cutscene(
        name="CUT2",
        track=TR1TrackId.Cutscene4,
        camera_pos_x=51962,
        camera_pos_z=53760,
        camera_rot=90.0,
        gun_swap=True,
    ),
    Level(
        name="LEVEL7B",
        titles={
            "en": "Tomb of Tihocan",
            "de": "Das Grab des Tihocan",
        },
        track=TR1TrackId.Ambience2,
        secrets=2,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Rusty Key",
                TR1ItemId.Key3: "Rusty Key",
            },
        },
    ),
    Video("VISION.RPL"),
    Level(
        name="LEVEL8A",
        titles={
            "en": "City of Khamoon",
            "de": "Die Stadt Khamoon",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Key1: "Saphire Key",
            },
        },
    ),
    Level(
        name="LEVEL8B",
        titles={
            "en": "Obelisk of Khamoon",
            "de": "Der Obelisk von Khamoon",
        },
        track=TR1TrackId.Ambience3,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Puzzle1: "Eye of Horus",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Puzzle3: "Seal of Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Saphire Key",
            },
        },
    ),
    Level(
        name="LEVEL8C",
        titles={
            "en": "Sanctuary of the Scion",
            "de": "Das Heiligtum des Scion",
        },
        track=TR1TrackId.Ambience3,
        secrets=1,
        inventory={TR1ItemId.Pistols: 1},
        item_titles={
            "en": {
                TR1ItemId.Puzzle1: "Ankh",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Key1: "Gold Key",
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
            "en": "Natla's Mines",
            "de": "Natlas Katakomben",
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
            "en": {
                TR1ItemId.Puzzle1: "Fuse",
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.Key1: "Gold Key",
            },
        },
    ),
    Cutscene(
        name="CUT4",
        track=TR1TrackId.Cutscene1,
        camera_rot=90.0,
        gun_swap=True,
    ),
    Video("PYRAMID.RPL"),
    Level(
        name="LEVEL10B",
        titles={
            "en": "Atlantis",
            "de": "Atlantis",
        },
        track=TR1TrackId.Ambience4,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1},
    ),
    Video("PRISON.RPL"),
    Level(
        name="LEVEL10C",
        titles={
            "en": "The Great Pyramid",
            "de": "Die Gro=e Pyramide",
        },
        track=TR1TrackId.Ambience4,
        secrets=3,
        inventory={TR1ItemId.Pistols: 1}
    ),
    Video("END.RPL"),
]
