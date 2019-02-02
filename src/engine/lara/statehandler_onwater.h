#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

#include "engine/laranode.h"

namespace engine
{
namespace lara
{
class StateHandler_OnWater
        : public AbstractStateHandler
{
public:
    explicit StateHandler_OnWater(LaraNode& lara, const LaraStateId id)
            : AbstractStateHandler{lara, id}
    {
    }

protected:
    void commonOnWaterHandling(CollisionInfo& collisionInfo)
    {
        collisionInfo.facingAngle = getMovementAngle();
        collisionInfo.initHeightInfo( getLara().m_state.position.position + core::TRVec( 0, 700, 0 ), getLevel(), 700 );
        applyShift( collisionInfo );
        if( collisionInfo.mid.floor.y < 0
            || collisionInfo.collisionType == CollisionInfo::AxisColl::TopFront
            || collisionInfo.collisionType == CollisionInfo::AxisColl::TopBottom
            || collisionInfo.collisionType == CollisionInfo::AxisColl::Top
            || collisionInfo.collisionType == CollisionInfo::AxisColl::Front)
        {
            getLara().m_state.fallspeed = 0;
            getLara().m_state.position.position = collisionInfo.oldPosition;
        }
        else if( collisionInfo.collisionType == CollisionInfo::AxisColl::Left )
        {
            getLara().m_state.rotation.Y += 5_deg;
        }
        else if( collisionInfo.collisionType == CollisionInfo::AxisColl::Right )
        {
            getLara().m_state.rotation.Y -= 5_deg;
        }

        auto wsh = getLara().getWaterSurfaceHeight();
        if( wsh.is_initialized() && *wsh > getLara().m_state.position.position.Y - 100 )
        {
            tryClimbOutOfWater( collisionInfo );
            return;
        }

        setAnimation( loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041 );
        setGoalAnimState( LaraStateId::UnderwaterForward );
        getLara().m_state.rotation.X = -45_deg;
        getLara().m_state.fallspeed = 80;
        setUnderwaterState( UnderwaterState::Diving );
    }

private:
    void tryClimbOutOfWater(CollisionInfo& collisionInfo)
    {
        if( getMovementAngle() != getLara().m_state.rotation.Y )
        {
            return;
        }

        if( collisionInfo.collisionType != CollisionInfo::AxisColl::Front )
        {
            return;
        }

        if( !getLevel().m_inputHandler->getInputState().action )
        {
            return;
        }

        const auto gradient = std::abs( collisionInfo.frontLeft.floor.y - collisionInfo.frontRight.floor.y );
        if( gradient >= core::MaxGrabbableGradient )
        {
            return;
        }

        if( collisionInfo.front.ceiling.y > 0 )
        {
            return;
        }

        if( collisionInfo.mid.ceiling.y > -core::ClimbLimit2ClickMin )
        {
            return;
        }

        if( collisionInfo.front.floor.y + 700 <= -2 * loader::QuarterSectorSize )
        {
            return;
        }

        if( collisionInfo.front.floor.y + 700 > core::DefaultCollisionRadius )
        {
            return;
        }

        const auto yRot = alignRotation( getLara().m_state.rotation.Y, 35_deg );
        if( !yRot.is_initialized() )
        {
            return;
        }

        getLara().m_state.position.position += core::TRVec( 0, 695 + collisionInfo.front.floor.y, 0 );
        getLara().updateFloorHeight( -381 );
        core::TRVec d = getLara().m_state.position.position;
        if( *yRot == 0_deg )
        {
            d.Z = (getLara().m_state.position.position.Z / loader::SectorSize + 1) * loader::SectorSize
                  + core::DefaultCollisionRadius;
        }
        else if( *yRot == 180_deg )
        {
            d.Z = (getLara().m_state.position.position.Z / loader::SectorSize + 0) * loader::SectorSize
                  - core::DefaultCollisionRadius;
        }
        else if( *yRot == -90_deg )
        {
            d.X = (getLara().m_state.position.position.X / loader::SectorSize + 0) * loader::SectorSize
                  - core::DefaultCollisionRadius;
        }
        else if( *yRot == 90_deg )
        {
            d.X = (getLara().m_state.position.position.X / loader::SectorSize + 1) * loader::SectorSize
                  + core::DefaultCollisionRadius;
        }
        else
        {
            throw std::runtime_error( "Unexpected angle value" );
        }

        getLara().m_state.position.position = d;

        setAnimation( loader::AnimationId::CLIMB_OUT_OF_WATER, 1849 );
        setGoalAnimState( LaraStateId::Stop );
        getLara().m_state.speed = 0;
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        getLara().m_state.rotation.X = 0_deg;
        getLara().m_state.rotation.Y = *yRot;
        getLara().m_state.rotation.Z = 0_deg;
        setHandStatus( HandStatus::Grabbing );
        setUnderwaterState( UnderwaterState::OnLand );
    }
};
}
}
