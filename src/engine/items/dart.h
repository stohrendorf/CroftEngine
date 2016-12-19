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
                 const core::ExactTRCoordinates& position,
                 uint16_t flags,
                 int16_t darkness,
                 const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, flags, true, 0x10, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                BOOST_LOG_TRIVIAL(debug) << "Dart " << reinterpret_cast<uintptr_t>(this) << " position " << getPosition().X << "/" << getPosition().Y << "/" << getPosition().Z;
                // TODO: check bone collisions

                auto room = getCurrentRoom();
                auto sector = getLevel().findFloorSectorWithClampedPosition(getPosition().toInexact(), &room);
                if( room != getCurrentRoom() )
                    setCurrentRoom(room);

                HeightInfo h = HeightInfo::fromFloor(sector, getPosition().toInexact(), getLevel().m_cameraController);
                setFloorHeight(h.distance);

                if( getPosition().Y < getFloorHeight() + 1 )
                    return;

                deactivate();

                // TODO: fx
            }
        };
    }
}
