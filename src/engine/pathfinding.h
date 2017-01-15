#pragma once

#include "items/itemnode.h"
#include "level/level.h"

namespace engine
{
    struct LotInfo
    {
        //! @brief Whether the NPC is a flying NPC
        bool fly = false;

        //! @brief Disallows entering certain boxes, marked in the @c loader::Box::overlap_index member.
        uint16_t block_mask = 0;

        //! @brief The start of the path
        uint16_t start_box;

        //! @brief The target box we need to reach
        uint16_t target_box;

        //! @brief The coordinates (within #target_box) the NPC needs to reach
        core::ExactTRCoordinates target;

        inline void calculateTarget(core::ExactTRCoordinates& targetPos, const items::ItemNode& npc, const items::ItemNode& enemy);

        std::vector<gsl::not_null<const loader::Box*>> path;


        const int stepHeight = loader::QuarterSectorSize;
        const int dropHeight = -loader::QuarterSectorSize;

        void findPath(const items::ItemNode& npc, const items::ItemNode& enemy)
        {
            path.clear();
            target = enemy.getPosition();

            if(auto tmp = npc.getCurrentBox())
            {
                start_box = *tmp & ~0xc000;
                BOOST_ASSERT(npc.getLevel().m_boxes[start_box].xmin <= npc.getPosition().X + 1);
                BOOST_ASSERT(npc.getLevel().m_boxes[start_box].xmax >= npc.getPosition().X - 1);
                BOOST_ASSERT(npc.getLevel().m_boxes[start_box].zmin <= npc.getPosition().Z + 1);
                BOOST_ASSERT(npc.getLevel().m_boxes[start_box].zmax >= npc.getPosition().Z - 1);
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "No box for NPC " << npc.getId();
                return;
            }

            if(auto tmp = enemy.getCurrentBox())
            {
                target_box = *tmp & ~0xc000;
                BOOST_ASSERT(enemy.getLevel().m_boxes[target_box].xmin <= enemy.getPosition().X + 1);
                BOOST_ASSERT(enemy.getLevel().m_boxes[target_box].xmax >= enemy.getPosition().X - 1);
                BOOST_ASSERT(enemy.getLevel().m_boxes[target_box].zmin <= enemy.getPosition().Z + 1);
                BOOST_ASSERT(enemy.getLevel().m_boxes[target_box].zmax >= enemy.getPosition().Z - 1);
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "No box for target " << enemy.getId();
                return;
            }

            static constexpr const uint16_t maxDepth = 5;

            std::map<uint16_t, uint16_t> parents;

            std::set<uint16_t> levelNodes;
            levelNodes.insert(start_box);

            std::map<uint16_t, uint16_t> costs;
            costs[start_box] = 0;

            for(uint16_t level = 1; level <= maxDepth; ++level)
            {
                std::set<uint16_t> nextLevel;

                for(const auto levelBoxIdx : levelNodes)
                {
                    BOOST_ASSERT(levelBoxIdx < npc.getLevel().m_boxes.size());
                    const auto& levelBox = npc.getLevel().m_boxes[levelBoxIdx];
                    const uint16_t overlapIdx = static_cast<uint16_t>( levelBox.overlap_index & ~0xc000 );
                    // examine edge levelBox --> childBox
                    for(auto childBox : getOverlaps(npc.getLevel(), overlapIdx))
                    {
                        BOOST_ASSERT(childBox != levelBoxIdx);
                        if(!canTravelFromTo(npc.getLevel(), levelBoxIdx, childBox))
                            continue;

                        if( costs.find(childBox) == costs.end() || level < costs[childBox] )
                        {
                            costs[childBox] = level;
                            parents[childBox] = levelBoxIdx;
                        }

                        if(childBox == target_box)
                        {
                            auto current = target_box;
                            BOOST_ASSERT(path.empty());
                            BOOST_ASSERT(current < npc.getLevel().m_boxes.size());
                            path.push_back(&npc.getLevel().m_boxes[current]);
                            while(parents.find(current) != parents.end())
                            {
                                current = parents[current];
                                BOOST_ASSERT(current < npc.getLevel().m_boxes.size());
                                path.push_back(&npc.getLevel().m_boxes[current]);
                            }

                            std::reverse(path.begin(), path.end());

                            BOOST_ASSERT(path.front() == &npc.getLevel().m_boxes[start_box]);

                            return;
                        }

                        nextLevel.insert(childBox);
                    }
                }
                levelNodes = std::move(nextLevel);
            }

            BOOST_ASSERT(path.empty());
        }


