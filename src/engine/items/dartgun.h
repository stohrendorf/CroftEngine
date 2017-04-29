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
                    const core::TRCoordinates& position,
                    const floordata::ActivationState& activationState,
                    int16_t darkness,
                    const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints, darkness, animatedModel)
            {
            }


            void update() override
            {
                if( updateActivationTimeout() )
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

                if(/*frameChangeType == FrameChangeType::EndOfAnim ||*/ getCurrentState() != 1 || getCurrentLocalFrame() < 0 || getCurrentLocalFrame() >= 1)
                {
                    ItemNode::update();
                    return;
                }

                auto axis = core::axisFromAngle(getRotation().Y, 45_deg);
                BOOST_ASSERT(axis.is_initialized());

                core::TRCoordinates d(0, 512, 0);

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

                auto dart = getLevel().createItem<Dart>(39, getCurrentRoom(), getRotation().Y, getPosition() - d, floordata::ActivationState{});
                dart->activate();
                dart->m_triggerState = engine::items::TriggerState::Enabled;

                playSoundEffect(0x97);
                ItemNode::update();
            }
        };
    }
}
