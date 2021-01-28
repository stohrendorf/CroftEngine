#pragma once

#include "modelobject.h"

namespace engine::objects
{
class LightningBall final : public ModelObject
{
public:
  LightningBall(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  LightningBall(const gsl::not_null<World*>& world,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  static constexpr size_t SegmentPoints = 16;

  void serialize(const serialization::Serializer<World>& ser) override;

private:
  static constexpr size_t ChildBolts = 5;

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
    std::shared_ptr<render::scene::Mesh> mesh;
    std::shared_ptr<render::scene::Node> node;
    std::shared_ptr<gl::VertexBuffer<glm::vec3>> vb;

    void serialize(const serialization::Serializer<World>& ser);
  };

  std::array<ChildBolt, ChildBolts> m_childBolts;

  std::shared_ptr<render::scene::Mesh> m_mainBoltMesh;
  std::shared_ptr<render::scene::Node> m_mainBoltNode;
  std::shared_ptr<gl::VertexBuffer<glm::vec3>> m_mainVb;

  void init(World& world);
};
} // namespace engine::objects
