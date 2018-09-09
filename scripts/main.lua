object_infos = (require "tr1.object_infos").object_infos

levelInfos = {
    {
        -- 1
        baseName = "GYM",
        name = "Lara's home",
        secrets = 0,
        useAlternativeLara = true,
        inventory = {}
    },
    {
        video = "SNOW"
    },
    {
        -- 2
        baseName = "LEVEL1",
        name = "Caves",
        track = 57,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        -- 3
        baseName = "LEVEL2",
        name = "City of Vilcabamba",
        track = 57,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        -- 4
        baseName = "LEVEL3A",
        name = "Lost Valley",
        track = 57,
        secrets = 5,
        inventory = { Pistols = 1 }
    },
    {
        -- 5
        baseName = "LEVEL3B",
        name = "Tomb of Qualopec",
        track = 57,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        cutscene = "CUT1",
        track = 23,
        cameraPosX = 36668,
        cameraPosZ = 63180,
        cameraRot = -128
    },
    {
        video = "LIFT"
    },
    {
        -- 6
        baseName = "LEVEL4",
        name = "St. Francis' Folly",
        track = 59,
        secrets = 4,
        inventory = { Pistols = 1 }
    },
    {
        -- 7
        baseName = "LEVEL5",
        name = "Colosseum",
        track = 59,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        -- 8
        baseName = "LEVEL6",
        name = "Palace Midas",
        track = 59,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        -- 9
        baseName = "LEVEL7A",
        name = "The Cistern",
        track = 58,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        cutscene = "CUT2",
        track = 25,
        cameraPosX = 51962,
        cameraPosZ = 53760,
        cameraRot = 90
    },
    {
        -- 10
        baseName = "LEVEL7B",
        name = "Tomb of Tihocan",
        track = 58,
        secrets = 2,
        inventory = { Pistols = 1 }
    },
    {
        video = "VISION"
    },
    {
        -- 11
        baseName = "LEVEL8A",
        name = "City of Khamoon",
        track = 59,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        -- 12
        baseName = "LEVEL8B",
        name = "Obelisk of Khamoon",
        track = 59,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        -- 13
        baseName = "LEVEL8C",
        name = "Sanctuary of the Scion",
        track = 59,
        secrets = 1,
        inventory = { Pistols = 1 }
    },
    {
        cutscene = "CUT3",
        track = 24,
        flipRooms = true,
        cameraRot = 90
    },
    {
        video = "CANYON"
    },
    {
        -- 14
        baseName = "LEVEL10A",
        name = "Natla's Mines",
        track = 58,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        cutscene = "CUT4",
        track = 22,
        cameraRot = 90
    },
    {
        video = "PYRAMID"
    },
    {
        -- 15
        baseName = "LEVEL10B",
        name = "Atlantis",
        track = 60,
        secrets = 3,
        inventory = { Pistols = 1 }
    },
    {
        video = "PRISON"
    },
    {
        -- 16
        baseName = "LEVEL10C",
        name = "The Great Pyramid",
        track = 60,
        secrets = 3,
        inventory = { Pistols = 1 }
    }
}

cheats = {
    godMode = false,
    inventory = {
        Key1 = 10,
        Key2 = 10,
        Key3 = 10,
        Key4 = 10,
        Puzzle1 = 10,
        Puzzle2 = 10,
        Puzzle3 = 10,
        Puzzle4 = 10,
    }
}

function getLevelInfo()
    return levelInfos[7]
end

function getGlidosPack()
    return nil -- "assets/trx/1SilverlokAllVers/silverlok/silverlok.txt"
end

function getObjectInfo(id)
    return object_infos[id]
end

print("Yay! Main script loaded.")