#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Dart final : public ModelItemNode
        {
        public:
            Dart(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::TRCoordinates& position,
                 uint16_t activationState,
                 int16_t darkness,
                 const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints, darkness, animatedModel)
            {
            }


            void update() override
            {
                // TODO: check bone collisions

                ModelItemNode::update();

                const loader::Room* room = m_state.position.room;
                auto sector = getLevel().findRealFloorSector(m_state.position.position, &room);
                if( room != m_state.position.room )
                    setCurrentRoom(room);

                HeightInfo h = HeightInfo::fromFloor(sector, m_state.position.position, getLevel().m_cameraController);
                m_state.floor = h.distance;

                if( m_state.position.position.Y < m_state.floor )
                    return;

                getLevel().scheduleDeletion(this);
                deactivate();

                // TODO: fx
            }
        };
    }
}
