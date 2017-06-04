#pragma once

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


        inline std::ostream& operator<<(std::ostream& str, Mood mood)
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
                    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid mood"));
            }
        }


        struct LookAhead
        {
            core::Angle pivotAngleToLara;
            core::Angle laraAngleToPivot;
            long pivotDistanceToLaraSq = 0;
            bool enemyFacing = false;
            bool laraAhead = false;

            explicit LookAhead(const items::ItemNode& npc, int pivotDistance);
        };


        struct Brain;


        struct RoutePlanner
        {
            //! @brief Disallows entering certain boxes, marked in the @c loader::Box::overlap_index member.
            uint16_t blockMask;

            //! @brief The target box we need to reach
            boost::optional<uint16_t> destinationBox;

            //! @brief Temporary override for #searchTarget
            boost::optional<uint16_t> searchOverride;

            //! @brief The coordinates (within #destinationBox) the NPC needs to reach
            core::TRCoordinates searchTarget;

            bool calculateTarget(core::TRCoordinates& targetPos, const items::ItemNode& npc, const items::ItemNode* enemy);

            std::vector<gsl::not_null<const loader::Box*>> path;

            //! @brief Movement limits
            //! @{
            const int dropHeight;
            const int stepHeight;
            const int flyHeight;
            //! @}


            RoutePlanner(uint16_t blockMask, int dropHeight, int stepHeight, int flyHeight)
                : blockMask{blockMask}
                , dropHeight{dropHeight}
                , stepHeight{stepHeight}
                , flyHeight{flyHeight}
            {
            }


            void updateMood(Brain& brain, LookAhead& lookAhead, items::AIAgent& npc, bool ignoreProbabilities, uint16_t attackTargetUpdateProbability);


            void findPath(const items::ItemNode& npc, const items::ItemNode* enemy);


            static gsl::span<const uint16_t> getOverlaps(const level::Level& lvl, uint16_t idx);


            bool canTravelFromTo(const level::Level& lvl, uint16_t from, uint16_t to) const;


            const loader::ZoneData& getZoneData(const level::Level& lvl) const;


            uint16_t getZone(const items::ItemNode& item) const
            {
                const auto& zone = getZoneData(item.getLevel());
                const auto box = item.getCurrentBox();
                BOOST_ASSERT(box.is_initialized());
                const auto boxIdx = static_cast<uint16_t>(*box & ~0x8000);
                BOOST_ASSERT(boxIdx < zone.size());
                return zone[boxIdx];
            }


            void setRandomSearchTarget(uint16_t boxIdx, const items::ItemNode& npc);


            void setRandomSearchTarget(uint16_t boxIdx, const loader::Box& box)
            {
                searchTarget.X = (box.xmin + box.xmax - loader::SectorSize) * std::rand() / RAND_MAX + loader::SectorSize / 2;
                searchTarget.Z = (box.zmin + box.zmax - loader::SectorSize) * std::rand() / RAND_MAX + loader::SectorSize / 2;
                searchOverride = boxIdx & ~0x8000;
                if( flyHeight != 0 )
                {
                    searchTarget.Y = box.floor - core::ClimbLimit2ClickMin;
                }
                else
                {
                    searchTarget.Y = box.floor;
                }
            }


            static bool stalkBox(const items::ItemNode& npc, uint16_t box);


            static bool stalkBox(const items::ItemNode& npc, const loader::Box& box);


            static bool inSameQuadrantAsBoxRelativeToLara(const items::ItemNode& npc, uint16_t box);


            static bool inSameQuadrantAsBoxRelativeToLara(const items::ItemNode& npc, const loader::Box& box);
        };


        struct Brain
        {
            Mood mood = Mood::Bored;
            core::TRRotation jointRotation;

            core::TRCoordinates moveTarget;

            RoutePlanner route;


            Brain(uint16_t blockMask, int dropHeight, int stepHeight, int flyHeight)
                : route{blockMask, dropHeight, stepHeight, flyHeight}
            {
            }


            bool isInsideZoneButNotInBox(const items::AIAgent& npc, const loader::ZoneData& zone, uint16_t boxIdx) const;
        };
    }
}
