from engine import TR1TrackId, TR1ItemId

level_sequence = (
    {
        "baseName": "TITLE",
        "name": {
            "en": "Tomb Raider",
            "de": "Tomb Raider",
        },
        "secrets": 0,
        "inventory": {}
    },
    {
        "baseName": "GYM",
        "name": {
            "en": "Lara's home",
            "de": "Laras Haus",
        },
        "secrets": 0,
        "useAlternativeLara": True,
        "inventory": {}
    },
    {
        "video": "SNOW.RPL"
    },
    {
        "baseName": "LEVEL1",
        "name": {
            "en": "Caves",
            "de": "Die Kavernen",
        },
        "track": TR1TrackId.Ambience1,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1}
    },
    {
        "baseName": "LEVEL2",
        "name": {
            "en": "City of Vilcabamba",
            "de": "Die Stadt Vilcabamba",
        },
        "track": TR1TrackId.Ambience1,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Puzzle1: "Gold Idol",
                TR1ItemId.Key1: "Silver Key",
            },
        }
    },
    {
        "baseName": "LEVEL3A",
        "name": {
            "en": "Lost Valley",
            "de": "Das Verlorene Tal",
        },
        "track": TR1TrackId.Ambience1,
        "secrets": 5,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Puzzle1: "Machine Cog",
            },
        }
    },
    {
        "baseName": "LEVEL3B",
        "name": {
            "en": "Tomb of Qualopec",
            "de": "Das Grab von Qualopec",
        },
        "track": TR1TrackId.Ambience1,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1}
    },
    {
        "cutscene": "CUT1",
        "track": TR1TrackId.Cutscene2,
        "cameraPosX": 36668,
        "cameraPosZ": 63180,
        "cameraRot": -128
    },
    {
        "video": "LIFT.RPL"
    },
    {
        "baseName": "LEVEL4",
        "name": {
            "en": "St. Francis' Folly",
            "de": "St. Francis' Folly",
        },
        "track": TR1TrackId.Ambience3,
        "secrets": 4,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Key1: "Neptune Key",
                TR1ItemId.Key2: "Atlas Key",
                TR1ItemId.Key3: "Damocles Key",
                TR1ItemId.Key4: "Thor Key",
            },
        }
    },
    {
        "baseName": "LEVEL5",
        "name": {
            "en": "Colosseum",
            "de": "Das Kolosseum",
        },
        "track": TR1TrackId.Ambience3,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Key1: "Rusty Key",
            },
        }
    },
    {
        "baseName": "LEVEL6",
        "name": {
            "en": "Palace Midas",
            "de": "Der Palast des Midas",
        },
        "track": TR1TrackId.Ambience3,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Puzzle1: "Gold Bar",
            },
        }
    },
    {
        "baseName": "LEVEL7A",
        "name": {
            "en": "The Cistern",
            "de": "Die Zisterne",
        },
        "track": TR1TrackId.Ambience2,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Silver Key",
                TR1ItemId.Key3: "Rusty Key",
            },
        }
    },
    {
        "cutscene": "CUT2",
        "track": TR1TrackId.Cutscene4,
        "cameraPosX": 51962,
        "cameraPosZ": 53760,
        "cameraRot": 90,
        "gunSwap": True
    },
    {
        "baseName": "LEVEL7B",
        "name": {
            "en": "Tomb of Tihocan",
            "de": "Das Grab des Tihocan",
        },
        "track": TR1TrackId.Ambience2,
        "secrets": 2,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Key1: "Gold Key",
                TR1ItemId.Key2: "Rusty Key",
                TR1ItemId.Key3: "Rusty Key",
            },
        }
    },
    {
        "video": "VISION.RPL"
    },
    {
        "baseName": "LEVEL8A",
        "name": {
            "en": "City of Khamoon",
            "de": "Die Stadt Khamoon",
        },
        "track": TR1TrackId.Ambience3,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Key1: "Saphire Key",
            },
        }
    },
    {
        "baseName": "LEVEL8B",
        "name": {
            "en": "Obelisk of Khamoon",
            "de": "Der Obelisk von Khamoon",
        },
        "track": TR1TrackId.Ambience3,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Puzzle1: "Eye of Horus",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Puzzle3: "Seal of Anubis",
                TR1ItemId.Puzzle4: "Ankh",
                TR1ItemId.Key1: "Saphire Key",
            },
        }
    },
    {
        "baseName": "LEVEL8C",
        "name": {
            "en": "Sanctuary of the Scion",
            "de": "Das Heiligtum des Scion",
        },
        "track": TR1TrackId.Ambience3,
        "secrets": 1,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Puzzle1: "Ankh",
                TR1ItemId.Puzzle2: "Scarab",
                TR1ItemId.Key1: "Gold Key",
            },
        }
    },
    {
        "cutscene": "CUT3",
        "track": TR1TrackId.Cutscene3,
        "flipRooms": True,
        "cameraRot": 90
    },
    {
        "video": "CANYON.RPL"
    },
    {
        "baseName": "LEVEL10A",
        "name": {
            "en": "Natla's Mines",
            "de": "Natlas Katakomben",
        },
        "track": TR1TrackId.Ambience2,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1},
        "names": {
            "en": {
                TR1ItemId.Puzzle1: "Fuse",
                TR1ItemId.Puzzle2: "Pyramid Key",
                TR1ItemId.Key1: "Gold Key",
            },
        }
    },
    {
        "cutscene": "CUT4",
        "track": TR1TrackId.Cutscene1,
        "cameraRot": 90,
        "gunSwap": True
    },
    {
        "video": "PYRAMID.RPL"
    },
    {
        "baseName": "LEVEL10B",
        "name": {
            "en": "Atlantis",
            "de": "Atlantis",
        },
        "track": TR1TrackId.Ambience4,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1}
    },
    {
        "video": "PRISON.RPL"
    },
    {
        "baseName": "LEVEL10C",
        "name": {
            "en": "The Great Pyramid",
            "de": "Die Gro=e Pyramide",
        },
        "track": TR1TrackId.Ambience4,
        "secrets": 3,
        "inventory": {TR1ItemId.Pistols: 1}
    }
)
