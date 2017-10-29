#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class BridgeFlat final : public ModelItemNode
        {
        public:
            BridgeFlat(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const core::Angle& angle,
                       const core::TRCoordinates& position,
                       uint16_t activationState,
                       int16_t darkness,
                       const loader::SkeletalModelType& animatedModel)
                : ModelItemNode{level, name, room, angle, position, activationState, false, 0, darkness, animatedModel}
            {
            }


            void patchFloor(const core::TRCoordinates& pos, int& y) override
            {
                if( pos.Y <= m_state.position.position.Y )
                    y = m_state.position.position.Y;
            }


            void patchCeiling(const core::TRCoordinates& pos, int& y) override
            {
                if( pos.Y <= m_state.position.position.Y )
                    return;

                y = m_state.position.position.Y + loader::QuarterSectorSize;
            }
        };
    }
}
