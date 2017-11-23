#include "ai.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace ai
{
gsl::span<const uint16_t> LotInfo::getOverlaps(const level::Level& lvl, const uint16_t idx)
{
    const auto first = &lvl.m_overlaps[idx & 0x3fff];
    auto last = first;
    const auto endOfUniverse = &lvl.m_overlaps.back() + 1;

    while( last < endOfUniverse && (*last & 0x8000) == 0 )
    {
        ++last;
    }

    return gsl::make_span( first, last + 1 );
}

bool
LotInfo::calculateTarget(const level::Level& lvl, core::TRCoordinates& target, const engine::items::ItemState& item)
{
    updatePath( lvl, 5 );
    target = item.position.position;
    auto box = item.box_number;
    if( box == nullptr )
    {
        return false;
    }

    constexpr auto NoClampXPos = 0x01;
    constexpr auto NoClampXNeg = 0x02;
    constexpr auto NoClampZPos = 0x04;
    constexpr auto NoClampZNeg = 0x08;
    constexpr auto ClampNone = NoClampXPos | NoClampXNeg | NoClampZPos | NoClampZNeg;
    constexpr auto Flag10 = 0x10;
    auto unclampedDirs = ClampNone;

    int32_t minZ = box->zmin, maxZ = box->zmax, minX = box->xmin, maxX = box->xmax;

    auto clampX = [&minX, &maxX, &box]() {
        minX = std::max( minX, box->xmin );
        maxX = std::min( maxX, box->xmax );
    };

    auto clampZ = [&minZ, &maxZ, &box]() {
        minZ = std::max( minZ, box->zmin );
        maxZ = std::min( maxZ, box->zmax );
    };

    while( true )
    {
        if( fly != 0 )
        {
            if( box->floor - loader::SectorSize < target.Y )
            {
                target.Y = box->floor - loader::SectorSize;
            }
        }
        else
        {
            if( box->floor < target.Y )
            {
                target.Y = box->floor;
            }
        }

        if( box->contains( item.position.position.X, item.position.position.Z ) )
        {
            minZ = box->zmin;
            maxZ = box->zmax;
            minX = box->xmin;
            maxX = box->xmax;
        }
        else
        {
            if( item.position.position.Z < box->zmin )
            {
                if( (unclampedDirs & NoClampZNeg) && box->containsX( item.position.position.X ) )
                {
                    unclampedDirs = NoClampZNeg;

                    target.Z = std::max( target.Z, box->zmin + loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampX();
                }
                else if( unclampedDirs != NoClampZNeg )
                {
                    target.Z = maxZ - loader::SectorSize / 2;
                    if( unclampedDirs != ClampNone )
                        return true;

                    unclampedDirs |= Flag10;
                }
            }
            else if( item.position.position.Z > box->zmax )
            {
                if( (unclampedDirs & NoClampZPos) && box->containsX( item.position.position.X ) )
                {
                    unclampedDirs = NoClampZPos;

                    target.Z = std::min( target.Z, box->zmax - loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampX();
                }
                else if( unclampedDirs != NoClampZPos )
                {
                    target.Z = minZ + loader::SectorSize / 2;
                    if( unclampedDirs != ClampNone )
                        return true;

                    unclampedDirs |= Flag10;
                }
            }

            if( item.position.position.X < box->xmin )
            {
                if( (unclampedDirs & NoClampXNeg) && box->containsZ( item.position.position.Z ) )
                {
                    unclampedDirs = NoClampXNeg;

                    target.X = std::max( target.X, box->xmin + loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampZ();
                }
                else if( unclampedDirs != NoClampXNeg )
                {
                    target.X = maxX - loader::SectorSize / 2;
                    if( unclampedDirs != ClampNone )
                        return true;

                    unclampedDirs |= Flag10;
                }
            }
            else if( item.position.position.X > box->xmax )
            {
                if( (unclampedDirs & NoClampXPos) && box->containsZ( item.position.position.Z ) )
                {
                    unclampedDirs = NoClampXPos;

                    target.X = std::min( target.X, box->xmax - loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampZ();
                }
                else if( unclampedDirs != NoClampXPos )
                {
                    target.X = minX + loader::SectorSize / 2;
                    if( unclampedDirs != ClampNone )
                        return true;

                    unclampedDirs |= Flag10;
                }
            }
        }

        if( box == this->target_box )
        {
            if( unclampedDirs & (NoClampZPos | NoClampZNeg) )
            {
                target.Z = this->target.Z;
            }
            else if( !(unclampedDirs & Flag10) )
            {
                target.Z = util::clamp( target.Z, box->zmin + loader::SectorSize / 2,
                                        box->zmax - loader::SectorSize / 2 );
            }

            target.Y = this->target.Y;
            if( unclampedDirs & (NoClampXPos | NoClampXNeg) )
            {
                target.X = this->target.X;
            }
            else if( !(unclampedDirs & Flag10) )
            {
                target.X = util::clamp( target.X, box->xmin + loader::SectorSize / 2,
                                        box->xmax - loader::SectorSize / 2 );
            }

            return true;
        }
        const auto nextBox = this->nodes[box].exit_box;
        if( nextBox == nullptr || (nextBox->overlap_index & this->block_mask) )
            break;

        box = nextBox;
    }

    BOOST_ASSERT( box != nullptr );
    if( unclampedDirs & (NoClampZPos | NoClampZNeg) )
    {
        const auto center = box->zmax - box->zmin - loader::SectorSize;
        const auto r = std::rand() & 0x7fff;
        target.Z = (r * center / 0x8000) + box->zmin + loader::SectorSize / 2;
    }
    else if( !(unclampedDirs & Flag10) )
    {
        target.Z = util::clamp( target.Z, box->zmin + loader::SectorSize / 2, box->zmax - loader::SectorSize / 2 );
    }

    if( unclampedDirs & (NoClampXPos | NoClampXNeg) )
    {
        const auto center = box->xmax - box->xmin - loader::SectorSize;
        const auto r = std::rand() & 0x7fff;
        target.X = (r * center / 0x8000) + box->xmin + loader::SectorSize / 2;
    }
    else if( !(unclampedDirs & Flag10) )
    {
        target.X = util::clamp( target.X, box->xmin + loader::SectorSize / 2, box->xmax - loader::SectorSize / 2 );
    }

    if( this->fly != 0 )
    {
        target.Y = box->floor - 384;
    }
    else
    {
        target.Y = box->floor;
    }
    return false;
}

void updateMood(const level::Level& lvl, const engine::items::ItemState& item, const AiInfo& aiInfo, const bool violent)
{
    if( item.creatureInfo == nullptr )
        return;

    CreatureInfo& creatureInfo = *item.creatureInfo;
    if( creatureInfo.lot.nodes[item.box_number].isBlocked() && creatureInfo.lot.nodes[item.box_number].getSearchVersion() == creatureInfo.lot.m_searchVersion )
    {
        creatureInfo.lot.required_box = nullptr;
    }
    if( creatureInfo.mood != Mood::Attack
        && creatureInfo.lot.required_box != nullptr
        && !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, creatureInfo.lot.target_box ) )
    {
        if( aiInfo.zone_number == aiInfo.enemy_zone )
        {
            creatureInfo.mood = Mood::Bored;
        }
        creatureInfo.lot.required_box = nullptr;
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
            switch( creatureInfo.mood )
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
                            && (creatureInfo.mood != Mood::Stalk || creatureInfo.lot.required_box != nullptr) )
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
            creatureInfo.lot.setRandomSearchTarget( creatureInfo.lot.target_box );
        }
        creatureInfo.lot.required_box = nullptr;
    }
    switch( creatureInfo.mood )
    {
        case Mood::Attack:
            if( (std::rand() & 0x7fff) >= int( lvl.m_scriptEngine["getObjectInfo"].call<sol::table>(
                    item.object_number )["target_update_chance"] ) )
                break;

            creatureInfo.lot.target = lvl.m_lara->m_state.position.position;
            creatureInfo.lot.required_box = lvl.m_lara->m_state.box_number;
            if( creatureInfo.lot.fly != 0 && lvl.m_lara->isOnLand() )
                creatureInfo.lot.target.Y += lvl.m_lara->getSkeleton()->getInterpolationInfo( item ).getNearestFrame()
                                                ->bbox.minY;

            break;
        case Mood::Bored:
        {
            const auto box = creatureInfo.lot.boxes[std::rand() % creatureInfo.lot.boxes.size()];
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, box ) )
                break;

            if( item.stalkBox( lvl, box ) )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
                creatureInfo.mood = Mood::Stalk;
            }
            else if( creatureInfo.lot.required_box == nullptr )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
            }
            break;
        }
        case Mood::Stalk:
        {
            if( creatureInfo.lot.required_box != nullptr && item.stalkBox( lvl, creatureInfo.lot.required_box ) )
                break;

            const auto box = creatureInfo.lot.boxes[std::rand() % creatureInfo.lot.boxes.size()];
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, box ) )
                break;

            if( item.stalkBox( lvl, box ) )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
            }
            else if( creatureInfo.lot.required_box == nullptr )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
                if( aiInfo.zone_number != aiInfo.enemy_zone )
                {
                    creatureInfo.mood = Mood::Bored;
                }
            }
            break;
        }
        case Mood::Escape:
        {
            const auto box = creatureInfo.lot.boxes[std::rand() % creatureInfo.lot.boxes.size()];
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, box )
                || creatureInfo.lot.required_box != nullptr )
                break;

            if( item.inSameQuadrantAsBoxRelativeToLara( lvl, box ) )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
            }
            else if( aiInfo.zone_number == aiInfo.enemy_zone && item.stalkBox( lvl, box ) )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
                creatureInfo.mood = Mood::Stalk;
            }
            break;
        }
    }
    if( creatureInfo.lot.target_box == nullptr )
    {
        creatureInfo.lot.setRandomSearchTarget( item.box_number );
    }
    creatureInfo.lot.calculateTarget( lvl, creatureInfo.target, item );
}

