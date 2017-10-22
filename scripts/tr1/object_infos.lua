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

function baddie_collision(baddie, lara, coll_info)
	if not baddie:within_proximity(lara, coll_info.radius) then
		return
	end
	if not baddie:test_bone_collision(lara) then
		return
	end
	if not coll_info.enable_baddie_push then
		return
	end

	local enable_spaz = false
	if baddie.health > 0 and coll_info.enable_spaz then
		enable_spaz = true
	end
	baddie:push_enemy(lara, coll_info, enable_spaz, false)
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

-- Lara Twin
infos[6] = {
	hit_points = 1000,
	shadow_size = 160,
	flags = (default_flags & ~0x0c) | 0x0c,
	radius = default_radius,
	initialise = function(item)
		item:copy_frame_base(0)
	end,
	control = function(item)
		-- TODO
	end,
	draw_routine = function(item)
		-- TODO
	end,
	collision = baddie_collision
}

local module = { object_infos = infos }
return module
