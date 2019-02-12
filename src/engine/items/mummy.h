#pragma once

#include "itemnode.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Mummy final
        : public ModelItemNode
{
public:
    Mummy(const gsl::not_null<level::Level*>& level,
          const gsl::not_null<const loader::file::Room*>& room,
          const loader::file::Item& item,
          const loader::file::SkeletalModelType& animatedModel);

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& info) override;

private:
    core::Angle m_headRotation{0_deg};
};
}
}
