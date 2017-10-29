#pragma once

#include "dart.h"


namespace engine
{
    namespace items
    {
        class DartGun final : public ModelItemNode
        {
        public:
            DartGun(const gsl::not_null<level::Level*>& level,
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
                if( m_state.updateActivationTimeout() )
                {
                    if( getCurrentState() == 0 )
                    {
                        m_state.goal_anim_state = 1;
                    }
                }
                else if( getCurrentState() == 1 )
                {
                    m_state.goal_anim_state = 0;
                }

                if(/*frameChangeType == FrameChangeType::EndOfAnim ||*/ getCurrentState() != 1 || m_state.frame_number != getLevel().m_animations[m_state.anim_number].firstFrame)
                {
                    ModelItemNode::update();
                    return;
                }

                auto axis = core::axisFromAngle(m_state.rotation.Y, 45_deg);
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

                auto dart = getLevel().createItem<Dart>(39, m_state.position.room, m_state.rotation.Y, m_state.position.position - d, 0);
                dart->activate();
                dart->m_triggerState = engine::items::TriggerState::Enabled;

                playSoundEffect(0x97);
                ModelItemNode::update();
            }
        };
    }
}
