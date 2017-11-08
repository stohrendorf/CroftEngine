#pragma once

#include <level/level.h>
#include "engine/items/itemnode.h"

namespace engine
{
namespace items
{
class AIAgent;
}

namespace ai
{
enum class Mood
{
    Bored,
    Attack,
    Escape,
    Stalk
};

inline std::ostream& operator<<(std::ostream& str, const Mood mood)
{
    switch( mood )
    {
        case Mood::Bored:
            return str << "Bored";
        case Mood::Attack:
            return str << "Attack";
        case Mood::Escape:
            return str << "Escape";
        case Mood::Stalk:
            return str << "Stalk";
        default:
            BOOST_THROW_EXCEPTION( std::runtime_error( "Invalid mood" ) );
    }
}

struct BoxNode
{
    const loader::Box* exit_box = nullptr;
    uint16_t search_number = 0;
    const loader::Box* next_expansion = nullptr;
};


struct LotInfo
{
    std::unordered_map<const loader::Box*, BoxNode> nodes;
    std::vector<const loader::Box*> boxes;
    const loader::Box* head = nullptr;
    const loader::Box* tail = nullptr;
    uint16_t search_number = 0;
    //! @brief Disallows entering certain boxes, marked in the @c loader::Box::overlap_index member.
    uint16_t block_mask = 0x4000;
    //! @brief Movement limits
    //! @{
    int16_t step = loader::QuarterSectorSize;
    int16_t drop = -loader::QuarterSectorSize;
    int16_t fly = 0;
    //! @}
    //! @brief The target box we need to reach
    const loader::Box* target_box = nullptr;
    const loader::Box* required_box = nullptr;
    core::TRCoordinates target;

    explicit LotInfo(const level::Level& lvl)
    {
        for (const auto& box : lvl.m_boxes)
            nodes.insert(std::make_pair(&box, BoxNode{}));
    }

    static gsl::span<const uint16_t> getOverlaps(const level::Level& lvl, uint16_t idx);

    void setRandomSearchTarget(const loader::Box* box)
    {
        Expects(box != nullptr);
        required_box = box;
        const auto zSize = required_box->zmax - required_box->zmin - loader::SectorSize;
        target.Z = zSize * (std::rand() & 0x7fff) / 0x8000 + required_box->zmin + loader::SectorSize / 2;
        const auto xSize = required_box->xmax - required_box->xmin - loader::SectorSize;
        target.X = xSize * (std::rand() & 0x7fff) / 0x8000 + required_box->xmin + loader::SectorSize / 2;
        if( fly )
        {
            target.Y = required_box->floor - 384;
        }
        else
        {
            target.Y = required_box->floor;
        }
    }

