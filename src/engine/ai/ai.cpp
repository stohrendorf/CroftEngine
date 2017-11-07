#include "ai.h"

#include "level/level.h"
#include "engine/laranode.h"
#include "engine/items/aiagent.h"

namespace engine
{
namespace ai
{
gsl::span<const uint16_t> LotInfo::getOverlaps(const level::Level& lvl, uint16_t idx)
{
    const uint16_t* first = &lvl.m_overlaps[idx];
    const uint16_t* last = first;
    const uint16_t* const endOfUniverse = &lvl.m_overlaps.back();

    while( last <= endOfUniverse && (*last & 0x8000) == 0 )
    {
        ++last;
    }

    return gsl::make_span( first, last );
}

bool LotInfo::canTravelFromTo(const level::Level& lvl, uint16_t from, uint16_t to) const
{
    Expects( from < lvl.m_boxes.size() );
    Expects( to < lvl.m_boxes.size() );

    const auto& fromBox = lvl.m_boxes[from];
    const auto& toBox = lvl.m_boxes[to];
    if( (toBox.overlap_index & block_mask) != 0 )
    {
        return false;
    }

    const auto& zone = getZoneData( lvl );

    BOOST_ASSERT( from < zone.size() );
    BOOST_ASSERT( to < zone.size() );

    if( zone[from] != zone[to] )
    {
        return false;
    }

    const auto d = toBox.floor - fromBox.floor;
    return d >= drop && d <= step;
}

const loader::ZoneData& LotInfo::getZoneData(const level::Level& lvl) const
{
    if( fly != 0 )
    {
        return lvl.roomsAreSwapped ? lvl.m_alternateZones.flyZone : lvl.m_baseZones.flyZone;
    }
    else if( step == loader::QuarterSectorSize )
    {
        return lvl.roomsAreSwapped ? lvl.m_alternateZones.groundZone1 : lvl.m_baseZones.groundZone1;
    }
    else
    {
        return lvl.roomsAreSwapped ? lvl.m_alternateZones.groundZone2 : lvl.m_baseZones.groundZone2;
    }
}

void updateMood(const level::Level& lvl, const engine::items::ItemState& item, const AiInfo& aiInfo, bool violent)
{
    if( item.creatureInfo == nullptr )
        return;

    CreatureInfo& creatureInfo = *item.creatureInfo;
    if( creatureInfo.lot.nodes[item.box_number].search_number == (creatureInfo.lot.search_number | 0x8000) )
    {
        creatureInfo.lot.required_box = -1;
    }
    if( creatureInfo.mood != Mood::Attack
        && creatureInfo.lot.required_box >= 0
        && !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, creatureInfo.lot.target_box ) )
    {
        if( aiInfo.zone_number == aiInfo.enemy_zone )
        {
            creatureInfo.mood = Mood::Bored;
        }
        creatureInfo.lot.required_box = -1;
    }
    const auto originalMood = creatureInfo.mood;
    if( lvl.m_lara->m_state.health > 0 )
    {
        if( violent )
        {
            switch( originalMood )
            {
                case Mood::Bored:
                case Mood::Stalk:
                    if( aiInfo.zone_number == aiInfo.enemy_zone )
                    {
                        creatureInfo.mood = Mood::Attack;
                    }
                    else if( item.is_hit )
                    {
                        creatureInfo.mood = Mood::Escape;
                    }
                    break;
                case Mood::Attack:
                    if( aiInfo.zone_number != aiInfo.enemy_zone )
                    {
                        creatureInfo.mood = Mood::Bored;
                    }
                    break;
                case Mood::Escape:
                    if( aiInfo.zone_number == aiInfo.enemy_zone )
                    {
                        creatureInfo.mood = Mood::Attack;
                    }
                    break;
            }
        }
        else
        {
            switch( originalMood )
            {
                case Mood::Bored:
                case Mood::Stalk:
                    if( item.is_hit && ((std::rand() & 0x7fff) < 2048 || aiInfo.zone_number != aiInfo.enemy_zone) )
                    {
                        creatureInfo.mood = Mood::Escape;
                    }
                    else if( aiInfo.zone_number == aiInfo.enemy_zone )
                    {
                        if( aiInfo.distance >= util::square( 3 * loader::SectorSize )
                            && (creatureInfo.mood != Mood::Stalk || creatureInfo.lot.required_box != -1) )
                        {
                            creatureInfo.mood = Mood::Stalk;
                        }
                        else
                        {
                            creatureInfo.mood = Mood::Attack;
                        }
                    }
                    break;
                case Mood::Attack:
                    if( item.is_hit
                        && ((std::rand() & 0x7fff) < 2048 || aiInfo.zone_number != aiInfo.enemy_zone) )
                    {
                        creatureInfo.mood = Mood::Escape;
                    }
                    else if( aiInfo.zone_number != aiInfo.enemy_zone )
                    {
                        creatureInfo.mood = Mood::Bored;
                    }
                    break;
                case Mood::Escape:
                    if( aiInfo.zone_number == aiInfo.enemy_zone && (std::rand() & 0x7fff) < 256 )
                    {
                        creatureInfo.mood = Mood::Stalk;
                    }
                    break;
            }
        }
    }
    else
    {
        creatureInfo.mood = Mood::Bored;
    }
    if( originalMood != creatureInfo.mood )
    {
        if( originalMood == Mood::Attack )
        {
            creatureInfo.lot.setRandomSearchTarget( lvl, creatureInfo.lot.target_box );
        }
        creatureInfo.lot.required_box = -1;
    }
    switch( creatureInfo.mood )
    {
        case Mood::Attack:
            if( (std::rand() & 0x7fff) >= int(lvl.m_scriptEngine["getObjectInfo"].call<sol::table>( item.object_number )["target_update_chance"]) )
                break;

            creatureInfo.lot.target = lvl.m_lara->m_state.position.position;
            creatureInfo.lot.required_box = lvl.m_lara->m_state.box_number;
            if( creatureInfo.lot.fly != 0 && lvl.m_lara->isOnLand() )
                creatureInfo.lot.target.Y += lvl.m_lara->getSkeleton()->getInterpolationInfo( item ).getNearestFrame()
                                                ->bbox.minY;

            break;
        case Mood::Bored:
        {
            const auto boxNumber = creatureInfo.lot.nodes[std::rand() % creatureInfo.lot.zone_count].box_number;
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, boxNumber ) )
                break;

