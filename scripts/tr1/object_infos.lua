-- init defaults
local infos = {}
for i = 0, 190 do
    infos[i] = {
        ai_agent = false,
        radius = 10,
        hit_points = -16384,
        pivot_length = 0,
        shadow_size = 0,
        target_update_chance = 0,
        initialise = nil,
        collision = nil,
        control = nil,
        ceiling = nil,
        floor = nil,
        render = nil
    }
end

local function baddie_init(item)
    item:set_y_angle((math.random(0, 32767) - 16384) * 2)
    item:set_collidable(true)
end

local function baddie_interact(item, lara, coll_info)
    if not item:is_near(lara, coll_info.radius) then
        return
    end
    if not item:test_bone_collision(lara) then
        return
    end
    if not coll_info:get_policy_flags().enable_baddie_push then
        return
    end

    local enable_spaz = (item.health > 0) and coll_info:get_policy_flags().enable_spaz
    item:do_enemy_push(lara, coll_info, enable_spaz, false);
end

local function sector_of(x)
    return math.floor(x / 1024)
end

local function sector_local(x)
    return x % 1024
end

-- Lara
infos[0].hit_points = 1000
infos[0].shadow_size = 160

-- Lara Twin
infos[6].hit_points = 1000
infos[6].shadow_size = 160

-- Wolf
infos[7].ai_agent = true
infos[7].radius = 341
infos[7].hit_points = 6
infos[7].pivot_length = 375
infos[7].shadow_size = 128
infos[7].target_update_chance = 0x2000
infos[7].initialise = function(item)
    item.frame_number = 96
    baddie_init(item)
end
infos[7].collison = baddie_interact

-- Bear
infos[8].ai_agent = true
infos[8].radius = 341
infos[8].hit_points = 20
infos[8].pivot_length = 500
infos[8].shadow_size = 128
infos[8].target_update_chance = 0x4000
infos[8].initialise = baddie_init
infos[8].collison = baddie_interact

-- Bat
infos[9].ai_agent = true
infos[9].radius = 102
infos[9].hit_points = 1
infos[9].shadow_size = 128
infos[9].target_update_chance = 0x400
infos[9].initialise = baddie_init
infos[9].collison = baddie_interact

-- Crocodile on land
infos[10].ai_agent = true
infos[10].radius = 341
infos[10].hit_points = 20
infos[10].pivot_length = 600
infos[10].shadow_size = 85
infos[10].target_update_chance = 0x2000
infos[10].initialise = baddie_init
infos[10].collison = baddie_interact

-- Crocodile in water
infos[11].ai_agent = true
infos[11].radius = 341
infos[11].hit_points = 20
infos[11].pivot_length = 600
infos[11].shadow_size = 85
infos[11].target_update_chance = 0x400
infos[11].initialise = baddie_init
infos[11].collison = baddie_interact

-- Lion (male)
infos[12].ai_agent = true
infos[12].radius = 341
infos[12].hit_points = 30
infos[12].pivot_length = 400
infos[12].shadow_size = 128
infos[12].target_update_chance = 0x7fff
infos[12].initialise = baddie_init
infos[12].collison = baddie_interact

-- Lion (female)
infos[13].ai_agent = true
infos[13].radius = 341
infos[13].hit_points = 25
infos[13].pivot_length = 400
infos[13].shadow_size = 128
infos[13].target_update_chance = 0x2000
infos[13].initialise = baddie_init
infos[13].collison = baddie_interact

-- Panther
infos[14].ai_agent = true
infos[14].radius = 341
infos[14].hit_points = 45
infos[14].pivot_length = 400
infos[14].shadow_size = 128
infos[14].target_update_chance = 0x2000
infos[14].initialise = baddie_init
infos[14].collison = baddie_interact

-- Gorilla
infos[15].ai_agent = true
infos[15].radius = 341
infos[15].hit_points = 22
infos[15].pivot_length = 250
infos[15].shadow_size = 128
infos[15].target_update_chance = 0x7fff
infos[15].initialise = baddie_init
infos[15].collison = baddie_interact

-- TRex
infos[18].ai_agent = true
infos[18].radius = 341
infos[18].hit_points = 100
infos[18].pivot_length = 2000
infos[18].shadow_size = 64
infos[18].target_update_chance = 0x7fff
infos[18].initialise = baddie_init
infos[18].collison = baddie_interact

-- Raptor
infos[19].ai_agent = true
infos[19].radius = 341
infos[19].hit_points = 20
infos[19].pivot_length = 400
infos[19].shadow_size = 128
infos[19].target_update_chance = 0x4000
infos[19].initialise = baddie_init
infos[19].collison = baddie_interact

-- Mummy
infos[24].hit_points = 18

-- Larson
infos[27].ai_agent = true
infos[27].radius = 102
infos[27].hit_points = 50
infos[27].shadow_size = 128
infos[27].target_update_chance = 0x7fff
infos[27].initialise = baddie_init
infos[27].collison = baddie_interact

