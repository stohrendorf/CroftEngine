#pragma once

#include "dart.h"

namespace engine
{
namespace items
{
class DartGun final : public ModelItemNode
{
public:
    DartGun(const gsl::not_null<Engine*>& engine,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            const loader::file::SkeletalModelType& animatedModel)
            : ModelItemNode{engine, room, item, true, animatedModel}
    {
    }

    void update() override
    {
        if( m_state.updateActivationTimeout() )
        {
            if( m_state.current_anim_state == 0_as )
            {
                m_state.goal_anim_state = 1_as;
            }
        }
        else if( m_state.current_anim_state == 1_as )
        {
            m_state.goal_anim_state = 0_as;
        }

        if( m_state.current_anim_state != 1_as || m_state.frame_number != m_state.anim->firstFrame )
        {
            ModelItemNode::update();
            return;
        }

        auto axis = axisFromAngle( m_state.rotation.Y, 45_deg );
        BOOST_ASSERT( axis.is_initialized() );

        core::TRVec d( 0_len, 512_len, 0_len );

        switch( *axis )
        {
            case core::Axis::PosZ:
                d.Z += 412_len;
                break;
            case core::Axis::PosX:
                d.X += 412_len;
                break;
            case core::Axis::NegZ:
                d.Z -= 412_len;
                break;
            case core::Axis::NegX:
                d.X -= 412_len;
                break;
            default:
                break;
        }

        auto dart = getEngine()
                .createItem<Dart>( core::TypeId{static_cast<core::TypeId::type>(TR1ItemId::Dart)},
                                   m_state.position.room,
                                   m_state.rotation.Y,
                                   m_state.position.position - d,
                                   0 );
        dart->activate();
        dart->m_state.triggerState = TriggerState::Active;

        playSoundEffect( TR1SoundId::DartgunShoot );
        ModelItemNode::update();
    }
};
}
}
