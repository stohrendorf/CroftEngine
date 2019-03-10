-- init defaults
local infos = {}
for i = 0, 190 do
    infos[i] = ObjectInfo.new()
end

-- Lara
infos[0].hit_points = 1000

-- Lara Twin
infos[6].hit_points = 1000

-- Wolf
infos[7].ai_agent = true
infos[7].radius = 341
infos[7].hit_points = 6
infos[7].pivot_length = 375
infos[7].target_update_chance = 0x2000

-- Bear
infos[8].ai_agent = true
infos[8].radius = 341
infos[8].hit_points = 20
infos[8].pivot_length = 500
infos[8].target_update_chance = 0x4000

-- Bat
infos[9].ai_agent = true
infos[9].radius = 102
infos[9].hit_points = 1
infos[9].target_update_chance = 0x400

-- Crocodile on land
infos[10].ai_agent = true
infos[10].radius = 341
infos[10].hit_points = 20
infos[10].pivot_length = 600
infos[10].target_update_chance = 0x2000

-- Crocodile in water
infos[11].ai_agent = true
infos[11].radius = 341
infos[11].hit_points = 20
infos[11].pivot_length = 600
infos[11].target_update_chance = 0x400

-- Lion (male)
infos[12].ai_agent = true
infos[12].radius = 341
infos[12].hit_points = 30
infos[12].pivot_length = 400
infos[12].target_update_chance = 0x7fff

-- Lion (female)
infos[13].ai_agent = true
infos[13].radius = 341
infos[13].hit_points = 25
infos[13].pivot_length = 400
infos[13].target_update_chance = 0x2000

-- Panther
infos[14].ai_agent = true
infos[14].radius = 341
infos[14].hit_points = 45
infos[14].pivot_length = 400
infos[14].target_update_chance = 0x2000

-- Gorilla
infos[15].ai_agent = true
infos[15].radius = 341
infos[15].hit_points = 22
infos[15].pivot_length = 250
infos[15].target_update_chance = 0x7fff

-- TRex
infos[18].ai_agent = true
infos[18].radius = 341
infos[18].hit_points = 100
infos[18].pivot_length = 2000
infos[18].target_update_chance = 0x7fff

-- Raptor
infos[19].ai_agent = true
infos[19].radius = 341
infos[19].hit_points = 20
infos[19].pivot_length = 400
infos[19].target_update_chance = 0x4000

-- Mummy
infos[24].hit_points = 18

-- Larson
infos[27].ai_agent = true
infos[27].radius = 102
infos[27].hit_points = 50
infos[27].target_update_chance = 0x7fff

-- Pierre
infos[28].ai_agent = true
infos[28].radius = 102
infos[28].hit_points = 70
infos[28].target_update_chance = 0x7fff

return infos