        static std::set<uint16_t> getOverlaps(const level::Level& lvl, uint16_t idx)
        {
            std::set<uint16_t> result;

            while(true)
            {
                BOOST_ASSERT(idx < lvl.m_overlaps.size());

                result.insert(lvl.m_overlaps[idx] & ~0xc000);

                if(lvl.m_overlaps[idx] & 0x8000)
                    break;

                ++idx;
            }

            return result;
        }


        bool canTravelFromTo(const level::Level& lvl, uint16_t from, uint16_t to) const
        {
            Expects(from < lvl.m_boxes.size());
            Expects(to < lvl.m_boxes.size());

            const auto& fromBox = lvl.m_boxes[from];
            const auto& toBox = lvl.m_boxes[to];
            if((toBox.overlap_index & block_mask) != 0)
            {
                BOOST_LOG_TRIVIAL(debug) << "Blocked";
                return false;
            }

            const loader::ZoneData* zone = nullptr;
            if ( fly )
            {
                zone = lvl.roomsAreSwapped ? &lvl.m_alternateZones.flyZone : &lvl.m_baseZones.flyZone;
            }
            else if ( stepHeight == loader::QuarterSectorSize )
            {
                zone = lvl.roomsAreSwapped ? &lvl.m_alternateZones.groundZone1 : &lvl.m_baseZones.groundZone1;
            }
            else
            {
                zone = lvl.roomsAreSwapped ? &lvl.m_alternateZones.groundZone2 : &lvl.m_baseZones.groundZone2;
            }

            BOOST_ASSERT(from < zone->size());
            BOOST_ASSERT(to < zone->size());

            if(zone[from] != zone[to])
            {
                return false;
            }

            const auto d = toBox.true_floor - fromBox.true_floor;
            //if(d > stepHeight || d < dropHeight)
            //    return false;

            return true;

        }
    };