    bool calculateTarget(const level::Level& lvl, core::TRCoordinates& target, const engine::items::ItemState& item)
    {
        updatePath( lvl, 5 );
        target = item.position.position;
        auto boxNumber = item.box_number;
        if( boxNumber == nullptr )
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

        int32_t minZ = 0, maxZ = 0, minX = 0, maxX = 0;

        const loader::Box* box = nullptr;
        do
        {
            box = boxNumber;

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

            if( item.position.position.Z >= box->zmin && item.position.position.Z <= box->zmax
                && item.position.position.X >= box->xmin && item.position.position.X <= box->xmax )
            {
                minZ = box->zmin;
                maxZ = box->zmax;
                maxX = box->xmax;
                minX = box->xmin;
            }
            else
            {
                if( item.position.position.Z < box->zmin )
                {
                    if( (unclampedDirs & NoClampZNeg) && item.position.position.X >= box->xmin
                        && item.position.position.X <= box->xmax )
                    {
                        target.Z = std::max( target.Z, box->zmin + loader::SectorSize / 2 );

                        if( unclampedDirs & Flag10 )
                            return true;

                        minX = std::max( minX, box->xmin );
                        maxX = std::min( maxX, box->xmax );
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
                    if( (unclampedDirs & NoClampZPos) && item.position.position.X >= box->xmin
                        && item.position.position.X <= box->xmax )
                    {
                        target.Z = std::min( target.Z, box->zmax - loader::SectorSize / 2 );

                        if( unclampedDirs & Flag10 )
                            return true;

                        minX = std::max( minX, box->xmin );
                        maxX = std::min( maxX, box->xmax );
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
                    if( (unclampedDirs & NoClampXNeg) && item.position.position.Z >= box->zmin
                        && item.position.position.Z <= box->zmax )
                    {
                        target.X = std::max( target.X, box->xmin + loader::SectorSize / 2 );

                        if( unclampedDirs & Flag10 )
                            return true;

                        minZ = std::max( minZ, box->zmin );
                        maxZ = std::min( maxZ, box->zmax );
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
                    if( (unclampedDirs & NoClampXPos) && item.position.position.Z >= box->zmin
                        && item.position.position.Z <= box->zmax )
                    {
                        target.X = std::min( target.X, box->xmax - loader::SectorSize / 2 );

                        if( unclampedDirs & Flag10 )
                            return true;

                        minZ = std::max( minZ, box->zmin );
                        maxZ = std::min( maxZ, box->zmax );
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

            if( boxNumber == this->target_box )
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
            boxNumber = this->nodes[boxNumber].exit_box;
        } while( boxNumber != nullptr && !(boxNumber->overlap_index & this->block_mask) );

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

    void updatePath(const level::Level& lvl, const uint8_t maxDepth)
    {
        if( required_box != nullptr && required_box != target_box )
        {
            target_box = required_box;
            const auto expand = &nodes[target_box];
            if( expand->next_expansion == nullptr && tail != target_box )
            {
                expand->next_expansion = head;
                if( head == nullptr )
                {
                    tail = target_box;
                }
                head = target_box;
            }
            ++search_number;
            expand->exit_box = nullptr;
            expand->search_number = search_number;
        }
        searchPath( lvl, maxDepth );
    }

    void searchPath(const level::Level& lvl, const uint8_t maxDepth)
    {
        const auto zoneRef = loader::Box::getZoneRef(lvl.roomsAreSwapped, fly, step);
        const auto currentZone = head->*zoneRef;
        for( uint8_t i = 0; i < maxDepth; ++i )
        {
            const auto box = head;
            if(box == nullptr )
            {
                return;
            }
            const auto node = &this->nodes[box];
            for(auto overlapBoxIdx : getOverlaps(lvl, box->overlap_index))
            {
                overlapBoxIdx &= 0x7FFFu;

                const auto* overlapBox = &lvl.m_boxes[overlapBoxIdx];

                if( currentZone != overlapBox->*zoneRef )
                    continue;

                const auto boxHeightDiff = overlapBox->floor - box->floor;
                if( boxHeightDiff > step || boxHeightDiff < drop )
                    continue;

                const auto nextExpansion = &this->nodes[overlapBox];
                const auto currentSearch = node->search_number & 0x7FFF;
                const auto expandSearch = nextExpansion->search_number & 0x7FFF;
                if( currentSearch < expandSearch )
                    continue;

                if( node->search_number & 0x8000 )
                {
                    if( currentSearch == expandSearch )
                    {
                        continue;
                    }
                    nextExpansion->search_number = node->search_number;
                }
                else if( currentSearch != expandSearch || (nextExpansion->search_number & 0x8000) )
                {
                    if( block_mask & overlapBox->overlap_index )
                    {
                        nextExpansion->search_number = node->search_number | 0x8000u;
                    }
                    else
                    {
                        nextExpansion->search_number = node->search_number;
                        nextExpansion->exit_box = head;
                    }
                }

                if( nextExpansion->next_expansion != nullptr )
                    continue;

                if(overlapBox == tail )
                    continue;

                this->nodes[tail].next_expansion = overlapBox;
                tail = overlapBox;
            }
            head = node->next_expansion;
            node->next_expansion = nullptr;
        }
    };
};


struct AiInfo
{
    loader::ZoneId zone_number;
    loader::ZoneId enemy_zone;
    int32_t distance;
    bool ahead;
    bool bite;
    core::Angle angle;
    core::Angle enemy_facing;

    AiInfo(const level::Level& lvl, engine::items::ItemState& item);
};

struct CreatureInfo
{
    core::Angle head_rotation = 0_deg;
    core::Angle neck_rotation = 0_deg;

    core::Angle maximum_turn = 1_deg;
    uint16_t flags = 0;

    engine::items::ItemState* item;
    uint16_t frame_number;
    Mood mood = Mood::Bored;
    LotInfo lot;
    core::TRCoordinates target;

    CreatureInfo(const level::Level& lvl, engine::items::ItemState* item)
        : item{item}
        , lot{lvl}
    {
        Expects(item != nullptr);

        switch( item->object_number )
        {
            case 7:
            case 12:
            case 13:
            case 14:
                lot.drop = -loader::SectorSize;
                break;

            case 9:
            case 11:
            case 26:
                lot.step = 20 * loader::SectorSize;
                lot.drop = -20 * loader::SectorSize;
                lot.fly = 16;
                break;

            case 15:
                lot.step = loader::SectorSize / 2;
                lot.drop = -loader::SectorSize;
                break;

            case 18:
            case 20:
            case 23:
                lot.block_mask = 0x8000;
                break;
        }
    }

    void rotateHead(const core::Angle& angle)
    {
        const auto delta = util::clamp(angle - head_rotation, -5_deg, +5_deg);
        head_rotation = util::clamp(delta + head_rotation, -90_deg, +90_deg);
    }

    static sol::usertype<CreatureInfo> userType()
    {
        return sol::usertype<CreatureInfo>(
                sol::meta_function::construct, sol::no_constructor,
                "head_rotation", &CreatureInfo::head_rotation,
                "neck_rotation", &CreatureInfo::neck_rotation,
                "maximum_turn", &CreatureInfo::maximum_turn,
                "flags", &CreatureInfo::flags,
                "item", sol::readonly(&CreatureInfo::item),
                "frame_number", &CreatureInfo::frame_number,
                "mood", &CreatureInfo::mood,
                "target", &CreatureInfo::target
        );
    }
};

void updateMood(const level::Level& lvl, const engine::items::ItemState& item, const AiInfo& aiInfo, bool violent);
}
}
