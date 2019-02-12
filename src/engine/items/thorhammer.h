#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class ThorHammerBlock final : public ModelItemNode
{
public:
    ThorHammerBlock(const gsl::not_null<loader::file::level::Level*>& level,
                     const gsl::not_null<const loader::file::Room*>& room,
                     const loader::file::Item& item,
                     const loader::file::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
    }

    void collide(LaraNode& node, CollisionInfo& info) override;
};

class ThorHammerHandle final : public ModelItemNode
{
public:
    ThorHammerHandle(const gsl::not_null<loader::file::level::Level*>& level,
                     const gsl::not_null<const loader::file::Room*>& room,
                     const loader::file::Item& item,
                     const loader::file::SkeletalModelType& animatedModel);

    void update() override;

    void collide(LaraNode& node, CollisionInfo& info) override;

private:
    std::shared_ptr<ThorHammerBlock> m_block;
};
}
}
