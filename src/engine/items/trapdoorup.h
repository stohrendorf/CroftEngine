#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class TrapDoorUp final : public ModelItemNode
        {
        public:
            TrapDoorUp(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const core::Angle& angle,
                       const core::TRCoordinates& position,
                       uint16_t activationState,
                       int16_t darkness,
                       const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags, darkness, animatedModel)
            {
            }


            void update() override;


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void patchFloor(const core::TRCoordinates& pos, int& y) override
            {
                if( getCurrentState() != 1 || !possiblyOnTrapdoor(pos) || pos.Y > getPosition().Y )
                    return;

                y = getPosition().Y;
            }


            void patchCeiling(const core::TRCoordinates& pos, int& y) override
            {
                if( getCurrentState() != 1 || !possiblyOnTrapdoor(pos) || pos.Y <= getPosition().Y )
                    return;

                y = getPosition().Y + loader::QuarterSectorSize;
            }


        private:
            bool possiblyOnTrapdoor(const core::TRCoordinates& pos) const
            {
                auto trapdoorSectorX = getPosition().X / loader::SectorSize;
                auto trapdoorSectorZ = getPosition().Z / loader::SectorSize;
                auto posSectorX = pos.X / loader::SectorSize;
                auto posSectorZ = pos.Z / loader::SectorSize;
                auto trapdoorAxis = core::axisFromAngle(getRotation().Y, 1_au);
                BOOST_ASSERT( trapdoorAxis.is_initialized() );

                if( *trapdoorAxis == core::Axis::PosZ && trapdoorSectorX == posSectorX && (trapdoorSectorZ - 1 == posSectorZ || trapdoorSectorZ - 2 == posSectorZ) )
                    return true;
                if( *trapdoorAxis == core::Axis::NegZ && trapdoorSectorX == posSectorX && (trapdoorSectorZ + 1 == posSectorZ || trapdoorSectorZ + 2 == posSectorZ) )
                    return true;
                if( *trapdoorAxis == core::Axis::PosX && trapdoorSectorZ == posSectorZ && (trapdoorSectorX - 1 == posSectorX || trapdoorSectorX - 2 == posSectorX) )
                    return true;
                if( *trapdoorAxis == core::Axis::NegX && trapdoorSectorZ == posSectorZ && (trapdoorSectorX + 1 == posSectorX || trapdoorSectorX + 2 == posSectorX) )
                    return true;

                return false;
            }
        };
    }
}
