#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Block final : public ModelItemNode
        {
        public:
            Block(const gsl::not_null<level::Level*>& level,
                  const std::string& name,
                  const gsl::not_null<const loader::Room*>& room,
                  const core::Angle& angle,
                  const core::TRCoordinates& position,
                  const floordata::ActivationState& activationState,
                  int16_t darkness,
                  const loader::AnimatedModel& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags | NonLot, darkness, animatedModel)
            {
                if( m_triggerState != TriggerState::Locked )
                    loader::Room::patchHeightsForBlock(*this, -loader::SectorSize);
            }


            void onInteract(LaraNode& lara) override;

            void update() override;

        private:
            bool isOnFloor(int height) const;

            bool canPushBlock(int height, core::Axis axis) const;

            bool canPullBlock(int height, core::Axis axis) const;
        };
    }
}
