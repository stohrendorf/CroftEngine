local infos = {}

local DEFAULT_RADIUS = 10
local DEFAULT_HIT_POINTS = -16384
local DEFAULT_PIVOT_LENGTH = 0
local DEFAULT_SHADOW_SIZE = 0

---------------------------------------------------------------------------
-- Code Completion Stubs

---@class Vector
---@field public x number
---@field public y number
---@field public y number
local VectorClass = {}

--- @class Item
--- @field public position Vector
--- @field public rotation Vector
--- @field public health number
--- @field public current_anim_state number
local ItemClass = {}

--- @param other Item
--- @param radius number
--- @return boolean
function ItemClass:is_near(other, radius)
end

--- @param other Item
--- @return boolean
function ItemClass:test_bone_collision(other)
end

---------------------------------------------------------------------------

--[[
Template for new object information structures:

infos[...] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    initialise = function(item)
    end
}

nmeshes, frame_number, bone_index, anim_index are filled from the level file
--]]

---@param item Item
local function baddie_init(item)
    item:set_y_angle((math.random(0, 32767) - 16384) / 2)
    item:set_collidable(true)
end

---@param baddie Item
---@param lara Item
local function baddie_interact(baddie, lara, lara_state)
    if baddie:is_near(lara, lara_state.radius) then
        if baddie:test_bone_collision(lara) then
            if lara_state:get_policy_flags().enable_baddie_push then
                local enable_spaz = false
                if baddie.health > 0 then
                    -- << 3 >> 7
                    enable_spaz = lara_state:get_policy_flags().enable_spaz
                end
                baddie:do_enemy_push(lara, lara_state, enable_spaz, false);
            end
        end
    end
end

local function sector_of(x)
    return math.floor(x / 1024)
end

local function sector_local(x)
    return x % 1024
end

-- Lara
infos[0] = {
    radius = DEFAULT_RADIUS,
    hit_points = 1000,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = 160,
    target_update_chance = 0
}

-- Lara Twin
infos[6] = {
    radius = DEFAULT_RADIUS,
    hit_points = 1000,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = 160,
    target_update_chance = 0
}

-- Wolf
infos[7] = {
    radius = 341,
    hit_points = 6,
    pivot_length = 375,
    shadow_size = 128,
    target_update_chance = 0x2000,
    ---@param item Item
    initialise = function(item)
        item.frame_number = 96
        baddie_init(item)
    end
    -- TODO: bone #3 uses custom Y rotation
}

-- Bear
infos[8] = {
    radius = 341,
    hit_points = 20,
    pivot_length = 500,
    shadow_size = 128,
    target_update_chance = 0x4000,
    initialise = baddie_init
    -- TODO: bone #14 uses custom Y rotation
}

-- Bat
infos[9] = {
    radius = 102,
    hit_points = 1,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = 128,
    target_update_chance = 0x400,
    initialise = baddie_init
}

-- Broken floor
infos[35] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    ---@param item Item
    ---@param x number
    ---@param y number
    ---@param z number
    ---@param base number
    ---@return number
    floor = function(item, x, y, z, base)
        local tmp = item.position.y - 512
        if y <= tmp then
            local state = item.current_anim_state
            if state == 0 or state == 1 then
                return tmp
            end
        end

        return base
    end,
    ---@param item Item
    ---@param x number
    ---@param y number
    ---@param z number
    ---@param base number
    ---@return number
    ceiling = function(item, x, y, z, base)
        local tmp = item.position.y - 512
        if y > tmp then
            local state = item.current_anim_state
            if state == 0 or state == 1 then
                return tmp + 256
            end
        end

        return base
    end
}

-- Swinging blade
infos[36] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = 128,
    target_update_chance = 0
}

-- Dart
infos[39] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = 128,
    target_update_chance = 0
}

-- Dart emitter
infos[40] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0
}

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
infos[41] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    floor = function(item, x, y, z, base)
        if item.current_anim_state == 1 and possibly_on_trapdoor_1(item, z, x) and y <= item.position.y then
            return item.position.y
        end
        return base
    end,
    ceiling = function(item, x, y, z, base)
        if item.current_anim_state == 1 and possibly_on_trapdoor_1(item, z, x) and y > item.position.y then
            return item.position.y + 256
        end
        return base
    end
}

-- Cubical blocks
infos[48] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    initialise = function(item)
        if item.activation_state ~= ActivationState.LOCKED then
            item:patch_heights(-1024)
        end
    end
}
infos[49] = infos[48]
infos[50] = infos[48]
infos[51] = infos[48]

-- Switch
infos[55] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0
}

-- Underwater switch
infos[56] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0
}

-- Doors
infos[57] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    initialise = function(item)
        -- TODO
    end
}
infos[58] = infos[57]
infos[59] = infos[57]
infos[60] = infos[57]
infos[61] = infos[57]
infos[62] = infos[57]
infos[63] = infos[57]
infos[64] = infos[57]

-- Trapdoors
infos[65] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    floor = function(item, x, y, z, base)
        local tmp = item.position.y
        if possibly_on_trapdoor_2(item, z, x) and y <= tmp and item.current_anim_state == 0 and base > tmp then
            return tmp
        end
        return base
    end,
    ceiling = function(item, x, y, z, base)
        local tmp = item.position.y
        if possibly_on_trapdoor_2(item, z, x) and y > tmp and item.current_anim_state == 0 and base < tmp then
            return tmp + 256
        end
        return base
    end
}
infos[66] = infos[65]

-- Flat bridge
infos[68] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    floor = function(item, x, y, z, base)
        local tmp = item.position.y
        if y <= tmp then
            return y
        end
        return base
    end,
    ceiling = function(item, x, y, z, base)
        local tmp = item.position.y
        if y > tmp then
            return y + 256
        end
        return base
    end
}
--- @param bridge Item
--- @param x number
--- @param z number
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
infos[69] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    floor = function(item, x, y, z, base)
        local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 4)
        if y <= tmp then
            return y
        end
        return base
    end,
    ceiling = function(item, x, y, z, base)
        local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 4)
        if y > tmp then
            return y + 256
        end
        return base
    end
}

-- Sloped bridge 2
infos[70] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0,
    floor = function(item, x, y, z, base)
        local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 2)
        if y <= tmp then
            return y
        end
        return base
    end,
    ceiling = function(item, x, y, z, base)
        local tmp = item.position.y + math.floor(get_bridge_height(item, x, z) / 2)
        if y > tmp then
            return y + 256
        end
        return base
    end
}

-- Simple animated entities
infos[74] = {
    radius = DEFAULT_RADIUS,
    hit_points = DEFAULT_HIT_POINTS,
    pivot_length = DEFAULT_PIVOT_LENGTH,
    shadow_size = DEFAULT_SHADOW_SIZE,
    target_update_chance = 0
}
infos[75] = infos[74]
infos[76] = infos[74]

local module = { object_infos = infos }
return module
