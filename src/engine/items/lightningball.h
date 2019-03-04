#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class LightningBall final : public ModelItemNode
{
public:
    LightningBall(const gsl::not_null<Engine*>& engine,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const loader::file::SkeletalModelType& animatedModel,
                  const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& boltProgram);

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& info) override;

    static constexpr const size_t SegmentPoints = 16;

    void load(const YAML::Node& n) override;

private:
    static constexpr const size_t ChildBolts = 5;

    void prepareRender();

    size_t m_poles = 0;
    bool m_laraHit = false;
    int m_chargeTimeout = 1;
    bool m_shooting = false;
    core::TRVec m_mainBoltEnd;

    struct ChildBolt
    {
        size_t startIndex = 0;
        core::TRVec end{};
        std::shared_ptr<gameplay::Mesh> mesh;
    };

    std::array<ChildBolt, ChildBolts> m_childBolts;

    std::shared_ptr<gameplay::Mesh> m_mainBoltMesh;
};
}
}
