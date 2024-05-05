from ce.engine import TR1ItemId, ObjectInfo
from ce.core import SectorSize

# init defaults
object_infos = {
    i: ObjectInfo()
    for i in TR1ItemId.__members__.values()
}

ob = object_infos[TR1ItemId.Lara]
ob.hit_points = 1000

ob = object_infos[TR1ItemId.Doppelganger]
ob.hit_points = 1000

ob = object_infos[TR1ItemId.Wolf]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 6
ob.pivot_length = 375
ob.target_update_chance = 0x2000
ob.drop_limit = -SectorSize

ob = object_infos[TR1ItemId.Bear]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 20
ob.pivot_length = 500
ob.target_update_chance = 0x4000

ob = object_infos[TR1ItemId.Bat]
ob.ai_agent = True
ob.radius = 102
ob.hit_points = 1
ob.target_update_chance = 0x400
ob.step_limit = 20 * SectorSize
ob.drop_limit = -20 * SectorSize
ob.fly_limit = 16

ob = object_infos[TR1ItemId.CrocodileOnLand]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 20
ob.pivot_length = 600
ob.target_update_chance = 0x2000

ob = object_infos[TR1ItemId.CrocodileInWater]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 20
ob.pivot_length = 600
ob.target_update_chance = 0x400
ob.step_limit = 20 * SectorSize
ob.drop_limit = -20 * SectorSize
ob.fly_limit = 16

ob = object_infos[TR1ItemId.LionMale]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 30
ob.pivot_length = 400
ob.target_update_chance = 0x7fff
ob.drop_limit = -SectorSize

ob = object_infos[TR1ItemId.LionFemale]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 25
ob.pivot_length = 400
ob.target_update_chance = 0x2000
ob.drop_limit = -SectorSize

ob = object_infos[TR1ItemId.Panther]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 45
ob.pivot_length = 400
ob.target_update_chance = 0x2000
ob.drop_limit = -SectorSize

ob = object_infos[TR1ItemId.Gorilla]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 22
ob.pivot_length = 250
ob.target_update_chance = 0x7fff
ob.step_limit = SectorSize // 2
ob.drop_limit = -SectorSize

ob = object_infos[TR1ItemId.RatOnLand]
ob.ai_agent = True
ob.radius = 204
ob.hit_points = 5
ob.pivot_length = 200
ob.target_update_chance = 0x2000

ob = object_infos[TR1ItemId.RatInWater]
ob.ai_agent = True
ob.radius = 204
ob.hit_points = 5
ob.pivot_length = 200
ob.target_update_chance = 0x2000

ob = object_infos[TR1ItemId.TRex]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 100
ob.pivot_length = 2000
ob.target_update_chance = 0x7fff
ob.cannot_visit_blockable = True
ob.cannot_visit_blocked = False

ob = object_infos[TR1ItemId.Raptor]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 20
ob.pivot_length = 400
ob.target_update_chance = 0x4000

ob = object_infos[TR1ItemId.FlyingMutant]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 50
ob.pivot_length = 150
ob.target_update_chance = 0x7fff
ob.cannot_visit_blockable = True
ob.cannot_visit_blocked = False

ob = object_infos[TR1ItemId.WalkingMutant1]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 50
ob.pivot_length = 150
ob.target_update_chance = 0x2000

ob = object_infos[TR1ItemId.WalkingMutant2]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 50
ob.pivot_length = 150
ob.target_update_chance = 0x2000

ob = object_infos[TR1ItemId.CentaurMutant]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 120
ob.pivot_length = 400
ob.target_update_chance = 0x7fff
ob.cannot_visit_blockable = True
ob.cannot_visit_blocked = False

ob = object_infos[TR1ItemId.Mummy]
ob.hit_points = 18

ob = object_infos[TR1ItemId.Larson]
ob.ai_agent = True
ob.radius = 102
ob.hit_points = 50
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.Pierre]
ob.ai_agent = True
ob.radius = 102
ob.hit_points = 70
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.SkateboardKid]
ob.ai_agent = True
ob.radius = 204
ob.hit_points = 125
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.TorsoBoss]
ob.ai_agent = True
ob.radius = 341
ob.hit_points = 500
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.Kold]
ob.ai_agent = True
ob.radius = 102
ob.hit_points = 200
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.Cowboy]
ob.ai_agent = True
ob.radius = 102
ob.hit_points = 150
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.Natla]
ob.ai_agent = True
ob.radius = 204
ob.hit_points = 400
ob.target_update_chance = 0x7fff

ob = object_infos[TR1ItemId.ScionPiece3]
ob.hit_points = 5

ob = object_infos[TR1ItemId.Fish]
ob.step_limit = 20 * SectorSize
ob.drop_limit = -20 * SectorSize
ob.fly_limit = 16

ob = object_infos[TR1ItemId.Sunglasses]
ob.hit_points = 1