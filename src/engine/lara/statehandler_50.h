#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "loader/file/level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_50 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_50(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::UseMidas}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        emitSparkles( getLara(), getLara().getLevel() );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0_len;
        setMovementAngle( getLara().m_state.rotation.Y );
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::LaraWalkHeight );
    }

    static void emitSparkles(const LaraNode& lara, loader::file::level::Level& level)
    {
        const auto spheres = lara.getSkeleton()->getBoneCollisionSpheres(
                lara.m_state,
                *lara.getSkeleton()->getInterpolationInfo( lara.m_state ).getNearestFrame(),
                nullptr );

        const auto& normalLara = *level.m_animatedModels[TR1ItemId::Lara];
        for( size_t i = 0; i < spheres.size(); ++i )
        {
            if( lara.getNode()->getChild( i )->getDrawable() == normalLara.models[i].get() )
                continue;

            const auto r = spheres[i].radius;
            auto p = core::TRVec{spheres[i].getPosition()};
            p.X += util::rand15s( r, core::Length::type() );
            p.Y += util::rand15s( r, core::Length::type() );
            p.Z += util::rand15s( r, core::Length::type() );
            auto fx = std::make_shared<SparkleParticle>(
                    core::RoomBoundPosition{lara.m_state.position.room, p}, level );
            level.m_particles.emplace_back( fx );
        }
    }
};
}
}
