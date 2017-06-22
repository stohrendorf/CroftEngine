#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Dart final : public ItemNode
        {
        public:
            Dart(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::TRCoordinates& position,
                 const floordata::ActivationState& activationState,
                 int16_t darkness,
                 const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints, darkness, animatedModel)
            {
            }


            void update() override
            {
                // TODO: check bone collisions

                ItemNode::update();

                auto room = getCurrentRoom();
                auto sector = getLevel().findRealFloorSector(getPosition(), &room);
                if( room != getCurrentRoom() )
                    setCurrentRoom(room);

                HeightInfo h = HeightInfo::fromFloor(sector, getPosition(), getLevel().m_cameraController);
                setFloorHeight(h.distance);

                if( getPosition().Y < getFloorHeight() )
                    return;

                getLevel().scheduleDeletion(shared_from_this());
                deactivate();

                // TODO: fx
            }
        };
    }
}
