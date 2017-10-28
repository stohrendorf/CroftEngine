local infos = {}

local default_radius = 10
local default_hit_points = -16384
local default_ceiling = 0
local default_floor = 0
local default_pivot_length = 0
local default_shadow_size = 0
local default_flags = 0x3e

function default_draw_routine(item)
    item:default_draw_routine()
end

function baddie_collision(baddie, enemy, coll_info)
    if not baddie:within_proximity(enemy, coll_info:get_radius()) then
        return
    end
    if not baddie:test_bone_collision(enemy) then
        return
    end
    if not coll_info:get_enable_baddie_push() then
        return
    end

    local enable_spaz = false
    if baddie.get_health() > 0 and coll_info:get_enable_spaz() then
        enable_spaz = true
    end
    baddie:push_enemy(enemy, coll_info, enable_spaz, false)
end

-- Lara
infos[0] = {
    hit_points = 1000,
    shadow_size = 160,
    flags = (default_flags & ~0x3c) | 0x3c,
    radius = default_radius,
    initialise = function(item)
        game:set_lara_item(item)
    end,
    control = function(item)
        -- nop - Lara is user-controlled
    end,
    draw_routine = function(item)
        -- nop - Lara is special
    end
}

function dump(o)
    if type(o) == 'table' then
        local s = '{ '
        for k, v in pairs(o) do
            if type(k) ~= 'number' then
                k = '"' .. k .. '"'
            end
            s = s .. '[' .. k .. '] = ' .. dump(v) .. ','
        end
        return s .. '} '
    else
        return tostring(o)
    end
end

-- Lara Twin
infos[6] = {
    hit_points = 1000,
    shadow_size = 160,
    flags = (default_flags & ~0x0c) | 0x0c,
    radius = default_radius,
    initialise = function(item)
        item:copy_frame_base(0)
    end,
    control = function(twin)
        local lara = game:get_lara_item()
        if twin:get_health() < 1000 then
            lara:set_health(lara:get_health() - 10 * (1000 - twin:get_health()))
            twin:set_health(1000)
        end
        -- TODO
    end,
    draw_routine = function(item)
        -- TODO
    end,
    collision = baddie_collision
}

local module = { object_infos = infos }
return module
