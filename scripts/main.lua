object_infos = (require "tr1.object_infos")
tracks = (require "tr1.audio")
level_infos = (require "tr1.level_infos")

cheats = {
    godMode = true,
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
    return level_infos[10]
end

function getGlidosPack()
    return nil
    -- return "assets/trx/1SilverlokAllVers/silverlok/"
    -- return "assets/trx/JC levels 1-12/Textures/JC/"
    -- return "assets/trx/JC levels 13-15/Textures/JC/"
end

function getObjectInfo(id)
    return object_infos[id]
end

function getTrackInfo(id)
    return tracks[id]
end

print("Yay! Main script loaded.")
