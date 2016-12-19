#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class TrapDoorDown final : public ItemNode
        {
        public:
            TrapDoorDown(const gsl::not_null<level::Level*>& level,
                         const std::string& name,
                         const gsl::not_null<const loader::Room*>& room,
                         const core::Angle& angle,
                         const core::ExactTRCoordinates& position,
                         uint16_t flags,
                         int16_t darkness,
                         const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, flags, true, 0x30, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                if( updateTriggerTimeout(deltaTime) )
                {
                    if( getCurrentState() == 0 )
                        setTargetState(1);
                }
                else if( getCurrentState() == 1 )
                {
                    setTargetState(0);
                }
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void patchFloor(const core::TRCoordinates& pos, long& y) override
            {
                if( getCurrentState() != 0 || !possiblyOnTrapdoor(pos) || pos.Y > getPosition().Y
                    || y <= getPosition().Y )
                    return;

                y = std::lround(getPosition().Y);
            }


            void patchCeiling(const core::TRCoordinates& pos, long& y) override
            {
                if( getCurrentState() != 0 || !possiblyOnTrapdoor(pos) || pos.Y <= getPosition().Y
                    || y > getPosition().Y )
                    return;

                y = std::lround(getPosition().Y + loader::QuarterSectorSize);
            }


        private:
            bool possiblyOnTrapdoor(const core::TRCoordinates& pos) const
            {
                auto trapdoorSectorX = std::lround(std::floor(getPosition().X / loader::SectorSize));
                auto trapdoorSectorZ = std::lround(std::floor(getPosition().Z / loader::SectorSize));
                auto posSectorX = pos.X / loader::SectorSize;
                auto posSectorZ = pos.Z / loader::SectorSize;
                auto trapdoorAxis = core::axisFromAngle(getRotation().Y, 1_au);
                BOOST_ASSERT(trapdoorAxis.is_initialized());

                if( *trapdoorAxis == core::Axis::PosZ && trapdoorSectorX == posSectorX && (trapdoorSectorZ + 1 == posSectorZ || trapdoorSectorZ == posSectorZ) )
                    return true;
                if( *trapdoorAxis == core::Axis::NegZ && trapdoorSectorX == posSectorX && (trapdoorSectorZ - 1 == posSectorZ || trapdoorSectorZ == posSectorZ) )
                    return true;
                if( *trapdoorAxis == core::Axis::PosX && trapdoorSectorZ == posSectorZ && (trapdoorSectorX + 1 == posSectorX || trapdoorSectorX == posSectorX) )
                    return true;
                if( *trapdoorAxis == core::Axis::NegX && trapdoorSectorZ == posSectorZ && (trapdoorSectorX - 1 == posSectorX || trapdoorSectorX == posSectorX) )
                    return true;

                return false;
            }
        };
    }
}
