#pragma once

#include "itemnode.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class AIAgent : public ModelItemNode
{
public:
    AIAgent(const gsl::not_null<level::Level*>& level,
            const std::string& name,
            const gsl::not_null<const loader::Room*>& room,
            const loader::Item& item,
            const loader::SkeletalModelType& animatedModel);

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

protected:
    void rotateCreatureTilt(core::Angle angle)
    {
        const auto dz = core::Angle( 4 * angle.toAU() ) - m_state.rotation.Z;
        const core::Angle z = util::clamp( dz, -3_deg, +3_deg );
        m_state.rotation.Z += z;
    }

    void rotateCreatureHead(const core::Angle& angle)
    {
        m_state.creatureInfo->rotateHead( angle );
    }

    bool animateCreature(core::Angle angle, core::Angle tilt);

    core::Angle rotateTowardsTarget(core::Angle maxRotationSpeed);

    int getHealth() const
    {
        return m_state.health;
    }

private:

    bool anyMovingEnabledItemInReach() const;

    bool isPositionOutOfReach(const core::TRVec& testPosition, int currentBoxFloor, int nextBoxFloor,
                              const ai::LotInfo& lotInfo) const;

    uint16_t m_requiredAnimState = 0;

    const int m_collisionRadius;
};
}
}