    inline void LotInfo::calculateTarget(core::ExactTRCoordinates& targetPos, const items::ItemNode& npc, const items::ItemNode& enemy)
    {
        targetPos = npc.getPosition();

        findPath(npc, enemy);
        if(!npc.getCurrentBox().is_initialized() || path.empty())
            return;

        static constexpr const uint16_t AllowNegX = (1 << 0);
        static constexpr const uint16_t AllowPosX = (1 << 1);
        static constexpr const uint16_t AllowNegZ = (1 << 2);
        static constexpr const uint16_t AllowPosZ = (1 << 3);
        static constexpr const uint16_t AllowAll = AllowNegX | AllowPosX | AllowNegZ | AllowPosZ;
        static constexpr const uint16_t StayInBox = (1 << 4);
        uint16_t reachable = AllowAll;

        // Defines the reachable area
        int minZ = npc.getPosition().Z, maxZ = npc.getPosition().Z, minX = npc.getPosition().X, maxX = npc.getPosition().X;
        BOOST_LOG_TRIVIAL(debug) << "Start search " << npc.getId() << ", length = " << path.size();
        for(const auto& currentBox : path)
        {
            BOOST_LOG_TRIVIAL(debug) << "Box: xmin=" << currentBox->xmin
                                     << " xmax=" << currentBox->xmax
                                     << " zmin=" << currentBox->zmin
                                     << " zmax=" << currentBox->zmax;
            BOOST_LOG_TRIVIAL(debug) << "Flags = " << reachable;
            BOOST_LOG_TRIVIAL(debug) << "Target " << targetPos.X << "/" << targetPos.Y << "/" << targetPos.Z;
            if( fly )
            {
                if( currentBox->true_floor - loader::SectorSize < targetPos.Y )
                {
                    targetPos.Y = currentBox->true_floor - loader::SectorSize;
                }
            }
            else
            {
                if( currentBox->true_floor < targetPos.Y )
                {
                    targetPos.Y = currentBox->true_floor;
                }
            }

            if( npc.getPosition().Z + 1 >= currentBox->zmin && npc.getPosition().Z <= currentBox->zmax - 1
                && npc.getPosition().X + 1 >= currentBox->xmin && npc.getPosition().X <= currentBox->xmax - 1 )
            {
                BOOST_LOG_TRIVIAL(debug) << "Init limits";
                // initialize the reachable area to the box the NPC is in.
                minZ = currentBox->zmin;
                maxZ = currentBox->zmax;
                maxX = currentBox->xmax;
                minX = currentBox->xmin;
            }

            if( npc.getPosition().Z > currentBox->zmax )
            {
                BOOST_LOG_TRIVIAL(debug) << "To -Z";
                // need to travel to -Z
                if( (reachable & AllowNegZ)
                    && npc.getPosition().X >= currentBox->xmin
                    && npc.getPosition().X <= currentBox->xmax )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 1";
                    // we may expand the line, as we have an overlap on the X axis,
                    // an are also allowed to travel towards -Z

                    if( currentBox->zmax - loader::SectorSize / 2 < targetPos.Z )
                    {
                        // travel as far as we can
                        targetPos.Z = currentBox->zmax - loader::SectorSize / 2;
                    }

                    if( reachable & StayInBox )
                    {
                        return;
                    }

                    // clamp the reachable area on the X axis
                    if( currentBox->xmin > minX )
                    {
                        minX = currentBox->xmin;
                    }
                    if( currentBox->xmax < maxX )
                    {
                        maxX = currentBox->xmax;
                    }

                    // Now we only can travel towards -Z
                    reachable = AllowNegZ;
                }
                else if( reachable != AllowNegZ )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 2";
                    // We can't travel to -Z, but there are other axes we may travel to.
                    // So let's travel towards -Z as far as we can, stopping right before the reachable area limit.
                    targetPos.Z = minZ + loader::SectorSize / 2;
                    if( reachable != AllowAll )
                    {
                        // This is only the case if we're not examining the NPC's start box anymore.
                        // Thus, there's no use of searching further, as we need to reach a target
                        // which we can't reach due to the obstacles we encountered so far.
                        return;
                    }
                    reachable |= StayInBox;
                }
            }
            else if( npc.getPosition().Z < currentBox->zmin )
            {
                BOOST_LOG_TRIVIAL(debug) << "To +Z";
                if( (reachable & AllowPosZ)
                    && npc.getPosition().X >= currentBox->xmin
                    && npc.getPosition().X <= currentBox->xmax )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 1";
                    if( currentBox->zmin + loader::SectorSize / 2 > targetPos.Z )
                    {
                        targetPos.Z = currentBox->zmin + loader::SectorSize / 2;
                    }
                    if( reachable & StayInBox )
                    {
                        return;
                    }
                    if( currentBox->xmin > minX )
                    {
                        minX = currentBox->xmin;
                    }
                    if( currentBox->xmax < maxX )
                    {
                        maxX = currentBox->xmax;
                    }
                    reachable = AllowPosZ;
                }
                else if( reachable != AllowPosZ )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 2";
                    targetPos.Z = maxZ - loader::SectorSize / 2;
                    if( reachable != AllowAll )
                    {
                        return;
                    }
                    reachable |= StayInBox;
                }
            }

            if( npc.getPosition().X > currentBox->xmax )
            {
                BOOST_LOG_TRIVIAL(debug) << "To -X";
                if( (reachable & AllowNegX)
                    && npc.getPosition().Z >= currentBox->zmin
                    && npc.getPosition().Z <= currentBox->zmax )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 1";
                    if( currentBox->xmax - loader::SectorSize / 2 < targetPos.X )
                    {
                        targetPos.X = currentBox->xmax - loader::SectorSize / 2;
                    }
                    if( reachable & StayInBox )
                    {
                        return;
                    }
                    if( currentBox->zmin > minZ )
                    {
                        minZ = currentBox->zmin;
                    }
                    if( currentBox->zmax < maxZ )
                    {
                        maxZ = currentBox->zmax;
                    }
                    reachable = AllowNegX;
                }
                else if( reachable != AllowNegX )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 2";
                    targetPos.X = minX + loader::SectorSize / 2;
                    if( reachable != AllowAll )
                    {
                        return;
                    }
                    reachable |= StayInBox;
                }
            }
            else if( npc.getPosition().X < currentBox->xmin )
            {
                BOOST_LOG_TRIVIAL(debug) << "To +X";
                if( (reachable & AllowPosX)
                    && npc.getPosition().Z >= currentBox->zmin
                    && npc.getPosition().Z <= currentBox->zmax )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 1";
                    if( currentBox->xmin + loader::SectorSize / 2 > targetPos.X )
                    {
                        targetPos.X = currentBox->xmin + loader::SectorSize / 2;
                    }
                    if( reachable & StayInBox )
                    {
                        return;
                    }
                    if( currentBox->zmin > minZ )
                    {
                        minZ = currentBox->zmin;
                    }
                    if( currentBox->zmax < maxZ )
                    {
                        maxZ = currentBox->zmax;
                    }
                    reachable = AllowPosX;
                }
                else if( reachable != AllowPosX )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Case 2";
                    targetPos.X = maxX - loader::SectorSize / 2;
                    if( reachable != AllowAll )
                    {
                        return;
                    }
                    reachable |= StayInBox;
                }
            }

            if( currentBox == path.back() )
            {
                BOOST_LOG_TRIVIAL(debug) << "Target reached";
                if( reachable & (AllowNegZ | AllowPosZ) )
                {
                    targetPos.Z = target.Z;
                }
                if( reachable & (AllowNegX | AllowPosX) )
                {
                    targetPos.X = target.X;
                }
                if( !(reachable & StayInBox) )
                {
                    targetPos.X = glm::clamp(std::lround(targetPos.X), currentBox->xmin + loader::SectorSize / 2L, currentBox->xmax - loader::SectorSize / 2L);
                    targetPos.Z = glm::clamp(std::lround(targetPos.Z), currentBox->zmin + loader::SectorSize / 2L, currentBox->zmax - loader::SectorSize / 2L);
                }

                targetPos.Y = target.Y;
                return;
            }
        }

        const loader::Box* endBox = nullptr;
        if(!path.empty())
        {
            endBox = path.back();
        }
        else
        {
            BOOST_ASSERT(*npc.getCurrentBox() < npc.getLevel().m_boxes.size());
            endBox = &npc.getLevel().m_boxes[*npc.getCurrentBox()];
        }

        if( reachable & (AllowNegZ | AllowPosZ) )
        {
            const auto v_centerZ = endBox->zmax - endBox->zmin - loader::SectorSize;
            targetPos.Z = (v_centerZ * std::rand() / RAND_MAX) + endBox->zmin + loader::SectorSize / 2;
        }
        else if( !(reachable & StayInBox) )
        {
            if( endBox->zmin + loader::SectorSize / 2 > targetPos.Z )
            {
                targetPos.Z = endBox->zmin + loader::SectorSize / 2;
            }
            else if( endBox->zmax - loader::SectorSize / 2 < targetPos.Z )
            {
                targetPos.Z = endBox->zmax - loader::SectorSize / 2;
            }
        }

        if( reachable & (AllowNegX | AllowPosX) )
        {
            const auto v_centerX = endBox->xmax - endBox->xmin - loader::SectorSize;
            targetPos.X = (v_centerX * std::rand() / RAND_MAX) + endBox->xmin + loader::SectorSize / 2;
        }
        else if( !(reachable & StayInBox) )
        {
            if( endBox->xmin + loader::SectorSize / 2 > targetPos.X )
            {
                targetPos.X = endBox->xmin + loader::SectorSize / 2;
            }
            else if( endBox->xmax - loader::SectorSize / 2 < targetPos.X )
            {
                targetPos.X = endBox->xmax - loader::SectorSize / 2;
            }
        }

        if( fly )
        {
            targetPos.Y = endBox->true_floor - core::ClimbLimit2ClickMin;
        }
        else
        {
            targetPos.Y = endBox->true_floor;
        }
    }
}
