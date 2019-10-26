local level_infos = {
    [1] = {
        baseName = "GYM",
        name = {
            ["en"] = "Lara's home",
            ["de"] = "Laras Haus",
        },
        secrets = 0,
        useAlternativeLara = true,
        inventory = {}
    },
    [2] = {
        video = "SNOW.RPL"
    },
    [3] = {
        baseName = "LEVEL1",
        name = {
            ["en"] = "Caves",
            ["de"] = "Die Kavernen",
        },
        track = TR1TrackId.Ambience1,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [4] = {
        baseName = "LEVEL2",
        name = {
            ["en"] = "City of Vilcabamba",
            ["de"] = "Die Stadt Vilcabamba",
        },
        track = TR1TrackId.Ambience1,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [5] = {
        baseName = "LEVEL3A",
        name = {
            ["en"] = "Lost Valley",
            ["de"] = "Das Verlorene Tal",
        },
        track = TR1TrackId.Ambience1,
        secrets = 5,
        inventory = { Pistols = 1 }
    },
    [6] = {
        baseName = "LEVEL3B",
        name = {
            ["en"] = "Tomb of Qualopec",
            ["de"] = "Das Grab von Qualopec",
        },
        track = TR1TrackId.Ambience1,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [7] = {
        cutscene = "CUT1",
        track = TR1TrackId.Cutscene2,
        cameraPosX = 36668,
        cameraPosZ = 63180,
        cameraRot = -128
    },
    [8] = {
        video = "LIFT.RPL"
    },
    [9] = {
        baseName = "LEVEL4",
        name = {
            ["en"] = "St. Francis' Folly",
            ["de"] = "St. Francis' Folly",
        },
        track = TR1TrackId.Ambience3,
        secrets = 4,
        inventory = { Pistols = 1 }
    },
    [10] = {
        baseName = "LEVEL5",
        name = {
            ["en"] = "Colosseum",
            ["de"] = "Das Kolosseum",
        },
        track = TR1TrackId.Ambience3,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [11] = {
        baseName = "LEVEL6",
        name = {
            ["en"] = "Palace Midas",
            ["de"] = "Der Palast des Midas",
        },
        track = TR1TrackId.Ambience3,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [12] = {
        baseName = "LEVEL7A",
        name = {
            ["en"] = "The Cistern",
            ["de"] = "Die Zisterne",
        },
        track = TR1TrackId.Ambience2,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [13] = {
        cutscene = "CUT2",
        track = TR1TrackId.Cutscene4,
        cameraPosX = 51962,
        cameraPosZ = 53760,
        cameraRot = 90,
        gunSwap = true
    },
    [14] = {
        baseName = "LEVEL7B",
        name = {
            ["en"] = "Tomb of Tihocan",
            ["de"] = "Das Grab des Tihocan",
        },
        track = TR1TrackId.Ambience2,
        secrets = 2,
        inventory = { Pistols = 1 }
    },
    [15] = {
        video = "VISION.RPL"
    },
    [16] = {
        baseName = "LEVEL8A",
        name = {
            ["en"] = "City of Khamoon",
            ["de"] = "Die Stadt Khamoon",
        },
        track = TR1TrackId.Ambience3,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [17] = {
        baseName = "LEVEL8B",
        name = {
            ["en"] = "Obelisk of Khamoon",
            ["de"] = "Der Obelisk von Khamoon",
        },
        track = TR1TrackId.Ambience3,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [18] = {
        baseName = "LEVEL8C",
        name = {
            ["en"] = "Sanctuary of the Scion",
            ["de"] = "Das Heiligtum des Scion",
        },
        track = TR1TrackId.Ambience3,
        secrets = 1,
        inventory = { Pistols = 1 }
    },
    [19] = {
        cutscene = "CUT3",
        track = TR1TrackId.Cutscene3,
        flipRooms = true,
        cameraRot = 90
    },
    [20] = {
        video = "CANYON.RPL"
    },
    [21] = {
        baseName = "LEVEL10A",
        name = {
            ["en"] = "Natla's Mines",
            ["de"] = "Natlas Katakomben",
        },
        track = TR1TrackId.Ambience2,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [22] = {
        cutscene = "CUT4",
        track = TR1TrackId.Cutscene1,
        cameraRot = 90,
        gunSwap = true
    },
    [23] = {
        video = "PYRAMID.RPL"
    },
    [24] = {
        baseName = "LEVEL10B",
        name = {
            ["en"] = "Atlantis",
            ["de"] = "Atlantis",
        },
        track = TR1TrackId.Ambience4,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    [25] = {
        video = "PRISON.RPL"
    },
    [26] = {
        baseName = "LEVEL10C",
        name = {
            ["en"] = "The Great Pyramid",
            ["de"] = "Die Gro=e Pyramide",
        },
        track = TR1TrackId.Ambience4,
        secrets = 3,
        inventory = { Pistols = 1 }
    }
}

return level_infos
