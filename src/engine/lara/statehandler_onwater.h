#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

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
        collisionInfo.initHeightInfo( getLara().m_state.position.position + core::TRVec( 0_len, core::LaraSwimHeight, 0_len ),
                                      getEngine(),
                                      core::LaraSwimHeight );
        applyShift( collisionInfo );
        if( collisionInfo.mid.floorSpace.y < 0_len
            || collisionInfo.collisionType == CollisionInfo::AxisColl::TopFront
            || collisionInfo.collisionType == CollisionInfo::AxisColl::TopBottom
            || collisionInfo.collisionType == CollisionInfo::AxisColl::Top
            || collisionInfo.collisionType == CollisionInfo::AxisColl::Front )
        {
            getLara().m_state.fallspeed = 0_spd;
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
        if( wsh.is_initialized() && *wsh > getLara().m_state.position.position.Y - core::DefaultCollisionRadius )
        {
            tryClimbOutOfWater( collisionInfo );
            return;
        }

        setAnimation( AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041_frame );
        setGoalAnimState( LaraStateId::UnderwaterForward );
        getLara().m_state.rotation.X = -45_deg;
        getLara().m_state.fallspeed = 80_spd;
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

        if( !getEngine().getInputHandler().getInputState().action )
        {
            return;
        }

        const auto gradient = abs( collisionInfo.frontLeft.floorSpace.y - collisionInfo.frontRight.floorSpace.y );
        if( gradient >= core::MaxGrabbableGradient )
        {
            return;
        }

        if( collisionInfo.front.ceilingSpace.y > 0_len )
        {
            return;
        }

        if( collisionInfo.mid.ceilingSpace.y > -core::ClimbLimit2ClickMin )
        {
            return;
        }

        if( collisionInfo.front.floorSpace.y + core::LaraSwimHeight <= 2 * -core::QuarterSectorSize )
        {
            return;
        }

        if( collisionInfo.front.floorSpace.y + core::LaraSwimHeight > core::DefaultCollisionRadius )
        {
            return;
        }

        const auto yRot = alignRotation( getLara().m_state.rotation.Y, 35_deg );
        if( !yRot.is_initialized() )
        {
            return;
        }

        getLara().m_state.position.position += core::TRVec( 0_len, 695_len + collisionInfo.front.floorSpace.y, 0_len );
        getLara().updateFloorHeight( -381_len );
        core::TRVec d = getLara().m_state.position.position;
        if( *yRot == 0_deg )
        {
            d.Z = (getLara().m_state.position.position.Z / core::SectorSize + 1) * core::SectorSize
                  + core::DefaultCollisionRadius;
        }
        else if( *yRot == 180_deg )
        {
            d.Z = (getLara().m_state.position.position.Z / core::SectorSize + 0) * core::SectorSize
                  - core::DefaultCollisionRadius;
        }
        else if( *yRot == -90_deg )
        {
            d.X = (getLara().m_state.position.position.X / core::SectorSize + 0) * core::SectorSize
                  - core::DefaultCollisionRadius;
        }
        else if( *yRot == 90_deg )
        {
            d.X = (getLara().m_state.position.position.X / core::SectorSize + 1) * core::SectorSize
                  + core::DefaultCollisionRadius;
        }
        else
        {
            throw std::runtime_error( "Unexpected angle value" );
        }

        getLara().m_state.position.position = d;

        setAnimation( AnimationId::CLIMB_OUT_OF_WATER, 1849_frame );
        setGoalAnimState( LaraStateId::Stop );
        getLara().m_state.speed = 0_spd;
        getLara().m_state.fallspeed = 0_spd;
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
