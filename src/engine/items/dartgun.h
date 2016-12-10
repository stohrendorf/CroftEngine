#pragma once

#include "dart.h"


namespace engine
{
    namespace items
    {
        class DartGun final : public ItemNode
        {
        public:
            DartGun(const gsl::not_null<level::Level*>& level,
                    const std::string& name,
                    const gsl::not_null<const loader::Room*>& room,
                    const core::Angle& angle,
                    const core::ExactTRCoordinates& position,
                    uint16_t flags,
                    const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, flags, true, 0x10, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                if( updateTriggerTimeout(deltaTime) )
                {
                    if( getCurrentState() == 0 )
                    {
                        setTargetState(1);
                    }
                }
                else if( getCurrentState() == 1 )
                {
                    setTargetState(0);
                }

                if( getCurrentState() != 1 || getCurrentLocalTime() >= 1_frame )
                {
                    //ItemNode::updateImpl(deltaTime);
                    return;
                }

                auto axis = core::axisFromAngle(getRotation().Y, 45_deg);
                BOOST_ASSERT(axis.is_initialized());

                core::ExactTRCoordinates d(0, 512, 0);

                switch(*axis)
                {
                    case core::Axis::PosZ:
                        d.Z += 412;
                        break;
                    case core::Axis::PosX:
                        d.X += 412;
                        break;
                    case core::Axis::NegZ:
                        d.Z -= 412;
                        break;
                    case core::Axis::NegX:
                        d.X -= 412;
                        break;
                    default:
                        break;
                }

                auto dart = getLevel().createItem<Dart>(39, getCurrentRoom(), getRotation().Y, getPosition() - d, 0);
                dart->activate();
                dart->m_flags2_02_toggledOn = true;
                dart->m_flags2_04_ready = false;

                playSoundEffect(0x97);
                //ItemNode::updateImpl(deltaTime);
            }
        };
    }
}
