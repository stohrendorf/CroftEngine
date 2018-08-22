#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_51 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_51(LaraNode& lara)
            : AbstractStateHandler( lara, LaraStateId::MidasDeath )
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.falling = false;
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush);
        auto it = getLevel().m_animatedModels.find( engine::TR1ItemId::LaraShotgunAnim );
        if( it == getLevel().m_animatedModels.end() )
            return;

        const auto& alternateLara = *it->second;

        auto frameOffs = getLara().m_state.frame_number
                         - getLevel().m_animations[getLara().m_state.anim_number].firstFrame;
        switch( frameOffs )
        {
            case 5:
                getLara().getNode()->getChild( 3 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 3].get() );
                getLara().getNode()->getChild( 6 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 6].get() );
                break;
            case 70:
                getLara().getNode()->getChild( 2 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 2].get() );
                break;
            case 90:
                getLara().getNode()->getChild( 1 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 1].get() );
                break;
            case 100:
                getLara().getNode()->getChild( 5 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 5].get() );
                break;
            case 120:
                getLara().getNode()->getChild( 0 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 0].get() );
                getLara().getNode()->getChild( 4 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 4].get() );
                break;
            case 135:
                getLara().getNode()->getChild( 7 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 7].get() );
                break;
            case 150:
                getLara().getNode()->getChild( 11 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 11].get() );
                break;
            case 163:
                getLara().getNode()->getChild( 12 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 12].get() );
                break;
            case 174:
                getLara().getNode()->getChild( 13 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 13].get() );
                break;
            case 186:
                getLara().getNode()->getChild( 8 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 8].get() );
                break;
            case 195:
                getLara().getNode()->getChild( 9 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 9].get() );
                break;
            case 218:
                getLara().getNode()->getChild( 10 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 10].get() );
                break;
            case 225:
                getLara().getNode()->getChild( 14 )
                         ->setDrawable( getLevel().m_models2[alternateLara.frame_number + 14].get() );
                break;
        }
        StateHandler_50::emitSparkles( getLara(), getLara().getLevel() );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        setMovementAngle( getLara().m_state.rotation.Y );
        collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
    }
};
}
}