AiInfo::AiInfo(const level::Level& lvl, engine::items::ItemState& item)
{
    if( item.creatureInfo == nullptr )
        return;

    const auto zoneRef = loader::Box::getZoneRef( lvl.roomsAreSwapped, item.creatureInfo->lot.fly,
                                                  item.creatureInfo->lot.step );

    item.box_number = item.getCurrentSector()->box;
    zone_number = item.box_number->*zoneRef;
    lvl.m_lara->m_state.box_number = lvl.m_lara->m_state.getCurrentSector()->box;
    enemy_zone = lvl.m_lara->m_state.box_number->*zoneRef;
    if( (item.creatureInfo->lot.block_mask & lvl.m_lara->m_state.box_number->overlap_index)
        || item.creatureInfo->lot.nodes[item.box_number].search_number
           == (item.creatureInfo->lot.m_searchVersion | 0x8000) )
    {
        enemy_zone |= 0x4000;
    }

    sol::table objectInfo = lvl.m_scriptEngine["getObjectInfo"].call( item.object_number );
    const int pivotLength = objectInfo["pivot_length"];
    const auto dz = lvl.m_lara->m_state.position.position.Z
                    - (item.position.position.Z + pivotLength * item.rotation.Y.cos());
    const auto dx = lvl.m_lara->m_state.position.position.X
                    - (item.position.position.X + pivotLength * item.rotation.Y.sin());
    const auto pivotAngle = core::Angle::fromAtan( dx, dz );
    distance = util::square( dx ) + util::square( dz );
    angle = pivotAngle - item.rotation.Y;
    enemy_facing = pivotAngle - 180_deg - lvl.m_lara->m_state.rotation.Y;
    ahead = angle > -90_deg && angle < 90_deg;
    bite = false;
    if( ahead )
    {
        const auto laraY = lvl.m_lara->m_state.position.position.Y;
        if( item.position.position.Y - loader::QuarterSectorSize < laraY
            && item.position.position.Y + loader::QuarterSectorSize > laraY )
        {
            bite = true;
        }
    }
}
}
}
