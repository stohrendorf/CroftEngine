#include "ai.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace ai
{
namespace
{
const char* toString(Mood m)
{
    switch( m )
    {
        case Mood::Bored:
            return "Bored";
        case Mood::Attack:
            return "Attack";
        case Mood::Escape:
            return "Escape";
        case Mood::Stalk:
            return "Stalk";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid Mood" ) );
    }
}

Mood parseMood(const std::string& s)
{
    if( s == "Bored" )
        return Mood::Bored;
    if( s == "Attack" )
        return Mood::Attack;
    if( s == "Escape" )
        return Mood::Escape;
    if( s == "Stalk" )
        return Mood::Stalk;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid Mood" ) );
}
}

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

bool LotInfo::calculateTarget(const level::Level& lvl,
                              core::TRVec& target,
                              const items::ItemState& item)
{
    updatePath( lvl, 5 );

    target = item.position.position;

    auto box = item.box_number;
    if( box == nullptr )
        return false;

    int minZ = 0, maxZ = 0, minX = 0, maxX = 0;

    auto clampX = [&minX, &maxX, &box]() {
        minX = std::max( minX, box->xmin );
        maxX = std::min( maxX, box->xmax );
    };

    auto clampZ = [&minZ, &maxZ, &box]() {
        minZ = std::max( minZ, box->zmin );
        maxZ = std::min( maxZ, box->zmax );
    };

    constexpr auto NoClampXPos = 0x01;
    constexpr auto NoClampXNeg = 0x02;
    constexpr auto NoClampZPos = 0x04;
    constexpr auto NoClampZNeg = 0x08;
    constexpr auto ClampNone = NoClampXPos | NoClampXNeg | NoClampZPos | NoClampZNeg;
    constexpr auto Flag10 = 0x10;

    int unclampedDirs = ClampNone;
    while( true )
    {
        if( fly != 0 )
        {
            if( box->floor - loader::SectorSize < target.Y )
                target.Y = box->floor - loader::SectorSize;
        }
        else
        {
            if( box->floor < target.Y )
                target.Y = box->floor;
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
                    target.Z = std::max( target.Z, box->zmin + loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampX();

                    unclampedDirs = NoClampZNeg;
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
                    target.Z = std::min( target.Z, box->zmax - loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampX();

                    unclampedDirs = NoClampZPos;
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
                    target.X = std::max( target.X, box->xmin + loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampZ();

                    unclampedDirs = NoClampXNeg;
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
                    target.X = std::min( target.X, box->xmax - loader::SectorSize / 2 );

                    if( unclampedDirs & Flag10 )
                        return true;

                    clampZ();

                    unclampedDirs = NoClampXPos;
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

        if( box == target_box )
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

            if( unclampedDirs & (NoClampXPos | NoClampXNeg) )
            {
                target.X = this->target.X;
            }
            else if( !(unclampedDirs & Flag10) )
            {
                target.X = util::clamp( target.X, box->xmin + loader::SectorSize / 2,
                                        box->xmax - loader::SectorSize / 2 );
            }

            target.Y = this->target.Y;

            return true;
        }

        const auto nextBox = this->nodes[box].exit_box;
        if( nextBox == nullptr || (nextBox->overlap_index & block_mask) )
            break;

        box = nextBox;
    }

    BOOST_ASSERT( box != nullptr );
    if( unclampedDirs & (NoClampZPos | NoClampZNeg) )
    {
        const auto center = box->zmax - box->zmin - loader::SectorSize;
        target.Z = util::rand15( center ) + box->zmin + loader::SectorSize / 2;
    }
    else if( !(unclampedDirs & Flag10) )
    {
        target.Z = util::clamp( target.Z, box->zmin + loader::SectorSize / 2,
                                box->zmax - loader::SectorSize / 2 );
    }

    if( unclampedDirs & (NoClampXPos | NoClampXNeg) )
    {
        const auto center = box->xmax - box->xmin - loader::SectorSize;
        target.X = util::rand15( center ) + box->xmin + loader::SectorSize / 2;
    }
    else if( !(unclampedDirs & Flag10) )
    {
        target.X = util::clamp( target.X, box->xmin + loader::SectorSize / 2,
                                box->xmax - loader::SectorSize / 2 );
    }

    if( fly != 0 )
        target.Y = box->floor - 384;
    else
        target.Y = box->floor;

    return false;
}

YAML::Node LotInfo::save(const level::Level& lvl) const
{
    YAML::Node node;
    for( const auto& entry : nodes )
        node["nodes"][std::distance( &lvl.m_boxes[0], entry.first )] = entry.second.save( lvl );
    for( const auto& box : boxes )
        node["boxes"].push_back( std::distance( &lvl.m_boxes[0], box.get() ) );
    if( head != nullptr )
        node["head"] = std::distance( &lvl.m_boxes[0], head );
    if( tail != nullptr )
        node["tail"] = std::distance( &lvl.m_boxes[0], tail );
    node["searchVersion"] = m_searchVersion;
    node["blockMask"] = block_mask;
    node["step"] = step;
    node["drop"] = drop;
    node["fly"] = fly;
    if( target_box != nullptr )
        node["targetBox"] = std::distance( &lvl.m_boxes[0], target_box );
    if( required_box != nullptr )
        node["requiredBox"] = std::distance( &lvl.m_boxes[0], required_box );
    node["target"] = target.save();

    return node;
}

void LotInfo::load(const YAML::Node& n, const level::Level& lvl)
{
    nodes.clear();
    for( const auto& entry : n["nodes"] )
        nodes[&lvl.m_boxes.at( entry.first.as<size_t>() )].load( entry.second, lvl );
    boxes.clear();
    for( const auto& entry : n["boxes"] )
        boxes.emplace_back( &lvl.m_boxes.at( entry.as<size_t>() ) );
    if( !n["head"].IsDefined() )
        head = nullptr;
    else
        head = &lvl.m_boxes.at( n["head"].as<size_t>() );
    if( !n["tail"].IsDefined() )
        tail = nullptr;
    else
        tail = &lvl.m_boxes.at( n["tail"].as<size_t>() );
    m_searchVersion = n["searchVersion"].as<uint16_t>();
    block_mask = n["blockMask"].as<uint16_t>();
    step = n["step"].as<int16_t>();
    drop = n["drop"].as<int16_t>();
    fly = n["fly"].as<int16_t>();
    if( !n["targetBox"].IsDefined() )
        target_box = nullptr;
    else
        target_box = &lvl.m_boxes.at( n["targetBox"].as<size_t>() );
    if( !n["requiredBox"].IsDefined() )
        required_box = nullptr;
    else
        required_box = &lvl.m_boxes.at( n["requiredBox"].as<size_t>() );
    target.load( n["target"] );
}

void updateMood(const level::Level& lvl, const items::ItemState& item, const AiInfo& aiInfo, const bool violent)
{
    if( item.creatureInfo == nullptr )
        return;

    CreatureInfo& creatureInfo = *item.creatureInfo;
    if( creatureInfo.lot.nodes[item.box_number].isBlocked()
        && creatureInfo.lot.nodes[item.box_number].getSearchVersion() == creatureInfo.lot.m_searchVersion )
    {
        creatureInfo.lot.required_box = nullptr;
    }

    if( creatureInfo.mood != Mood::Attack
        && creatureInfo.lot.required_box != nullptr
        && !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, *creatureInfo.lot.target_box ) )
    {
        if( aiInfo.zone_number == aiInfo.enemy_zone )
        {
            creatureInfo.mood = Mood::Bored;
        }
        creatureInfo.lot.required_box = nullptr;
    }
    const auto originalMood = creatureInfo.mood;
    if( lvl.m_lara->m_state.health <= 0 )
    {
        creatureInfo.mood = Mood::Bored;
    }
    else if( violent )
    {
        switch( creatureInfo.mood )
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
                if( item.is_hit && (util::rand15() < 2048 || aiInfo.zone_number != aiInfo.enemy_zone) )
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
                if( item.is_hit && (util::rand15() < 2048 || aiInfo.zone_number != aiInfo.enemy_zone) )
                {
                    creatureInfo.mood = Mood::Escape;
                }
                else if( aiInfo.zone_number != aiInfo.enemy_zone )
                {
                    creatureInfo.mood = Mood::Bored;
                }
                break;
            case Mood::Escape:
                if( aiInfo.zone_number == aiInfo.enemy_zone && util::rand15() < 256 )
                {
                    creatureInfo.mood = Mood::Stalk;
                }
                break;
        }
    }

    if( originalMood != creatureInfo.mood )
    {
        if( originalMood == Mood::Attack )
        {
            Expects( creatureInfo.lot.target_box != nullptr );
            creatureInfo.lot.setRandomSearchTarget( gsl::make_not_null( creatureInfo.lot.target_box ) );
        }
        creatureInfo.lot.required_box = nullptr;
    }

    switch( creatureInfo.mood )
    {
        case Mood::Attack:
            if( util::rand15() >= int( lvl.m_scriptEngine["getObjectInfo"].call<sol::table>(
                    item.object_number )["target_update_chance"] ) )
                break;

            creatureInfo.lot.target = lvl.m_lara->m_state.position.position;
            creatureInfo.lot.required_box = lvl.m_lara->m_state.box_number;
            if( creatureInfo.lot.fly != 0 && lvl.m_lara->isOnLand() )
                creatureInfo.lot.target.Y += lvl.m_lara->getSkeleton()->getInterpolationInfo( lvl.m_lara->m_state )
                                                .getNearestFrame()->bbox.minY;

            break;
        case Mood::Bored:
        {
            const auto box = creatureInfo.lot.boxes[util::rand15( creatureInfo.lot.boxes.size() )];
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, *box ) )
                break;

            if( item.stalkBox( lvl, *box ) )
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
            if( creatureInfo.lot.required_box != nullptr && item.stalkBox( lvl, *creatureInfo.lot.required_box ) )
                break;

            const auto box = creatureInfo.lot.boxes[util::rand15( creatureInfo.lot.boxes.size() )];
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, *box ) )
                break;

            if( item.stalkBox( lvl, *box ) )
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
            const auto box = creatureInfo.lot.boxes[util::rand15( creatureInfo.lot.boxes.size() )];
            if( !item.isInsideZoneButNotInBox( lvl, aiInfo.zone_number, *box )
                || creatureInfo.lot.required_box != nullptr )
                break;

            if( item.inSameQuadrantAsBoxRelativeToLara( lvl, box ) )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
            }
            else if( aiInfo.zone_number == aiInfo.enemy_zone && item.stalkBox( lvl, *box ) )
            {
                creatureInfo.lot.setRandomSearchTarget( box );
                creatureInfo.mood = Mood::Stalk;
            }
            break;
        }
    }

    if( creatureInfo.lot.target_box == nullptr )
    {
        Expects( item.box_number != nullptr );
        creatureInfo.lot.setRandomSearchTarget( gsl::make_not_null( item.box_number ) );
    }
    creatureInfo.lot.calculateTarget( lvl, creatureInfo.target, item );
}