-- Broken floor
infos[35].floor = function(item, _, y, _, base)
    local tmp = item.position.y - 512
    if y <= tmp then
        local state = item.current_anim_state
        if state == 0 or state == 1 then
            return tmp
        end
    end

    return base
end
infos[35].ceiling = function(item, _, y, _, base)
    local tmp = item.position.y - 512
    if y > tmp then
        local state = item.current_anim_state
        if state == 0 or state == 1 then
            return tmp + 256
        end
    end

    return base
end

-- Swinging blade
infos[36].shadow_size = 128

-- Dart
infos[39].shadow_size = 128

local function possibly_on_trapdoor_1(item, z, x)
    local sector_z = sector_of(z)
    local sector_x = sector_of(x)
    local item_sector_z = sector_of(item.position.z)
    local item_sector_x = sector_of(item.position.x)
    if item.rotation.y == 0 then
        return sector_x == item_sector_x and (sector_z == item_sector_z - 1 or sector_z == item_sector_z - 2)
    elseif item.rotation.y == -0x8000 then
        return sector_x == item_sector_x and (sector_z == item_sector_z + 1 or sector_z == item_sector_z + 2)
    elseif item.rotation.y == 0x4000 then
        return sector_z == item_sector_z and (sector_x == item_sector_x - 1 or sector_x == item_sector_x - 2)
    elseif item.rotation.y == -0x4000 then
        return sector_z == item_sector_z and (sector_x == item_sector_x + 1 or sector_x == item_sector_x + 2)
    else
        return false
    end
end

local function possibly_on_trapdoor_2(item, z, x)
    local sector_z = sector_of(z)
    local sector_x = sector_of(x)
    local item_sector_z = sector_of(item.position.z)
    local item_sector_x = sector_of(item.position.x)
    if item.rotation.y == 0 then
        return sector_x == item_sector_x and (sector_z == item_sector_z - 1)
    elseif item.rotation.y == -0x8000 then
        return sector_x == item_sector_x and (sector_z == item_sector_z + 1)
    elseif item.rotation.y == 0x4000 then
        return sector_z == item_sector_z and (sector_x == item_sector_x - 1)
    elseif item.rotation.y == -0x4000 then
        return sector_z == item_sector_z and (sector_x == item_sector_x + 1)
    else
        return false
    end
end

-- Trapdoor (open upwards)
infos[41].floor = function(item, x, y, z, base)
    if item.current_anim_state == 1 and possibly_on_trapdoor_1(item, z, x) and y <= item.position.y then
        return item.position.y
    end
    return base
end
infos[41].ceiling = function(item, x, y, z, base)
    if item.current_anim_state == 1 and possibly_on_trapdoor_1(item, z, x) and y > item.position.y then
        return item.position.y + 256
    end
    return base
end


-- Cubical blocks
infos[48].initialise = function(item)
    if item.activation_state ~= ActivationState.LOCKED then
        item:patch_heights(-1024)
    end
end
infos[49] = infos[48]
infos[50] = infos[48]
infos[51] = infos[48]

-- Doors
infos[57].initialise = function(item)
    -- TODO
end
infos[58] = infos[57]
infos[59] = infos[57]
infos[60] = infos[57]
infos[61] = infos[57]
infos[62] = infos[57]
infos[63] = infos[57]
infos[64] = infos[57]

-- Trapdoors
infos[65].floor = function(item, x, y, z, base)
    local tmp = item.position.y
    if possibly_on_trapdoor_2(item, z, x) and y <= tmp and item.current_anim_state == 0 and base > tmp then
        return tmp
    end
    return base
end
infos[65].ceiling = function(item, x, y, z, base)
    local tmp = item.position.y
    if possibly_on_trapdoor_2(item, z, x) and y > tmp and item.current_anim_state == 0 and base < tmp then
        return tmp + 256
    end
    return base
end
infos[66] = infos[65]

-- Flat bridge
infos[68].floor = function(item, _, y, _, base)
    local tmp = item.position.y
    if y <= tmp then
        return y
    end
    return base
end
infos[68].ceiling = function(item, _, y, _, base)
    local tmp = item.position.y
    if y > tmp then
        return y + 256
    end
    return base
end

local function get_bridge_height(bridge, x, z)
    if bridge.rotation.y == -0x8000 then
        return sector_local(x)
    elseif bridge.rotation.y == 0x4000 then
        return sector_local(z)
    elseif bridge.rotation.y == -0x4000 then
        return sector_local(1024 - z)
    elseif bridge.rotation.y == 0 then
        return sector_local(1024 - x)
    end
end

-- Sloped bridge 1
infos[69].floor = function(item, x, y, z, base)
    local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 4)
    if y <= tmp then
        return y
    end
    return base
end
infos[69].ceiling = function(item, x, y, z, base)
    local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 4)
    if y > tmp then
        return y + 256
    end
    return base
end

-- Sloped bridge 2
infos[70].floor = function(item, x, y, z, base)
    local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 2)
    if y <= tmp then
        return y
    end
    return base
end
infos[70].ceiling = function(item, x, y, z, base)
    local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 2)
    if y > tmp then
        return y + 256
    end
    return base
end

return infos