            if( item.stalkBox( lvl, boxNumber ) )
            {
                creatureInfo.lot.setRandomSearchTarget( lvl, boxNumber );
                creatureInfo.mood = Mood::Stalk;
            }
            else if( creatureInfo.lot.required_box == -1 )
            {
                creatureInfo.lot.setRandomSearchTarget( lvl, boxNumber );
            }
            break;
        }
        case Mood::Stalk:
        {
            if( creatureInfo.lot.required_box != -1 && item.stalkBox( lvl, creatureInfo.lot.required_box ) )
                break;

            const auto boxNumber1 = creatureInfo.lot.nodes[std::rand() % creatureInfo.lot.zone_count].box_number;
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, boxNumber1 ) )
                break;

            if( item.stalkBox( lvl, boxNumber1 ) )
            {
                creatureInfo.lot.setRandomSearchTarget( lvl, boxNumber1 );
            }
            else if( creatureInfo.lot.required_box == -1 )
            {
                creatureInfo.lot.setRandomSearchTarget( lvl, boxNumber1 );
                if( aiInfo.zone_number != aiInfo.enemy_zone )
                {
                    creatureInfo.mood = Mood::Bored;
                }
            }
            break;
        }
        case Mood::Escape:
        {
            const auto boxNumber = creatureInfo.lot.nodes[std::rand() % creatureInfo.lot.zone_count].box_number;
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, boxNumber )
                || creatureInfo.lot.required_box != -1 )
                break;

            if( item.inSameQuadrantAsBoxRelativeToLara( lvl, boxNumber ) )
            {
                creatureInfo.lot.setRandomSearchTarget( lvl, boxNumber );
            }
            else if( aiInfo.zone_number == aiInfo.enemy_zone && item.stalkBox( lvl, boxNumber ) )
            {
                creatureInfo.lot.setRandomSearchTarget( lvl, boxNumber );
                creatureInfo.mood = Mood::Stalk;
            }
            break;
        }
    }
    if( creatureInfo.lot.target_box < 0 )
    {
        creatureInfo.lot.setRandomSearchTarget( lvl, item.box_number );
    }
    creatureInfo.lot.calculateTarget( lvl, creatureInfo.target, item );
}

AiInfo::AiInfo(const level::Level& lvl, engine::items::ItemState& item)
{
    if( item.creatureInfo == nullptr)
        return;

    const loader::ZoneData* zone = nullptr;
    if( item.creatureInfo->lot.fly != 0 )
    {
        zone = lvl.roomsAreSwapped ? &lvl.m_alternateZones.flyZone : &lvl.m_baseZones.flyZone;
    }
    else if( item.creatureInfo->lot.step == loader::QuarterSectorSize )
    {
        zone = lvl.roomsAreSwapped ? &lvl.m_alternateZones.groundZone1 : &lvl.m_baseZones.groundZone1;
    }
    else
    {
        zone = lvl.roomsAreSwapped ? &lvl.m_alternateZones.groundZone2 : &lvl.m_baseZones.groundZone2;
    }

    item.box_number = item.getCurrentSector()->boxIndex;
    zone_number = (*zone)[item.box_number];
    lvl.m_lara->m_state.box_number = lvl.m_lara->m_state.getCurrentSector()->boxIndex;
    enemy_zone = (*zone)[lvl.m_lara->m_state.box_number];
    if( (item.creatureInfo->lot.block_mask & lvl.m_boxes[lvl.m_lara->m_state.box_number].overlap_index)
        || item.creatureInfo->lot.nodes[item.box_number].search_number == (item.creatureInfo->lot.search_number | 0x8000) )
    {
        enemy_zone |= 0x4000;
    }

    sol::table objectInfo = lvl.m_scriptEngine["getObjectInfo"].call( item.object_number );
    const int pivotLength = objectInfo["pivot_length"];
    const auto dz = lvl.m_lara->m_state.position.position.Z - (item.position.position.Z + pivotLength * item.rotation.Y.cos());
    const auto dx = lvl.m_lara->m_state.position.position.X - (item.position.position.X + pivotLength * item.rotation.Y.sin());
    const auto pivotAngle = core::Angle::fromAtan(dx, dz);
    distance = util::square(dx) + util::square(dz);
    angle = pivotAngle - item.rotation.Y;
    enemy_facing = pivotAngle - 180_deg - lvl.m_lara->m_state.rotation.Y;
    ahead = angle > -90_deg && angle < 90_deg;
    bite = false;
    if( ahead )
    {
        const auto laraY = lvl.m_lara->m_state.position.position.Y;
        if( item.position.position.Y - loader::QuarterSectorSize < laraY && item.position.position.Y + loader::QuarterSectorSize > laraY )
        {
            bite = true;
        }
    }
}
}
}