AiInfo::AiInfo(const level::Level& lvl, items::ItemState& item)
{
    if( item.creatureInfo == nullptr )
        return;

    const auto zoneRef = loader::Box::getZoneRef( lvl.roomsAreSwapped,
                                                  item.creatureInfo->lot.fly,
                                                  item.creatureInfo->lot.step );

    item.box_number = item.getCurrentSector()->box;
    zone_number = item.box_number->*zoneRef;
    lvl.m_lara->m_state.box_number = lvl.m_lara->m_state.getCurrentSector()->box;
    enemy_zone = lvl.m_lara->m_state.box_number->*zoneRef;
    if( (item.creatureInfo->lot.block_mask & lvl.m_lara->m_state.box_number->overlap_index)
        || item.creatureInfo->lot.nodes[item.box_number].search_version
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

CreatureInfo::CreatureInfo(const level::Level& lvl, const gsl::not_null<items::ItemState*>& item)
        : lot{lvl}
{
    switch( item->object_number )
    {
        case engine::TR1ItemId::Wolf:
        case engine::TR1ItemId::LionMale:
        case engine::TR1ItemId::LionFemale:
        case engine::TR1ItemId::Panther:
            lot.drop = -loader::SectorSize;
            break;

        case engine::TR1ItemId::Bat:
        case engine::TR1ItemId::CrocodileInWater:
        case engine::TR1ItemId::Fish:
            lot.step = 20 * loader::SectorSize;
            lot.drop = -20 * loader::SectorSize;
            lot.fly = 16;
            break;

        case engine::TR1ItemId::Gorilla:
            lot.step = loader::SectorSize / 2;
            lot.drop = -loader::SectorSize;
            break;

        case engine::TR1ItemId::TRex:
        case engine::TR1ItemId::Mutant:
        case engine::TR1ItemId::CentaurMutant:
            lot.block_mask = 0x8000;
            break;
    }
}

YAML::Node CreatureInfo::save(const level::Level& lvl) const
{
    YAML::Node node;
    node["headRot"] = head_rotation.toDegrees();
    node["neckRot"] = neck_rotation.toDegrees();
    node["maxTurn"] = maximum_turn.toDegrees();
    node["flags"] = flags;
    node["mood"] = toString( mood );
    node["lot"] = lot.save( lvl );
    node["target"] = target.save();
    return node;
}

void CreatureInfo::load(const YAML::Node& n, const level::Level& lvl)
{
    head_rotation = core::Angle::fromDegrees( n["headRot"].as<float>() );
    neck_rotation = core::Angle::fromDegrees( n["neckRot"].as<float>() );
    maximum_turn = core::Angle::fromDegrees( n["maxTurn"].as<float>() );
    flags = n["flags"].as<uint16_t>();
    mood = parseMood( n["mood"].as<std::string>() );
    lot.load( n["lot"], lvl );
    target.load( n["target"] );
}

YAML::Node SearchNode::save(const level::Level& lvl) const
{
    YAML::Node node;
    node["searchVersion"] = search_version;
    if( exit_box != nullptr )
        node["exitBox"] = std::distance( &lvl.m_boxes[0], exit_box );
    if( next_expansion != nullptr )
        node["nextExpansion"] = std::distance( &lvl.m_boxes[0], next_expansion );
    return node;
}

void SearchNode::load(const YAML::Node& n, const level::Level& lvl)
{
    search_version = n["searchVersion"].as<uint16_t>();
    if( !n["exitBox"].IsDefined() )
        exit_box = nullptr;
    else
        exit_box = &lvl.m_boxes.at( n["exitBox"].as<size_t>() );
    if( !n["nextExpansion"].IsDefined() )
        next_expansion = nullptr;
    else
        next_expansion = &lvl.m_boxes.at( n["nextExpansion"].as<size_t>() );
}
}
}
