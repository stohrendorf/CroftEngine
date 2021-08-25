#pragma once

#include "modelobject.h"

namespace engine::objects
{
class LightningEmitter final : public ModelObject
{
public:
  LightningEmitter(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  LightningEmitter(const std::string& name,
                   const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const world::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  static constexpr size_t SegmentSplits = 8;
  static constexpr size_t ControlPoints = (1u << SegmentSplits) + 1;

  void serialize(const serialization::Serializer<world::World>& ser) override;

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
    std::shared_ptr<render::scene::Mesh> mesh;
    std::shared_ptr<render::scene::Node> node;
    std::shared_ptr<gl::VertexBuffer<glm::vec3>> vb;
  };

  std::array<ChildBolt, ChildBolts> m_childBolts;

  std::shared_ptr<render::scene::Mesh> m_mainBoltMesh;
  std::shared_ptr<render::scene::Node> m_mainBoltNode;
  std::shared_ptr<gl::VertexBuffer<glm::vec3>> m_mainVb;

  void init(world::World& world);
};
} // namespace engine::objects
